# Style and conventions

Python style:
- PEP8-ish formatting and naming.
- snake_case functions/variables, ALL_CAPS constants.
- Module and function docstrings describe purpose and usage.
- argparse used for CLI definitions; f-strings for messages.
- Explicit error handling prints messages and exits with non-zero status.
- UTF-8 encoding used when reading/writing text files.

YAML config:
- config.yaml organizes modules, platform-specific paths, and symbol metadata (category, alias, prerequisites).
