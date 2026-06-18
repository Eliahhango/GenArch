from __future__ import annotations

import base64
import json
import logging
import re
from dataclasses import dataclass, field
from io import BytesIO
from pathlib import Path
from typing import Any

from PIL import Image as PILImage

from backends.base import AIBackend, VisionBackend
from schemas import BuildingPlan

logger = logging.getLogger(__name__)


try:
    import pymupdf
    HAS_PYMUPDF = True
except ImportError:
    HAS_PYMUPDF = False
    logger.warning("pymupdf not installed — PDF import will not work. Install with: pip install pymupdf")


@dataclass
class ScaleInfo:
    pixels_per_mm: float = 0.0
    source: str = ""
    confidence: str = "low"


@dataclass
class ImportResult:
    source_file: str
    file_type: str
    pages_processed: int
    plans: list[BuildingPlan] = field(default_factory=list)
    errors: list[str] = field(default_factory=list)
    scale: ScaleInfo = field(default_factory=ScaleInfo)
    extracted_text: str = ""

    @property
    def success(self) -> bool:
        return len(self.plans) > 0 and len(self.errors) == 0


class ArchicadImporter:
    SUPPORTED_IMAGE_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".tiff", ".tif", ".webp"}
    SUPPORTED_PDF_EXTENSIONS = {".pdf"}

    def __init__(self, backend: VisionBackend) -> None:
        self._backend = backend

    def import_file(
        self,
        file_path: str | Path,
        json_schema: dict[str, Any] | None = None,
        import_prompt: str = "",
    ) -> ImportResult:
        file_path = Path(file_path)
        if not file_path.exists():
            return ImportResult(source_file=str(file_path), file_type="unknown", errors=[f"File not found: {file_path}"])

        ext = file_path.suffix.lower()

        if ext in self.SUPPORTED_IMAGE_EXTENSIONS:
            return self._import_image(file_path, json_schema, import_prompt)
        elif ext in self.SUPPORTED_PDF_EXTENSIONS:
            return self._import_pdf(file_path, json_schema, import_prompt)
        else:
            return ImportResult(
                source_file=str(file_path),
                file_type=ext,
                errors=[f"Unsupported file type: {ext}. Supported: {self.SUPPORTED_IMAGE_EXTENSIONS} {self.SUPPORTED_PDF_EXTENSIONS}"],
            )

    def _import_image(
        self,
        file_path: Path,
        json_schema: dict[str, Any] | None,
        import_prompt: str,
    ) -> ImportResult:
        result = ImportResult(source_file=str(file_path), file_type=file_path.suffix.lower())

        try:
            pil_image = PILImage.open(file_path)
            b64_string = self._pil_to_base64(pil_image)
            result.pages_processed = 1

            plan = self._call_vision_model(b64_string, json_schema, import_prompt, result)
            if plan:
                result.plans.append(plan)
        except Exception as e:
            result.errors.append(str(e))
            logger.exception("Image import failed")

        return result

    def _import_pdf(
        self,
        file_path: Path,
        json_schema: dict[str, Any] | None,
        import_prompt: str,
    ) -> ImportResult:
        if not HAS_PYMUPDF:
            return ImportResult(
                source_file=str(file_path),
                file_type="pdf",
                errors=["pymupdf not installed. Install with: pip install pymupdf"],
            )

        result = ImportResult(source_file=str(file_path), file_type="pdf")

        try:
            doc = pymupdf.open(str(file_path))
            result.pages_processed = len(doc)

            for page_num, page in enumerate(doc):
                pdf_type = self._detect_pdf_type(page)
                logger.info("PDF page %d: type=%s", page_num + 1, pdf_type)

                text = ""
                if pdf_type in ("vector", "hybrid"):
                    text = page.get_text("text")
                    result.extracted_text += text

                pix = page.get_pixmap(dpi=200)
                b64_string = self._pixmap_to_base64(pix)

                plan = self._call_vision_model(b64_string, json_schema, import_prompt, result, page_num)
                if plan:
                    result.plans.append(plan)

                if page_num >= 4:
                    logger.info("Limiting to first 5 pages of PDF")
                    break

            doc.close()
        except Exception as e:
            result.errors.append(str(e))
            logger.exception("PDF import failed")

        return result

    def _call_vision_model(
        self,
        b64_image: str,
        json_schema: dict[str, Any] | None,
        import_prompt: str,
        result: ImportResult,
        page_num: int = 0,
    ) -> BuildingPlan | None:
        from prompts import FLOOR_PLAN_FROM_IMAGE_PROMPT
        from schemas import BuildingPlan, FloorDef, ProjectDef

        system_prompt = import_prompt or FLOOR_PLAN_FROM_IMAGE_PROMPT
        schema = json_schema or BuildingPlan.model_json_schema()

        user_prompt = f"Extract the building plan from this floor plan image{' (page ' + str(page_num + 1) + ')' if page_num > 0 else ''}."
        if result.extracted_text:
            user_prompt += f"\n\nExtracted text from the drawing:\n{result.extracted_text[:2000]}"

        try:
            raw = self._backend.generate_vision(system_prompt, user_prompt, schema, [b64_image])
            plan = BuildingPlan.model_validate(raw)
            return plan
        except Exception as e:
            logger.warning("Vision model failed on page %d: %s", page_num + 1, e)
            return None

    @staticmethod
    def _detect_pdf_type(page) -> str:
        text = page.get_text("text").strip()
        images = page.get_images()
        full_page_images = []
        for img in images:
            bbox = page.get_image_bbox(img)
            if hasattr(bbox, 'width') and hasattr(bbox, 'height'):
                area_ratio = (bbox.width * bbox.height) / (page.rect.width * page.rect.height)
                if area_ratio > 0.8:
                    full_page_images.append(img)

        if text and not full_page_images:
            return "vector"
        elif not text and full_page_images:
            return "scanned"
        else:
            return "hybrid"

    @staticmethod
    def _pil_to_base64(pil_image: PILImage.Image, fmt: str = "PNG") -> str:
        buffer = BytesIO()
        pil_image.save(buffer, format=fmt)
        return base64.b64encode(buffer.getvalue()).decode("utf-8")

    @staticmethod
    def _pixmap_to_base64(pixmap, fmt: str = "PNG") -> str:
        return base64.b64encode(pixmap.tobytes(fmt)).decode("utf-8")
