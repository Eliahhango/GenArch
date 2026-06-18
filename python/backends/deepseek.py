from __future__ import annotations

import json
import os
from typing import Any

import requests

from .base import AIBackend


class DeepSeekBackend(AIBackend):
    def __init__(self, model: str | None = None) -> None:
        self._api_key = os.environ.get("DEEPSEEK_API_KEY", "")
        self._model = model or os.environ.get("DEEPSEEK_MODEL", "deepseek-v4-pro")
        self._endpoint = "https://api.deepseek.com/chat/completions"

    @property
    def name(self) -> str:
        return f"deepseek ({self._model})"

    def generate(self, system_prompt: str, user_prompt: str, json_schema: dict[str, Any]) -> dict[str, Any]:
        if not self._api_key:
            raise RuntimeError("DEEPSEEK_API_KEY not set in environment or .env file")

        schema_text = json.dumps(json_schema, indent=2)
        messages = [
            {
                "role": "system",
                "content": (
                    f"{system_prompt}\n\n"
                    "You MUST respond with ONLY a valid JSON object. No markdown, no code fences, no explanation.\n"
                    f"The JSON must conform to this schema:\n{schema_text}"
                ),
            },
            {"role": "user", "content": user_prompt},
        ]

        response = requests.post(
            self._endpoint,
            headers={
                "Content-Type": "application/json",
                "Authorization": f"Bearer {self._api_key}",
            },
            json={
                "model": self._model,
                "messages": messages,
                "response_format": {"type": "json_object"},
                "stream": False,
            },
            timeout=120,
        )
        response.raise_for_status()
        data = response.json()
        content = data["choices"][0]["message"]["content"]
        return json.loads(content)
