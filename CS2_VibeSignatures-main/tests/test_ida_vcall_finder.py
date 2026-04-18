import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import AsyncMock, patch

import ida_vcall_finder


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


class TestBuildFunctionDumpExportPyEval(unittest.TestCase):
    def test_build_function_dump_export_py_eval_embeds_yaml_dump_and_absolute_path(self) -> None:
        output_path = str(Path("/tmp/vcall-detail.yaml").resolve())
        script = ida_vcall_finder.build_function_dump_export_py_eval(
            0x3EA720,
            output_path=output_path,
            object_name="g_pNetworkMessages",
            module_name="networksystem",
            platform="linux",
        )
        self.assertIn("import yaml", script)
        self.assertIn("PyYAML is required for vcall_finder detail export", script)
        self.assertIn("yaml.dump", script)
        self.assertIn(output_path, script)


class TestExportObjectXrefDetailsViaMcp(unittest.IsolatedAsyncioTestCase):
    async def test_export_object_xref_details_via_mcp_counts_success_from_remote_ack(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            detail_path = ida_vcall_finder.build_vcall_detail_path(
                temp_dir,
                "14141b",
                "g_pNetworkMessages",
                "networksystem",
                "linux",
                "sub_2000",
            ).resolve()
            session = AsyncMock()
            session.call_tool.side_effect = [
                _py_eval_payload(
                    {
                        "object_ea": "0x1000",
                        "functions": [
                            {
                                "func_name": "sub_2000",
                                "func_va": "0x2000",
                            }
                        ],
                    }
                ),
                _py_eval_payload(
                    {
                        "ok": True,
                        "output_path": str(detail_path),
                        "bytes_written": 512,
                        "format": "yaml",
                    }
                ),
            ]

            summary = await ida_vcall_finder.export_object_xref_details_via_mcp(
                session,
                output_root=temp_dir,
                gamever="14141b",
                module_name="networksystem",
                platform="linux",
                object_name="g_pNetworkMessages",
                debug=False,
            )

            self.assertEqual("success", summary["status"])
            self.assertEqual(1, summary["exported_functions"])
            self.assertEqual(0, summary["failed_functions"])
            second_code = session.call_tool.await_args_list[1].kwargs["arguments"]["code"]
            self.assertIn(str(detail_path), second_code)
            self.assertIn("yaml.dump", second_code)

    async def test_export_object_xref_details_via_mcp_counts_failure_from_remote_ack(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session = AsyncMock()
            session.call_tool.side_effect = [
                _py_eval_payload(
                    {
                        "object_ea": "0x1000",
                        "functions": [
                            {
                                "func_name": "sub_2000",
                                "func_va": "0x2000",
                            }
                        ],
                    }
                ),
                _py_eval_payload(
                    {
                        "ok": False,
                        "output_path": str(Path(temp_dir, "detail.yaml")),
                        "error": "permission denied",
                    }
                ),
            ]

            summary = await ida_vcall_finder.export_object_xref_details_via_mcp(
                session,
                output_root=temp_dir,
                gamever="14141b",
                module_name="networksystem",
                platform="linux",
                object_name="g_pNetworkMessages",
                debug=False,
            )

            self.assertEqual("failed", summary["status"])
            self.assertEqual(0, summary["exported_functions"])
            self.assertEqual(1, summary["failed_functions"])
            self.assertEqual(0, summary["skipped_functions"])


class TestCallOpenAiForVcalls(unittest.TestCase):
    @patch("ida_vcall_finder.call_llm_text")
    def test_call_openai_for_vcalls_uses_shared_llm_helper(self, mock_call_llm_text) -> None:
        mock_call_llm_text.return_value = """
```yaml
found_vcall:
  - insn_va: 0x12345678
    insn_disasm: call    [rax+68h]
    vfunc_offset: 0x68
```
""".strip()

        found_vcall = ida_vcall_finder.call_openai_for_vcalls(
            object(),
            {
                "object_name": "g_pNetworkMessages",
                "module": "networksystem",
                "platform": "linux",
                "func_name": "sub_2000",
                "func_va": "0x2000",
                "disasm_code": "call    [rax+68h]",
                "procedure": "obj->vfptr[13](obj);",
            },
            "gpt-4.1-mini",
        )

        self.assertEqual(
            [
                {
                    "insn_va": "0x12345678",
                    "insn_disasm": "call    [rax+68h]",
                    "vfunc_offset": "0x68",
                }
            ],
            found_vcall,
        )
        mock_call_llm_text.assert_called_once()
        self.assertEqual("gpt-4.1-mini", mock_call_llm_text.call_args.kwargs["model"])
        self.assertNotIn("temperature", mock_call_llm_text.call_args.kwargs)
        self.assertEqual(
            "You are a reverse engineering expert.",
            mock_call_llm_text.call_args.kwargs["messages"][0]["content"],
        )
        self.assertIn(
            "g_pNetworkMessages",
            mock_call_llm_text.call_args.kwargs["messages"][1]["content"],
        )


class TestAggregateVcallResultsForObject(unittest.TestCase):
    @patch("ida_vcall_finder.call_llm_text")
    @patch("ida_vcall_finder.create_openai_client")
    def test_aggregate_vcall_results_for_object_uses_shared_client_factory(
        self,
        mock_create_openai_client,
        mock_call_llm_text,
    ) -> None:
        mock_create_openai_client.return_value = object()
        mock_call_llm_text.return_value = """
found_vcall:
  - insn_va: 0x12345678
    insn_disasm: call    [rax+68h]
    vfunc_offset: 0x68
""".strip()

        with tempfile.TemporaryDirectory() as temp_dir:
            detail_path = ida_vcall_finder.build_vcall_detail_path(
                temp_dir,
                "14141b",
                "g_pNetworkMessages",
                "networksystem",
                "linux",
                "sub_2000",
            )
            ida_vcall_finder.write_vcall_detail_yaml(
                detail_path,
                {
                    "object_name": "g_pNetworkMessages",
                    "module": "networksystem",
                    "platform": "linux",
                    "func_name": "sub_2000",
                    "func_va": "0x2000",
                    "disasm_code": "call    [rax+68h]",
                    "procedure": "obj->vfptr[13](obj);",
                },
            )

            stats = ida_vcall_finder.aggregate_vcall_results_for_object(
                base_dir=temp_dir,
                gamever="14141b",
                object_name="g_pNetworkMessages",
                model="gpt-4.1-mini",
                api_key="test-api-key",
                base_url="https://example.invalid/v1",
                temperature=0.45,
                debug=False,
            )

            self.assertEqual({"status": "success", "processed": 1, "failed": 0}, stats)
            mock_create_openai_client.assert_called_once_with(
                api_key="test-api-key",
                base_url="https://example.invalid/v1",
                api_key_required_message="-llm_apikey is required when -vcall_finder is enabled",
            )
            saved_detail = ida_vcall_finder.load_yaml_file(detail_path)
            self.assertEqual(
                [
                    {
                        "insn_va": "0x12345678",
                        "insn_disasm": "call    [rax+68h]",
                        "vfunc_offset": "0x68",
                    }
                ],
                saved_detail["found_vcall"],
            )
            summary_text = ida_vcall_finder.build_vcall_summary_path(
                temp_dir,
                "14141b",
                "g_pNetworkMessages",
            ).read_text(encoding="utf-8")
            self.assertIn("insn_va: '0x12345678'", summary_text)
            self.assertIn("func_name: sub_2000", summary_text)
            self.assertEqual(0.45, mock_call_llm_text.call_args.kwargs["temperature"])


if __name__ == "__main__":
    unittest.main()
