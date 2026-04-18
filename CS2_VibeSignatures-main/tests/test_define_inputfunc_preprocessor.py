import json
import unittest
from unittest.mock import AsyncMock, patch

from ida_preprocessor_scripts import _define_inputfunc as define_inputfunc


class _FakeTextContent:
    def __init__(self, text: str) -> None:
        self.text = text


class _FakeCallToolResult:
    def __init__(self, payload: dict[str, object]) -> None:
        self.content = [_FakeTextContent(json.dumps(payload))]


def _py_eval_payload(payload: object) -> _FakeCallToolResult:
    return _FakeCallToolResult(
        {
            "result": json.dumps(payload),
            "stdout": "",
            "stderr": "",
        }
    )


class TestBuildDefineInputFuncPyEval(unittest.TestCase):
    def test_build_define_inputfunc_py_eval_embeds_exact_string_offset_and_text_filter(
        self,
    ) -> None:
        code = define_inputfunc._build_define_inputfunc_py_eval(
            input_name="ShowHudHint",
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
        )

        self.assertIn("ShowHudHint", code)
        self.assertIn("handler_ptr_offset = 16", code)
        self.assertIn("allowed_segment_names", code)
        self.assertIn("idautils.Strings", code)
        self.assertIn("idautils.XrefsTo", code)
        self.assertIn("idaapi.getseg", code)
        self.assertIn("ida_bytes.get_qword", code)
        self.assertIn("handler_seg_name == '.text'", code)
        compile(code, "<define_inputfunc_py_eval>", "exec")

    def test_build_define_inputfunc_py_eval_embeds_custom_segment_names(self) -> None:
        code = define_inputfunc._build_define_inputfunc_py_eval(
            input_name="CustomInput",
            handler_ptr_offset=0x18,
            allowed_segment_names=(".data", ".data.rel.ro"),
        )

        self.assertIn("CustomInput", code)
        self.assertIn("handler_ptr_offset = 24", code)
        self.assertIn(".data.rel.ro", code)
        compile(code, "<define_inputfunc_py_eval_custom>", "exec")


class TestCollectDefineInputFuncCandidates(unittest.IsolatedAsyncioTestCase):
    async def test_collect_define_inputfunc_candidates_uses_py_eval_and_returns_candidates(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "string_eas": ["0x180800000"],
                "items": [
                    {
                        "string_ea": "0x180800000",
                        "xref_from": "0x180900000",
                        "xref_seg_name": ".data",
                        "handler_ptr_ea": "0x180900010",
                        "handler_va": "0x180123450",
                        "handler_seg_name": ".text",
                    }
                ],
            }
        )

        result = await define_inputfunc._collect_define_inputfunc_candidates(
            session=session,
            input_name="ShowHudHint",
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
            debug=True,
        )

        self.assertEqual(
            {
                "string_eas": ["0x180800000"],
                "items": [
                    {
                        "string_ea": "0x180800000",
                        "xref_from": "0x180900000",
                        "xref_seg_name": ".data",
                        "handler_ptr_ea": "0x180900010",
                        "handler_va": "0x180123450",
                        "handler_seg_name": ".text",
                    }
                ],
            },
            result,
        )
        code = session.call_tool.await_args.kwargs["arguments"]["code"]
        self.assertIn("ShowHudHint", code)
        self.assertIn("handler_ptr_offset = 16", code)

    async def test_collect_define_inputfunc_candidates_returns_none_on_invalid_payload(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload({"unexpected": []})

        result = await define_inputfunc._collect_define_inputfunc_candidates(
            session=session,
            input_name="ShowHudHint",
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
            debug=True,
        )

        self.assertIsNone(result)

    async def test_collect_define_inputfunc_candidates_returns_none_on_non_text_handler(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "string_eas": ["0x180800000"],
                "items": [
                    {
                        "string_ea": "0x180800000",
                        "xref_from": "0x180900000",
                        "xref_seg_name": ".data",
                        "handler_ptr_ea": "0x180900010",
                        "handler_va": "0x180A00000",
                        "handler_seg_name": ".rdata",
                    }
                ],
            }
        )

        result = await define_inputfunc._collect_define_inputfunc_candidates(
            session=session,
            input_name="ShowHudHint",
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
            debug=True,
        )

        self.assertIsNone(result)


