#!/usr/bin/env python3

from __future__ import annotations

from collections.abc import Mapping, Sequence
from typing import Any

from openai import OpenAI


def require_nonempty_text(value: Any, name: str) -> str:
    if value is None:
        raise ValueError(f"{name} cannot be empty")
    text = str(value).strip()
    if not text:
        raise ValueError(f"{name} cannot be empty")
    return text


def normalize_optional_temperature(value: Any, name: str = "temperature") -> float | None:
    if value is None:
        return None
    if isinstance(value, str):
        value = value.strip()
        if not value:
            return None
    try:
        return float(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{name} must be a number") from exc


def create_openai_client(api_key, base_url=None, *, api_key_required_message):
    if api_key is None or not str(api_key).strip():
        raise RuntimeError(api_key_required_message)

    client_kwargs = {
        "api_key": require_nonempty_text(api_key, "api_key"),
    }
    if base_url is not None:
        client_kwargs["base_url"] = require_nonempty_text(base_url, "base_url")

    return OpenAI(**client_kwargs)


def extract_first_message_text(response) -> str:
    choices = getattr(response, "choices", None) or []
    if not choices:
        raise ValueError("OpenAI response missing choices")

    message = getattr(choices[0], "message", None)
    content = getattr(message, "content", "") if message is not None else ""
    if isinstance(content, str):
        return content

    if isinstance(content, Sequence) and not isinstance(content, (str, bytes, bytearray)):
        parts: list[str] = []
        for part in content:
            if isinstance(part, Mapping):
                text = part.get("text")
            else:
                text = getattr(part, "text", None)
            if text:
                parts.append(str(text))
        return "".join(parts)

    text = getattr(content, "text", None)
    if text is not None:
        return str(text)
    return str(content)


def call_llm_text(client, *, model, messages, temperature=None) -> str:
    request_kwargs = {
        "model": require_nonempty_text(model, "model"),
        "messages": messages,
    }
    normalized_temperature = normalize_optional_temperature(temperature)
    if normalized_temperature is not None:
        request_kwargs["temperature"] = normalized_temperature
    response = client.chat.completions.create(**request_kwargs)
    return extract_first_message_text(response)
