from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Any


class AIBackend(ABC):
    @property
    @abstractmethod
    def name(self) -> str:
        ...

    @abstractmethod
    def generate(self, system_prompt: str, user_prompt: str, json_schema: dict[str, Any]) -> dict[str, Any]:
        """Send prompts to the AI and return a parsed JSON dict matching the schema."""
        ...

    def generate_vision(
        self,
        system_prompt: str,
        user_prompt: str,
        json_schema: dict[str, Any],
        images: list[str],
    ) -> dict[str, Any]:
        """Send prompts with images to the AI. Default: raise NotImplementedError."""
        raise NotImplementedError(f"Vision mode not supported by {self.name}")


class VisionBackend(AIBackend):
    @abstractmethod
    def generate_vision(
        self,
        system_prompt: str,
        user_prompt: str,
        json_schema: dict[str, Any],
        images: list[str],
    ) -> dict[str, Any]:
        ...