class TestPreprocessDefineInputFuncSkill(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_define_inputfunc_skill_writes_requested_func_payload(
        self,
    ) -> None:
        session = AsyncMock()
        requested_fields = [
            (
                "ShowHudHint",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

        with patch.object(
            define_inputfunc,
            "_collect_define_inputfunc_candidates",
            AsyncMock(
                return_value={
                    "string_eas": ["0x180800000"],
                    "items": [
                        {
                            "string_ea": "0x180800000",
                            "xref_from": "0x180900000",
                            "xref_seg_name": ".data",
                            "handler_ptr_ea": "0x180900010",
                            "handler_va": "0x180123450",
                            "handler_seg_name": ".text",
                        }
                    ],
                }
            ),
        ), patch.object(
            define_inputfunc,
            "_query_func_info",
            AsyncMock(
                return_value={"func_va": "0x180123450", "func_size": "0x90"}
            ),
        ), patch.object(
            define_inputfunc,
            "preprocess_gen_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_sig": "48 89 5C 24 ? 57 48 83 EC ?",
                    "func_rva": "0x123450",
                    "func_size": "0x90",
                }
            ),
        ), patch.object(
            define_inputfunc,
            "write_func_yaml",
        ) as mock_write, patch.object(
            define_inputfunc,
            "_rename_func_best_effort",
            AsyncMock(),
        ) as mock_rename:
            result = await define_inputfunc.preprocess_define_inputfunc_skill(
                session=session,
                expected_outputs=["/tmp/ShowHudHint.windows.yaml"],
                platform="windows",
                image_base=0x180000000,
                target_name="ShowHudHint",
                input_name="ShowHudHint",
                generate_yaml_desired_fields=requested_fields,
                handler_ptr_offset=0x10,
                allowed_segment_names=(".data",),
                rename_to="ShowHudHint",
                debug=True,
            )

        self.assertTrue(result)
        mock_write.assert_called_once_with(
            "/tmp/ShowHudHint.windows.yaml",
            {
                "func_name": "ShowHudHint",
                "func_va": "0x180123450",
                "func_rva": "0x123450",
                "func_size": "0x90",
                "func_sig": "48 89 5C 24 ? 57 48 83 EC ?",
            },
        )
        mock_rename.assert_awaited_once_with(
            session=session,
            func_va="0x180123450",
            func_name="ShowHudHint",
            debug=True,
        )

    async def test_preprocess_define_inputfunc_skill_rejects_multiple_text_handlers(
        self,
    ) -> None:
        requested_fields = [
            (
                "ShowHudHint",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

        with patch.object(
            define_inputfunc,
            "_collect_define_inputfunc_candidates",
            AsyncMock(
                return_value={
                    "string_eas": ["0x180800000"],
                    "items": [
                        {
                            "string_ea": "0x180800000",
                            "xref_from": "0x180900000",
                            "xref_seg_name": ".data",
                            "handler_ptr_ea": "0x180900010",
                            "handler_va": "0x180123450",
                            "handler_seg_name": ".text",
                        },
                        {
                            "string_ea": "0x180800000",
                            "xref_from": "0x180910000",
                            "xref_seg_name": ".data",
                            "handler_ptr_ea": "0x180910010",
                            "handler_va": "0x180223450",
                            "handler_seg_name": ".text",
                        },
                    ],
                }
            ),
        ), patch.object(define_inputfunc, "write_func_yaml") as mock_write:
            result = await define_inputfunc.preprocess_define_inputfunc_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/ShowHudHint.windows.yaml"],
                platform="windows",
                image_base=0x180000000,
                target_name="ShowHudHint",
                input_name="ShowHudHint",
                generate_yaml_desired_fields=requested_fields,
                handler_ptr_offset=0x10,
                allowed_segment_names=(".data",),
                rename_to="ShowHudHint",
                debug=True,
            )

        self.assertFalse(result)
        mock_write.assert_not_called()
