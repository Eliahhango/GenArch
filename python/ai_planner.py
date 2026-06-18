from __future__ import annotations

import json
import logging
from typing import Any

from pydantic import ValidationError

from backends.base import AIBackend
from schemas import BuildingPlan

logger = logging.getLogger(__name__)


class AIPlanner:
    def __init__(self, backend: AIBackend, system_prompt: str | None = None) -> None:
        self._backend = backend
        self._system_prompt = system_prompt or self._default_system_prompt()

    @staticmethod
    def _default_system_prompt() -> str:
        from prompts import FULL_BUILDING_SYSTEM_PROMPT
        return FULL_BUILDING_SYSTEM_PROMPT

    @property
    def backend_name(self) -> str:
        return self._backend.name

    def plan(self, user_prompt: str, max_retries: int = 2) -> BuildingPlan:
        json_schema = BuildingPlan.model_json_schema()

        for attempt in range(max_retries + 1):
            try:
                raw = self._backend.generate(self._system_prompt, user_prompt, json_schema)
                plan = BuildingPlan.model_validate(raw)
                logger.info("Plan generated successfully via %s", self._backend.name)
                return plan
            except ValidationError as e:
                logger.warning("Schema validation failed (attempt %d/%d): %s", attempt + 1, max_retries + 1, e)
                if attempt == max_retries:
                    raise
            except json.JSONDecodeError as e:
                logger.warning("JSON parse failed (attempt %d/%d): %s", attempt + 1, max_retries + 1, e)
                if attempt == max_retries:
                    raise
            except Exception:
                logger.exception("Unexpected error from backend %s (attempt %d/%d)", self._backend.name, attempt + 1, max_retries + 1)
                if attempt == max_retries:
                    raise

        raise RuntimeError("Failed to generate a valid plan after all retries")

    def plan_to_json(self, user_prompt: str) -> str:
        plan = self.plan(user_prompt)
        return plan.model_dump_json(indent=2)
