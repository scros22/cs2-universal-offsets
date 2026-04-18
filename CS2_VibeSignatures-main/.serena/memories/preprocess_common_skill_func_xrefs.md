# preprocess_common_skill func_xrefs

## Summary
- `preprocess_common_skill` 只接受统一的 `func_xrefs`
- `func_xrefs` 现固定为六元组：
  `(func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)`
- `exclude_strings_list` 与 `xref_strings_list` 一样使用子串匹配

## Contract
- `xref_strings_list`、`xref_signatures_list`、`xref_funcs_list` 不能同时为空
- `exclude_funcs_list` 与 `exclude_strings_list` 可为空
- 旧 5 元组不再支持，命中后应直接视为非法配置

## Operational notes
- `exclude_funcs_list` 在正向交集后按 `func_va` 做差集
- `exclude_strings_list` 在正向交集后，按字符串 xref 所属函数集合并集做差集
- `exclude_strings_list` 若某个字符串没有命中任何函数，不视为失败，只视为空排除集
- `find-CBaseEntity_SetStateChanged.py` 的 Linux 路径使用
  `CNetworkTransmitComponent::StateChanged(%s) @%s:%d`
  作为内联 vcall 排除字符串
