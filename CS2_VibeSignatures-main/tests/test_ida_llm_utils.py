import unittest
from types import SimpleNamespace
from unittest.mock import MagicMock, patch

import ida_llm_utils


class TestRequireNonemptyText(unittest.TestCase):
    def test_require_nonempty_text_returns_stripped_text(self) -> None:
        self.assertEqual("hello", ida_llm_utils.require_nonempty_text("  hello  ", "value"))

    def test_require_nonempty_text_raises_value_error_for_blank_text(self) -> None:
        with self.assertRaises(ValueError):
            ida_llm_utils.require_nonempty_text("   ", "value")


class TestCreateOpenAiClient(unittest.TestCase):
    def test_create_openai_client_raises_runtime_error_when_api_key_missing(self) -> None:
        with self.assertRaisesRegex(RuntimeError, "LLM API key required"):
            ida_llm_utils.create_openai_client(
                None,
                api_key_required_message="LLM API key required",
            )

    @patch("ida_llm_utils.OpenAI")
    def test_create_openai_client_uses_trimmed_api_key_and_base_url(self, mock_openai) -> None:
        mock_client = object()
        mock_openai.return_value = mock_client

        client = ida_llm_utils.create_openai_client(
            "  test-api-key  ",
            "  https://example.invalid/v1  ",
            api_key_required_message="unused",
        )

        self.assertIs(mock_client, client)
        mock_openai.assert_called_once_with(
            api_key="test-api-key",
            base_url="https://example.invalid/v1",
        )


class TestExtractFirstMessageText(unittest.TestCase):
    def test_extract_first_message_text_supports_string_content(self) -> None:
        response = SimpleNamespace(
            choices=[
                SimpleNamespace(
                    message=SimpleNamespace(content="hello from llm"),
                )
            ]
        )

        self.assertEqual("hello from llm", ida_llm_utils.extract_first_message_text(response))

    def test_extract_first_message_text_supports_text_parts(self) -> None:
        response = SimpleNamespace(
            choices=[
                SimpleNamespace(
                    message=SimpleNamespace(
                        content=[
                            SimpleNamespace(text="hello "),
                            {"text": "from "},
                            SimpleNamespace(text="parts"),
                        ]
                    ),
                )
            ]
        )

        self.assertEqual("hello from parts", ida_llm_utils.extract_first_message_text(response))

    def test_extract_first_message_text_raises_value_error_on_empty_choices(self) -> None:
        response = SimpleNamespace(choices=[])

        with self.assertRaises(ValueError):
            ida_llm_utils.extract_first_message_text(response)


class TestCallLlmText(unittest.TestCase):
    def test_call_llm_text_invokes_chat_completions_and_returns_first_message_text(self) -> None:
        response = SimpleNamespace(
            choices=[
                SimpleNamespace(
                    message=SimpleNamespace(content="found_vcall:\n  []"),
                )
            ]
        )
        create = MagicMock(return_value=response)
        client = SimpleNamespace(
            chat=SimpleNamespace(
                completions=SimpleNamespace(create=create),
            )
        )
        messages = [{"role": "user", "content": "hello"}]

        text = ida_llm_utils.call_llm_text(
            client,
            model="  gpt-4o-mini  ",
            messages=messages,
            temperature=0.25,
        )

        self.assertEqual("found_vcall:\n  []", text)
        create.assert_called_once_with(
            model="gpt-4o-mini",
            messages=messages,
            temperature=0.25,
        )

    def test_call_llm_text_omits_temperature_when_not_configured(self) -> None:
        response = SimpleNamespace(
            choices=[
                SimpleNamespace(
                    message=SimpleNamespace(content="found_vcall:\n  []"),
                )
            ]
        )
        create = MagicMock(return_value=response)
        client = SimpleNamespace(
            chat=SimpleNamespace(
                completions=SimpleNamespace(create=create),
            )
        )
        messages = [{"role": "user", "content": "hello"}]

        text = ida_llm_utils.call_llm_text(
            client,
            model="gpt-4o-mini",
            messages=messages,
        )

        self.assertEqual("found_vcall:\n  []", text)
        create.assert_called_once_with(
            model="gpt-4o-mini",
            messages=messages,
        )


if __name__ == "__main__":
    unittest.main()
