from __future__ import annotations

import json
import os
import re
from typing import Any

from ollama import Client

from .base import VisionBackend


class OllamaCloudBackend(VisionBackend):
    def __init__(self, model: str | None = None) -> None:
        self._api_key = os.environ.get("OLLAMA_API_KEY", "")
        self._model = model or os.environ.get("OLLAMA_CLOUD_MODEL", "deepseek-v3.1:671b-cloud")
        self._vision_model = os.environ.get("OLLAMA_CLOUD_VISION_MODEL", "qwen3-coder:480b-cloud")
        self._client = Client(
            host="https://ollama.com",
            headers={"Authorization": f"Bearer {self._api_key}"},
        )

    @property
    def name(self) -> str:
        return f"ollama-cloud ({self._model})"

    def generate(self, system_prompt: str, user_prompt: str, json_schema: dict[str, Any]) -> dict[str, Any]:
        if not self._api_key:
            raise RuntimeError("OLLAMA_API_KEY not set in environment or .env file")

        combined_prompt = (
            f"{system_prompt}\n\n"
            "You MUST respond with ONLY a valid JSON object. No markdown, no code fences, no explanation.\n"
            f"The JSON must conform to this schema:\n{json.dumps(json_schema, indent=2)}"
        )

        response = self._client.chat(
            model=self._model,
            messages=[{"role": "user", "content": f"{combined_prompt}\n\nUser request: {user_prompt}"}],
            format=json_schema,
            options={"temperature": 0},
            stream=False,
        )
        content = response["message"]["content"]
        return json.loads(self._extract_json(content))

    def generate_vision(
        self,
        system_prompt: str,
        user_prompt: str,
        json_schema: dict[str, Any],
        images: list[str],
    ) -> dict[str, Any]:
        if not self._api_key:
            raise RuntimeError("OLLAMA_API_KEY not set in environment or .env file")

        combined_prompt = (
            f"{system_prompt}\n\n"
            "You MUST respond with ONLY a valid JSON object. No markdown, no code fences, no explanation.\n"
            f"The JSON must conform to this schema:\n{json.dumps(json_schema, indent=2)}\n\n"
            f"User request: {user_prompt}"
        )

        response = self._client.chat(
            model=self._vision_model,
            messages=[{"role": "user", "content": combined_prompt, "images": images}],
            format=json_schema,
            options={"temperature": 0},
            stream=False,
        )
        content = response["message"]["content"]
        return json.loads(self._extract_json(content))

    @staticmethod
    def _extract_json(text: str) -> str:
        text = text.strip()
        if text.startswith("```"):
            lines = text.split("\n")
            if lines[0].startswith("```"):
                lines = lines[1:]
            if lines and lines[-1].strip() == "```":
                lines = lines[:-1]
            text = "\n".join(lines)
        return text
