import json
import tempfile
import unittest
from argparse import Namespace
from contextlib import asynccontextmanager
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import AsyncMock, MagicMock, patch

import yaml

import generate_reference_yaml


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


def _raw_py_eval_payload(result: object, *, stderr: str = "") -> _FakeCallToolResult:
    return _FakeCallToolResult(
        {
            "result": result,
            "stdout": "",
            "stderr": stderr,
        }
    )


def _write_yaml(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(yaml.safe_dump(payload, sort_keys=False), encoding="utf-8")


class _FakeStreamableHttpClient:
    def __init__(self) -> None:
        self.calls: list[dict[str, object]] = []

    @asynccontextmanager
    async def __call__(self, url: str, *, http_client: object):
        self.calls.append({"url": url, "http_client": http_client})
        yield ("read_stream", "write_stream", None)


class _FakeAsyncClient:
    instances: list["_FakeAsyncClient"] = []

    def __init__(self, **kwargs: object) -> None:
        self.kwargs = kwargs
        self.entered = False
        self.exited = False
        type(self).instances.append(self)

    async def __aenter__(self) -> "_FakeAsyncClient":
        self.entered = True
        return self

    async def __aexit__(self, exc_type, exc, tb) -> None:
        self.exited = True


class _FakeClientSession:
    instances: list["_FakeClientSession"] = []

    def __init__(self, read_stream: object, write_stream: object) -> None:
        self.read_stream = read_stream
        self.write_stream = write_stream
        self.initialized = False
        self.closed = False
        type(self).instances.append(self)

    async def __aenter__(self) -> "_FakeClientSession":
        return self

    async def __aexit__(self, exc_type, exc, tb) -> None:
        self.closed = True

    async def initialize(self) -> None:
        self.initialized = True


class _FakeClientSessionInitFailure(_FakeClientSession):
    async def initialize(self) -> None:
        raise RuntimeError("initialize failed")


def _make_fake_httpx() -> SimpleNamespace:
    _FakeAsyncClient.instances.clear()
    return SimpleNamespace(
        AsyncClient=_FakeAsyncClient,
        Timeout=lambda *args, **kwargs: {"args": args, "kwargs": kwargs},
    )


def _base_args(**overrides: object) -> Namespace:
    payload = {
        "gamever": "14141",
        "module": "engine",
        "platform": "windows",
        "func_name": "CNetworkMessages_FindNetworkGroup",
        "mcp_host": "127.0.0.1",
        "mcp_port": 13337,
        "ida_args": "--headless",
        "debug": False,
        "binary": None,
        "auto_start_mcp": False,
    }
    payload.update(overrides)
    return Namespace(**payload)


class TestReferenceYamlPureHelpers(unittest.TestCase):
    def test_build_reference_output_path_includes_module_and_platform(self) -> None:
        repo_root = Path("/repo")

        output_path = generate_reference_yaml.build_reference_output_path(
            repo_root=repo_root,
            module="engine",
            func_name="CNetworkMessages_FindNetworkGroup",
            platform="windows",
        )

        self.assertEqual(
            Path(
                "/repo/ida_preprocessor_scripts/references/engine/"
                "CNetworkMessages_FindNetworkGroup.windows.yaml"
            ),
            output_path,
        )

    def test_build_existing_yaml_path_uses_bin_gamever_module_func_platform(self) -> None:
        repo_root = Path("/repo")

        existing_yaml_path = generate_reference_yaml.build_existing_yaml_path(
            repo_root=repo_root,
            gamever="14141",
            module="engine",
            func_name="CNetworkMessages_FindNetworkGroup",
            platform="windows",
        )

        self.assertEqual(
            Path("/repo/bin/14141/engine/CNetworkMessages_FindNetworkGroup.windows.yaml"),
            existing_yaml_path,
        )

    def test_load_yaml_mapping_returns_empty_for_missing_and_empty_yaml(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            tmp_path = Path(temp_dir)
            missing_path = tmp_path / "missing.yaml"
            empty_yaml_path = tmp_path / "empty.yaml"
            empty_yaml_path.write_text("", encoding="utf-8")

            self.assertEqual({}, generate_reference_yaml.load_yaml_mapping(missing_path))
            self.assertEqual({}, generate_reference_yaml.load_yaml_mapping(empty_yaml_path))

    def test_load_yaml_mapping_rejects_non_mapping_root(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            yaml_path = Path(temp_dir) / "invalid.yaml"
            yaml_path.write_text("- item\n", encoding="utf-8")

            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError):
                generate_reference_yaml.load_yaml_mapping(yaml_path)

    def test_load_yaml_mapping_raises_for_yaml_syntax_error(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            yaml_path = Path(temp_dir) / "invalid_syntax.yaml"
            yaml_path.write_text("broken: [1, 2\n", encoding="utf-8")

            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError):
                generate_reference_yaml.load_yaml_mapping(yaml_path)

    def test_load_existing_func_va_reads_from_bin_path(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            yaml_path = (
                repo_root
                / "bin"
                / "14141"
                / "engine"
                / "CNetworkMessages_FindNetworkGroup.windows.yaml"
            )
            _write_yaml(yaml_path, {"func_va": "0x180123456"})

            func_va = generate_reference_yaml.load_existing_func_va(
                repo_root=repo_root,
                gamever="14141",
                module="engine",
                func_name="CNetworkMessages_FindNetworkGroup",
                platform="windows",
            )

            self.assertEqual("0x180123456", func_va)

    def test_load_existing_func_va_accepts_unquoted_yaml_integer_value(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            yaml_path = (
                repo_root
                / "bin"
                / "14141"
                / "engine"
                / "CNetworkMessages_FindNetworkGroup.windows.yaml"
            )
            _write_yaml(yaml_path, {"func_va": 0x180123450})

            func_va = generate_reference_yaml.load_existing_func_va(
                repo_root=repo_root,
                gamever="14141",
                module="engine",
                func_name="CNetworkMessages_FindNetworkGroup",
                platform="windows",
            )

            self.assertEqual("0x180123450", func_va)

    def test_load_existing_func_va_returns_none_for_missing_yaml(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            func_va = generate_reference_yaml.load_existing_func_va(
                repo_root=Path(temp_dir),
                gamever="14141",
                module="engine",
                func_name="CNetworkMessages_FindNetworkGroup",
                platform="windows",
            )

            self.assertIsNone(func_va)

    def test_load_existing_func_va_returns_none_for_unparseable_value(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            yaml_path = (
                repo_root
                / "bin"
                / "14141"
                / "engine"
                / "CNetworkMessages_FindNetworkGroup.windows.yaml"
            )
            _write_yaml(yaml_path, {"func_va": "not-an-address"})

            func_va = generate_reference_yaml.load_existing_func_va(
                repo_root=repo_root,
                gamever="14141",
                module="engine",
                func_name="CNetworkMessages_FindNetworkGroup",
                platform="windows",
            )

            self.assertIsNone(func_va)

    def test_load_symbol_aliases_collects_name_then_alias(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            config_path = repo_root / "config.yaml"
            _write_yaml(
                config_path,
                {
                    "modules": [
                        {
                            "name": "engine",
                            "symbols": [
                                {
                                    "name": "CNetworkMessages_FindNetworkGroup",
                                    "alias": [
                                        "FindNetworkGroupAliasA",
                                        "FindNetworkGroupAliasB",
                                    ],
                                }
                            ],
                        }
                    ]
                },
            )

            aliases = generate_reference_yaml.load_symbol_aliases(
                repo_root=repo_root,
                module="engine",
                func_name="CNetworkMessages_FindNetworkGroup",
            )

            self.assertEqual(
                [
                    "CNetworkMessages_FindNetworkGroup",
                    "FindNetworkGroupAliasA",
                    "FindNetworkGroupAliasB",
                ],
                aliases,
            )

    def test_load_symbol_aliases_raises_when_symbol_missing(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            config_path = repo_root / "config.yaml"
            _write_yaml(
                config_path,
                {
                    "modules": [
                        {
                            "name": "engine",
                            "symbols": [{"name": "OtherFunc", "alias": "OtherAlias"}],
                        }
                    ]
                },
            )

            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError):
                generate_reference_yaml.load_symbol_aliases(
                    repo_root=repo_root,
                    module="engine",
                    func_name="CNetworkMessages_FindNetworkGroup",
                )

    def test_load_symbol_aliases_raises_when_config_missing_or_modules_not_list(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)

            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError):
                generate_reference_yaml.load_symbol_aliases(
                    repo_root=repo_root,
                    module="engine",
                    func_name="CNetworkMessages_FindNetworkGroup",
                )

            _write_yaml(repo_root / "config.yaml", {"modules": {"name": "engine"}})
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError):
                generate_reference_yaml.load_symbol_aliases(
                    repo_root=repo_root,
                    module="engine",
                    func_name="CNetworkMessages_FindNetworkGroup",
                )

    def test_load_symbol_aliases_raises_when_target_module_symbols_not_list(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            _write_yaml(
                repo_root / "config.yaml",
                {
                    "modules": [
                        {
                            "name": "engine",
                            "symbols": {"name": "CNetworkMessages_FindNetworkGroup"},
                        }
                    ]
                },
            )

            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError):
                generate_reference_yaml.load_symbol_aliases(
                    repo_root=repo_root,
                    module="engine",
                    func_name="CNetworkMessages_FindNetworkGroup",
                )

    def test_parse_args_requires_func_name_only(self) -> None:
        with self.assertRaises(SystemExit):
            generate_reference_yaml.parse_args([])

    def test_parse_args_allows_missing_target_inputs(self) -> None:
        args = generate_reference_yaml.parse_args(
            [
                "-func_name",
                "CNetworkMessages_FindNetworkGroup",
            ]
        )

        self.assertIsNone(args.gamever)
        self.assertIsNone(args.module)
        self.assertIsNone(args.platform)
        self.assertEqual("CNetworkMessages_FindNetworkGroup", args.func_name)
        self.assertFalse(args.auto_start_mcp)
        self.assertIsNone(args.binary)

    def test_parse_args_requires_binary_with_auto_start_mcp(self) -> None:
        with self.assertRaises(SystemExit):
            generate_reference_yaml.parse_args(
                [
                    "-gamever",
                    "14141",
                    "-module",
                    "engine",
                    "-platform",
                    "windows",
                    "-func_name",
                    "CNetworkMessages_FindNetworkGroup",
                    "-auto_start_mcp",
                ]
            )

    def test_parse_args_requires_auto_start_mcp_with_binary(self) -> None:
        with self.assertRaises(SystemExit):
            generate_reference_yaml.parse_args(
                [
                    "-gamever",
                    "14141",
                    "-module",
                    "engine",
                    "-platform",
                    "windows",
                    "-func_name",
                    "CNetworkMessages_FindNetworkGroup",
                    "-binary",
                    "/tmp/server.dll",
                ]
            )

    def test_parse_args_accepts_pair_required_inputs_and_defaults(self) -> None:
        args = generate_reference_yaml.parse_args(
            [
                "-gamever",
                "14141",
                "-module",
                "engine",
                "-platform",
                "linux",
                "-func_name",
                "CNetworkMessages_FindNetworkGroup",
                "-auto_start_mcp",
                "-binary",
                "/tmp/server.so",
            ]
        )

        self.assertEqual("14141", args.gamever)
        self.assertEqual("engine", args.module)
        self.assertEqual("linux", args.platform)
        self.assertEqual("CNetworkMessages_FindNetworkGroup", args.func_name)
        self.assertTrue(args.auto_start_mcp)
        self.assertEqual("/tmp/server.so", args.binary)
        self.assertEqual("127.0.0.1", args.mcp_host)
        self.assertEqual(13337, args.mcp_port)
        self.assertEqual("", args.ida_args)
        self.assertFalse(args.debug)

    def test_infer_target_from_binary_path_extracts_gamever_module_and_platform(self) -> None:
        windows_target = generate_reference_yaml.infer_target_from_binary_path(
            r"D:\CS2_VibeSignatures\bin\14141c\engine\engine2.dll.i64"
        )
        linux_target = generate_reference_yaml.infer_target_from_binary_path(
            "/mnt/d/CS2_VibeSignatures/bin/14141c/server/libserver.so"
        )

        self.assertEqual(
            {"gamever": "14141c", "module": "engine", "platform": "windows"},
            windows_target,
        )
        self.assertEqual(
            {"gamever": "14141c", "module": "server", "platform": "linux"},
            linux_target,
        )

    def test_infer_target_from_binary_path_raises_for_unexpected_layout(self) -> None:
        with self.assertRaises(generate_reference_yaml.ReferenceGenerationError):
            generate_reference_yaml.infer_target_from_binary_path("/tmp/engine2.dll")


class TestResolveFuncVa(unittest.IsolatedAsyncioTestCase):
    async def test_resolve_func_va_uses_existing_yaml_before_ida_lookup(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            _write_yaml(
                repo_root
                / "bin"
                / "14141"
                / "engine"
                / "CNetworkMessages_FindNetworkGroup.windows.yaml",
                {"func_va": "0x180123450"},
            )
            session = AsyncMock()

            func_va = await generate_reference_yaml.resolve_func_va(
                session=session,
                repo_root=repo_root,
                gamever="14141",
                module="engine",
                platform="windows",
                func_name="CNetworkMessages_FindNetworkGroup",
                debug=False,
            )

            self.assertEqual("0x180123450", func_va)
            session.call_tool.assert_not_called()

    async def test_resolve_func_va_falls_back_to_config_aliases(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            _write_yaml(
                repo_root / "config.yaml",
                {
                    "modules": [
                        {
                            "name": "engine",
                            "symbols": [
                                {
                                    "name": "CNetworkMessages_FindNetworkGroup",
                                    "alias": ["FindNetworkGroupAlias"],
                                }
                            ],
                        }
                    ]
                },
            )
            session = AsyncMock()
            session.call_tool.return_value = _py_eval_payload(
                [
                    {
                        "name": "FindNetworkGroupAlias",
                        "func_va": "0x180123456",
                    }
                ]
            )

            func_va = await generate_reference_yaml.resolve_func_va(
                session=session,
                repo_root=repo_root,
                gamever="14141",
                module="engine",
                platform="windows",
                func_name="CNetworkMessages_FindNetworkGroup",
                debug=False,
            )

            self.assertEqual("0x180123456", func_va)
            session.call_tool.assert_awaited_once()
            py_code = session.call_tool.await_args.kwargs["arguments"]["code"]
            self.assertIn("CNetworkMessages_FindNetworkGroup", py_code)
            self.assertIn("FindNetworkGroupAlias", py_code)

    async def test_resolve_func_va_raises_on_ambiguous_matches(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            _write_yaml(
                repo_root / "config.yaml",
                {
                    "modules": [
                        {
                            "name": "engine",
                            "symbols": [
                                {
                                    "name": "CNetworkMessages_FindNetworkGroup",
                                    "alias": ["FindNetworkGroupAlias"],
                                }
                            ],
                        }
                    ]
                },
            )
            session = AsyncMock()
            session.call_tool.return_value = _py_eval_payload(
                [
                    {
                        "name": "CNetworkMessages_FindNetworkGroup",
                        "func_va": "0x180123450",
                    },
                    {
                        "name": "FindNetworkGroupAlias",
                        "func_va": "0x180123456",
                    },
                ]
            )

            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                await generate_reference_yaml.resolve_func_va(
                    session=session,
                    repo_root=repo_root,
                    gamever="14141",
                    module="engine",
                    platform="windows",
                    func_name="CNetworkMessages_FindNetworkGroup",
                    debug=False,
                )

            self.assertEqual(
                "ambiguous function address matches returned via IDA: "
                "['0x180123450', '0x180123456']",
                str(ctx.exception),
            )


class TestFindFunctionAddrByNames(unittest.IsolatedAsyncioTestCase):
    async def test_find_function_addr_by_names_raises_with_candidate_address_list(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            [
                {
                    "name": "CNetworkMessages_FindNetworkGroup",
                    "func_va": "0x180123450",
                },
                {
                    "name": "FindNetworkGroupAlias",
                    "func_va": "0x180123456",
                },
            ]
        )

        with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
            await generate_reference_yaml.find_function_addr_by_names(
                session,
                [
                    "CNetworkMessages_FindNetworkGroup",
                    "FindNetworkGroupAlias",
                ],
                debug=False,
            )

        self.assertEqual(
            "ambiguous function address matches returned via IDA: "
            "['0x180123450', '0x180123456']",
            str(ctx.exception),
        )

    async def test_find_function_addr_by_names_ignores_invalid_func_va_entries(self) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            [
                {
                    "name": "BrokenAlias",
                    "func_va": None,
                },
                {
                    "name": "FindNetworkGroupAlias",
                    "func_va": "0x180123456",
                },
            ]
        )

        func_va = await generate_reference_yaml.find_function_addr_by_names(
            session,
            [
                "BrokenAlias",
                "FindNetworkGroupAlias",
            ],
            debug=False,
        )

        self.assertEqual("0x180123456", func_va)


class TestParsePyEvalJsonResult(unittest.TestCase):
    def test_parse_py_eval_json_result_raises_when_result_missing_or_empty(self) -> None:
        for payload in (
            {"stdout": "", "stderr": ""},
            {"result": "", "stdout": "", "stderr": ""},
        ):
            with self.subTest(payload=payload):
                with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                    generate_reference_yaml._parse_py_eval_json_result(
                        _FakeCallToolResult(payload),
                    )

                self.assertEqual("missing py_eval result from IDA", str(ctx.exception))

    def test_parse_py_eval_json_result_raises_when_result_not_json(self) -> None:
        with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
            generate_reference_yaml._parse_py_eval_json_result(
                _raw_py_eval_payload("not-json"),
            )

        self.assertEqual("invalid py_eval JSON payload from IDA", str(ctx.exception))


class TestExportReferencePayload(unittest.IsolatedAsyncioTestCase):
    async def test_export_reference_payload_keeps_empty_procedure_when_hexrays_missing(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "func_name": "sub_180123450",
                "func_va": "0x180123450",
                "disasm_code": "text:180123450 push rbp",
                "procedure": "",
                "extra_field": "ignored",
            }
        )

        payload = await generate_reference_yaml.export_reference_payload_via_mcp(
            session=session,
            func_name="CNetworkMessages_FindNetworkGroup",
            func_va="0x180123450",
            debug=False,
        )

        self.assertEqual(
            {
                "func_name": "CNetworkMessages_FindNetworkGroup",
                "func_va": "0x180123450",
                "disasm_code": "text:180123450 push rbp",
                "procedure": "",
            },
            payload,
        )
        session.call_tool.assert_awaited_once()

    async def test_export_reference_payload_raises_when_func_va_is_none_or_invalid(self) -> None:
        for exported in (
            {
                "func_name": "sub_180123450",
                "func_va": None,
                "disasm_code": "text:180123450 push rbp",
                "procedure": "",
            },
            {
                "func_name": "sub_180123450",
                "func_va": "bad-va",
                "disasm_code": "text:180123450 push rbp",
                "procedure": "",
            },
        ):
            with self.subTest(exported=exported):
                session = AsyncMock()
                session.call_tool.return_value = _py_eval_payload(exported)

                with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                    await generate_reference_yaml.export_reference_payload_via_mcp(
                        session=session,
                        func_name="CNetworkMessages_FindNetworkGroup",
                        func_va="0x180123450",
                        debug=False,
                    )

                self.assertEqual(
                    "unable to export reference payload via IDA",
                    str(ctx.exception),
                )

    async def test_export_reference_payload_raises_when_disasm_code_empty_or_none(self) -> None:
        for exported in (
            {
                "func_name": "sub_180123450",
                "func_va": "0x180123450",
                "disasm_code": "",
                "procedure": "",
            },
            {
                "func_name": "sub_180123450",
                "func_va": "0x180123450",
                "disasm_code": None,
                "procedure": "",
            },
        ):
            with self.subTest(exported=exported):
                session = AsyncMock()
                session.call_tool.return_value = _py_eval_payload(exported)

                with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                    await generate_reference_yaml.export_reference_payload_via_mcp(
                        session=session,
                        func_name="CNetworkMessages_FindNetworkGroup",
                        func_va="0x180123450",
                        debug=False,
                    )

                self.assertEqual(
                    "unable to export reference payload via IDA",
                    str(ctx.exception),
                )


class TestWriteReferenceYaml(unittest.TestCase):
    def test_write_reference_yaml_writes_minimal_schema_only(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            output_path = Path(temp_dir) / "refs" / "func.yaml"

            generate_reference_yaml.write_reference_yaml(
                output_path,
                {
                    "func_name": "CNetworkMessages_FindNetworkGroup",
                    "func_va": "0x180123450",
                    "disasm_code": "text:180123450 push rbp",
                    "procedure": "",
                    "extra_field": "ignored",
                },
            )

            written = yaml.safe_load(output_path.read_text(encoding="utf-8"))
            self.assertEqual(
                {
                    "func_name": "CNetworkMessages_FindNetworkGroup",
                    "func_va": "0x180123450",
                    "disasm_code": "text:180123450 push rbp",
                    "procedure": "",
                },
                written,
            )

    def test_write_reference_yaml_raises_for_missing_required_field_without_writing_file(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            output_path = Path(temp_dir) / "refs" / "func.yaml"

            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError):
                generate_reference_yaml.write_reference_yaml(
                    output_path,
                    {
                        "func_name": "CNetworkMessages_FindNetworkGroup",
                        "func_va": "0x180123450",
                        "procedure": "",
                    },
                )

            self.assertFalse(output_path.exists())


class TestMcpSessionModes(unittest.IsolatedAsyncioTestCase):
    async def test_open_mcp_session_raises_reference_generation_error_when_open_fails(self) -> None:
        @asynccontextmanager
        async def _failing_streamable_http_client(url: str, *, http_client: object):
            raise RuntimeError("open failed")
            yield ("read_stream", "write_stream", None)

        with (
            patch.object(generate_reference_yaml, "httpx", _make_fake_httpx(), create=True),
            patch.object(
                generate_reference_yaml,
                "streamable_http_client",
                _failing_streamable_http_client,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "ClientSession",
                _FakeClientSession,
                create=True,
            ),
        ):
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                async with generate_reference_yaml._open_mcp_session("127.0.0.1", 13337):
                    self.fail("_open_mcp_session should not yield on open failure")

        self.assertEqual(
            "unable to open MCP session at 127.0.0.1:13337",
            str(ctx.exception),
        )

    async def test_open_mcp_session_raises_reference_generation_error_when_initialize_fails(
        self,
    ) -> None:
        fake_stream_client = _FakeStreamableHttpClient()

        with (
            patch.object(generate_reference_yaml, "httpx", _make_fake_httpx(), create=True),
            patch.object(
                generate_reference_yaml,
                "streamable_http_client",
                fake_stream_client,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "ClientSession",
                _FakeClientSessionInitFailure,
                create=True,
            ),
        ):
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                async with generate_reference_yaml._open_mcp_session("127.0.0.1", 13337):
                    self.fail("_open_mcp_session should not yield on initialize failure")

        self.assertEqual(
            "unable to open MCP session at 127.0.0.1:13337",
            str(ctx.exception),
        )

    async def test_attach_existing_mcp_session_checks_health_first(self) -> None:
        _FakeClientSession.instances.clear()
        fake_stream_client = _FakeStreamableHttpClient()
        call_order: list[str] = []

        async def _fake_check_mcp_health(host: str, port: int) -> bool:
            call_order.append(f"health:{host}:{port}")
            return True

        with (
            patch.object(
                generate_reference_yaml,
                "check_mcp_health",
                AsyncMock(side_effect=_fake_check_mcp_health),
                create=True,
            ) as check_health,
            patch.object(generate_reference_yaml, "httpx", _make_fake_httpx(), create=True),
            patch.object(
                generate_reference_yaml,
                "streamable_http_client",
                fake_stream_client,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "ClientSession",
                _FakeClientSession,
                create=True,
            ),
        ):
            async with generate_reference_yaml.attach_existing_mcp_session(
                host="127.0.0.1",
                port=13337,
                debug=True,
            ) as session:
                self.assertIsInstance(session, _FakeClientSession)
                call_order.append("session-opened")

        self.assertEqual(["health:127.0.0.1:13337", "session-opened"], call_order)
        check_health.assert_awaited_once_with("127.0.0.1", 13337)
        self.assertEqual("http://127.0.0.1:13337/mcp", fake_stream_client.calls[0]["url"])
        self.assertEqual(
            {
                "args": (30.0,),
                "kwargs": {"read": 300.0},
            },
            _FakeAsyncClient.instances[0].kwargs["timeout"],
        )
        self.assertTrue(_FakeAsyncClient.instances[0].kwargs["follow_redirects"])
        self.assertFalse(_FakeAsyncClient.instances[0].kwargs["trust_env"])
        self.assertTrue(_FakeClientSession.instances[0].initialized)

    async def test_attach_existing_mcp_session_raises_when_health_check_fails(self) -> None:
        with patch.object(
            generate_reference_yaml,
            "check_mcp_health",
            AsyncMock(return_value=False),
            create=True,
        ) as check_health:
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                async with generate_reference_yaml.attach_existing_mcp_session(
                    host="127.0.0.1",
                    port=13337,
                    debug=False,
                ):
                    self.fail("attach_existing_mcp_session should not yield when health check fails")

        self.assertEqual(
            "MCP server is not reachable at 127.0.0.1:13337",
            str(ctx.exception),
        )
        check_health.assert_awaited_once_with("127.0.0.1", 13337)

    async def test_autostart_mcp_session_starts_and_quits_process(self) -> None:
        _FakeClientSession.instances.clear()
        fake_stream_client = _FakeStreamableHttpClient()
        process = MagicMock(name="fake_process")

        with (
            patch.object(
                generate_reference_yaml,
                "start_idalib_mcp",
                return_value=process,
                create=True,
            ) as start_idalib_mcp,
            patch.object(
                generate_reference_yaml,
                "quit_ida_gracefully_async",
                new_callable=AsyncMock,
                create=True,
            ) as quit_ida_gracefully_async,
            patch.object(generate_reference_yaml, "httpx", _make_fake_httpx(), create=True),
            patch.object(
                generate_reference_yaml,
                "streamable_http_client",
                fake_stream_client,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "ClientSession",
                _FakeClientSession,
                create=True,
            ),
        ):
            async with generate_reference_yaml.autostart_mcp_session(
                binary_path="/tmp/server.dll",
                host="127.0.0.1",
                port=13337,
                ida_args="--headless",
                debug=True,
            ) as session:
                self.assertIsInstance(session, _FakeClientSession)

        start_idalib_mcp.assert_called_once_with(
            "/tmp/server.dll",
            "127.0.0.1",
            13337,
            "--headless",
            True,
        )
        quit_ida_gracefully_async.assert_awaited_once_with(
            process,
            "127.0.0.1",
            13337,
            debug=True,
        )
        self.assertEqual("http://127.0.0.1:13337/mcp", fake_stream_client.calls[0]["url"])
        self.assertTrue(_FakeClientSession.instances[0].initialized)

    async def test_autostart_mcp_session_raises_when_start_returns_none(self) -> None:
        with patch.object(
            generate_reference_yaml,
            "start_idalib_mcp",
            return_value=None,
            create=True,
        ) as start_idalib_mcp:
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                async with generate_reference_yaml.autostart_mcp_session(
                    binary_path="/tmp/server.dll",
                    host="127.0.0.1",
                    port=13337,
                    ida_args="--headless",
                    debug=False,
                ):
                    self.fail("autostart_mcp_session should not yield when process start fails")

        self.assertEqual(
            "failed to start idalib-mcp for /tmp/server.dll",
            str(ctx.exception),
        )
        start_idalib_mcp.assert_called_once_with(
            "/tmp/server.dll",
            "127.0.0.1",
            13337,
            "--headless",
            False,
        )

    async def test_autostart_mcp_session_quits_process_when_session_body_raises(self) -> None:
        _FakeClientSession.instances.clear()
        fake_stream_client = _FakeStreamableHttpClient()
        process = MagicMock(name="fake_process")

        with (
            patch.object(
                generate_reference_yaml,
                "start_idalib_mcp",
                return_value=process,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "quit_ida_gracefully_async",
                new_callable=AsyncMock,
                create=True,
            ) as quit_ida_gracefully_async,
            patch.object(generate_reference_yaml, "httpx", _make_fake_httpx(), create=True),
            patch.object(
                generate_reference_yaml,
                "streamable_http_client",
                fake_stream_client,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "ClientSession",
                _FakeClientSession,
                create=True,
            ),
        ):
            with self.assertRaises(RuntimeError) as ctx:
                async with generate_reference_yaml.autostart_mcp_session(
                    binary_path="/tmp/server.dll",
                    host="127.0.0.1",
                    port=13337,
                    ida_args="--headless",
                    debug=True,
                ):
                    raise RuntimeError("boom")

        self.assertEqual("boom", str(ctx.exception))
        quit_ida_gracefully_async.assert_awaited_once_with(
            process,
            "127.0.0.1",
            13337,
            debug=True,
        )

    async def test_autostart_mcp_session_quits_process_when_open_session_fails(self) -> None:
        process = MagicMock(name="fake_process")

        @asynccontextmanager
        async def _failing_open_mcp_session(host: str, port: int):
            raise generate_reference_yaml.ReferenceGenerationError(
                f"unable to open MCP session at {host}:{port}"
            )
            yield None

        with (
            patch.object(
                generate_reference_yaml,
                "start_idalib_mcp",
                return_value=process,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "_open_mcp_session",
                side_effect=_failing_open_mcp_session,
                create=True,
            ) as open_mcp_session,
            patch.object(
                generate_reference_yaml,
                "quit_ida_gracefully_async",
                new_callable=AsyncMock,
                create=True,
            ) as quit_ida_gracefully_async,
        ):
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                async with generate_reference_yaml.autostart_mcp_session(
                    binary_path="/tmp/server.dll",
                    host="127.0.0.1",
                    port=13337,
                    ida_args="--headless",
                    debug=False,
                ):
                    self.fail("autostart_mcp_session should not yield when open session fails")

        self.assertEqual(
            "unable to open MCP session at 127.0.0.1:13337",
            str(ctx.exception),
        )
        open_mcp_session.assert_called_once_with("127.0.0.1", 13337)
        quit_ida_gracefully_async.assert_awaited_once_with(
            process,
            "127.0.0.1",
            13337,
            debug=False,
        )


class TestIdaAnalyzeBinWrappers(unittest.IsolatedAsyncioTestCase):
    async def test_check_mcp_health_wraps_system_exit_as_reference_generation_error(self) -> None:
        with patch.object(
            generate_reference_yaml.importlib,
            "import_module",
            side_effect=SystemExit(2),
        ):
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                await generate_reference_yaml.check_mcp_health("127.0.0.1", 13337)

        self.assertEqual("failed to import ida_analyze_bin helpers", str(ctx.exception))

    async def test_start_idalib_mcp_wraps_system_exit_as_reference_generation_error(self) -> None:
        with patch.object(
            generate_reference_yaml.importlib,
            "import_module",
            side_effect=SystemExit(2),
        ):
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                generate_reference_yaml.start_idalib_mcp(
                    "/tmp/server.dll",
                    "127.0.0.1",
                    13337,
                    "--headless",
                    False,
                )

        self.assertEqual("failed to import ida_analyze_bin helpers", str(ctx.exception))

    async def test_quit_ida_gracefully_wraps_system_exit_as_reference_generation_error(self) -> None:
        with patch.object(
            generate_reference_yaml.importlib,
            "import_module",
            side_effect=SystemExit(2),
        ):
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                generate_reference_yaml.quit_ida_gracefully(
                    MagicMock(name="fake_process"),
                    "127.0.0.1",
                    13337,
                    debug=False,
                )

        self.assertEqual("failed to import ida_analyze_bin helpers", str(ctx.exception))

    async def test_quit_ida_gracefully_async_wraps_system_exit_as_reference_generation_error(
        self,
    ) -> None:
        with patch.object(
            generate_reference_yaml.importlib,
            "import_module",
            side_effect=SystemExit(2),
        ):
            with self.assertRaises(generate_reference_yaml.ReferenceGenerationError) as ctx:
                await generate_reference_yaml.quit_ida_gracefully_async(
                    MagicMock(name="fake_process"),
                    "127.0.0.1",
                    13337,
                    debug=False,
                )

        self.assertEqual("failed to import ida_analyze_bin helpers", str(ctx.exception))


class TestRunReferenceGeneration(unittest.IsolatedAsyncioTestCase):
    async def test_run_reference_generation_uses_attach_mode_by_default(self) -> None:
        fake_session = object()
        output_path = Path("/repo/out/reference.yaml")
        exported_payload = {
            "func_name": "CNetworkMessages_FindNetworkGroup",
            "func_va": "0x180123450",
            "disasm_code": "text:180123450 push rbp",
            "procedure": "",
        }
        call_order: list[str] = []

        @asynccontextmanager
        async def _fake_attach_existing_mcp_session(**kwargs):
            self.assertEqual({"host": "127.0.0.1", "port": 13337, "debug": False}, kwargs)
            yield fake_session

        async def _fake_resolve_func_va(*args, **kwargs):
            call_order.append("resolve_func_va")
            self.assertIs(args[0], fake_session)
            return "0x180123450"

        async def _fake_export_reference_payload_via_mcp(*args, **kwargs):
            call_order.append("export_reference_payload_via_mcp")
            self.assertIs(args[0], fake_session)
            self.assertEqual("0x180123450", kwargs["func_va"])
            return exported_payload

        def _fake_build_reference_output_path(*args):
            call_order.append("build_reference_output_path")
            return output_path

        def _fake_write_reference_yaml(*args):
            call_order.append("write_reference_yaml")

        with (
            patch.object(
                generate_reference_yaml,
                "attach_existing_mcp_session",
                _fake_attach_existing_mcp_session,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "autostart_mcp_session",
                create=True,
            ) as autostart_mcp_session,
            patch.object(
                generate_reference_yaml,
                "resolve_func_va",
                AsyncMock(side_effect=_fake_resolve_func_va),
                create=True,
            ) as resolve_func_va,
            patch.object(
                generate_reference_yaml,
                "export_reference_payload_via_mcp",
                AsyncMock(side_effect=_fake_export_reference_payload_via_mcp),
                create=True,
            ) as export_reference_payload_via_mcp,
            patch.object(
                generate_reference_yaml,
                "build_reference_output_path",
                side_effect=_fake_build_reference_output_path,
            ) as build_reference_output_path,
            patch.object(
                generate_reference_yaml,
                "write_reference_yaml",
                side_effect=_fake_write_reference_yaml,
            ) as write_reference_yaml,
        ):
            result = await generate_reference_yaml.run_reference_generation(
                _base_args(),
                repo_root=Path("/repo"),
            )

        self.assertEqual(output_path, result)
        self.assertEqual(
            [
                "resolve_func_va",
                "export_reference_payload_via_mcp",
                "build_reference_output_path",
                "write_reference_yaml",
            ],
            call_order,
        )
        autostart_mcp_session.assert_not_called()
        resolve_func_va.assert_awaited_once_with(
            fake_session,
            repo_root=Path("/repo"),
            gamever="14141",
            module="engine",
            platform="windows",
            func_name="CNetworkMessages_FindNetworkGroup",
            debug=False,
        )
        export_reference_payload_via_mcp.assert_awaited_once_with(
            fake_session,
            func_name="CNetworkMessages_FindNetworkGroup",
            func_va="0x180123450",
            debug=False,
        )
        build_reference_output_path.assert_called_once_with(
            Path("/repo"),
            "engine",
            "CNetworkMessages_FindNetworkGroup",
            "windows",
        )
        write_reference_yaml.assert_called_once_with(output_path, exported_payload)

    async def test_run_reference_generation_uses_autostart_mode_when_enabled(self) -> None:
        fake_session = object()
        output_path = Path("/repo/out/reference.yaml")

        @asynccontextmanager
        async def _fake_autostart_mcp_session(**kwargs):
            self.assertEqual(
                {
                    "binary_path": "/tmp/server.dll",
                    "host": "127.0.0.1",
                    "port": 13337,
                    "ida_args": "--headless",
                    "debug": True,
                },
                kwargs,
            )
            yield fake_session

        with (
            patch.object(
                generate_reference_yaml,
                "attach_existing_mcp_session",
                create=True,
            ) as attach_existing_mcp_session,
            patch.object(
                generate_reference_yaml,
                "autostart_mcp_session",
                _fake_autostart_mcp_session,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "resolve_func_va",
                AsyncMock(return_value="0x180123450"),
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "export_reference_payload_via_mcp",
                AsyncMock(
                    return_value={
                        "func_name": "CNetworkMessages_FindNetworkGroup",
                        "func_va": "0x180123450",
                        "disasm_code": "text:180123450 push rbp",
                        "procedure": "",
                    }
                ),
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "build_reference_output_path",
                return_value=output_path,
            ),
            patch.object(generate_reference_yaml, "write_reference_yaml"),
        ):
            result = await generate_reference_yaml.run_reference_generation(
                _base_args(
                    auto_start_mcp=True,
                    binary="/tmp/server.dll",
                    debug=True,
                ),
                repo_root=Path("/repo"),
            )

        self.assertEqual(output_path, result)
        attach_existing_mcp_session.assert_not_called()

    async def test_run_reference_generation_fills_missing_inputs_from_survey_binary(self) -> None:
        fake_session = MagicMock()
        fake_session.call_tool = AsyncMock(
            return_value=_FakeCallToolResult(
                {
                    "metadata": {
                        "path": r"D:\CS2_VibeSignatures\bin\14141c\engine\engine2.dll.i64"
                    }
                }
            )
        )
        output_path = Path("/repo/out/reference.yaml")

        @asynccontextmanager
        async def _fake_attach_existing_mcp_session(**kwargs):
            self.assertEqual({"host": "127.0.0.1", "port": 13337, "debug": False}, kwargs)
            yield fake_session

        with (
            patch.object(
                generate_reference_yaml,
                "attach_existing_mcp_session",
                _fake_attach_existing_mcp_session,
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "survey_binary_via_mcp",
                AsyncMock(),
                create=True,
            ) as survey_binary_via_mcp,
            patch.object(
                generate_reference_yaml,
                "resolve_func_va",
                AsyncMock(return_value="0x180123450"),
                create=True,
            ) as resolve_func_va,
            patch.object(
                generate_reference_yaml,
                "export_reference_payload_via_mcp",
                AsyncMock(
                    return_value={
                        "func_name": "CNetworkMessages_FindNetworkGroup",
                        "func_va": "0x180123450",
                        "disasm_code": "text:180123450 push rbp",
                        "procedure": "",
                    }
                ),
                create=True,
            ),
            patch.object(
                generate_reference_yaml,
                "build_reference_output_path",
                return_value=output_path,
            ) as build_reference_output_path,
            patch.object(generate_reference_yaml, "write_reference_yaml"),
        ):
            result = await generate_reference_yaml.run_reference_generation(
                _base_args(
                    gamever=None,
                    module=None,
                    platform=None,
                ),
                repo_root=Path("/repo"),
            )

        self.assertEqual(output_path, result)
        fake_session.call_tool.assert_awaited_once_with(
            name="survey_binary",
            arguments={"detail_level": "minimal"},
        )
        survey_binary_via_mcp.assert_not_called()
        resolve_func_va.assert_awaited_once_with(
            fake_session,
            repo_root=Path("/repo"),
            gamever="14141c",
            module="engine",
            platform="windows",
            func_name="CNetworkMessages_FindNetworkGroup",
            debug=False,
        )
        build_reference_output_path.assert_called_once_with(
            Path("/repo"),
            "engine",
            "CNetworkMessages_FindNetworkGroup",
            "windows",
        )


class TestMain(unittest.TestCase):
    def test_main_returns_zero_and_prints_generated_path(self) -> None:
        args = _base_args()
        output_path = Path("/tmp/reference.yaml")

        def _fake_asyncio_run(coro):
            coro.close()
            return output_path

        fake_asyncio = SimpleNamespace(run=MagicMock(side_effect=_fake_asyncio_run))

        with (
            patch.object(generate_reference_yaml, "parse_args", return_value=args),
            patch.object(generate_reference_yaml, "asyncio", fake_asyncio, create=True),
            patch("builtins.print") as fake_print,
        ):
            exit_code = generate_reference_yaml.main(["--ignored"])

        self.assertEqual(0, exit_code)
        fake_print.assert_called_once_with(f"Generated reference YAML: {output_path}")

    def test_main_returns_one_and_prints_error_message(self) -> None:
        args = _base_args()

        def _fake_asyncio_run(coro):
            coro.close()
            raise generate_reference_yaml.ReferenceGenerationError("boom")

        fake_asyncio = SimpleNamespace(run=MagicMock(side_effect=_fake_asyncio_run))

        with (
            patch.object(generate_reference_yaml, "parse_args", return_value=args),
            patch.object(generate_reference_yaml, "asyncio", fake_asyncio, create=True),
            patch("builtins.print") as fake_print,
        ):
            exit_code = generate_reference_yaml.main(["--ignored"])

        self.assertEqual(1, exit_code)
        fake_print.assert_called_once_with("ERROR: boom")
