# IDA Pro Power

Connect Kiro to IDA Pro for binary analysis and reverse engineering.

## Features

- Analyze binary files loaded in IDA Pro
- Search for patterns and signatures
- Decompile functions
- Find cross-references
- Analyze function parameters and calling conventions

## Setup

1. Install the IDA MCP server plugin in your IDA Pro plugins directory
2. Ensure Python is available in your PATH
3. Configure the IDA_PATH environment variable to point to your IDA Pro installation

## Usage

Once activated, you can ask Kiro to:
- "Find the SetModel function in client.dll"
- "Decompile the function at address 0x12345678"
- "Search for the pattern 40 53 48 83 EC"
- "Show me cross-references to this function"

## Requirements

- IDA Pro 9.0 or later
- Python 3.8+
- IDA MCP server plugin installed
