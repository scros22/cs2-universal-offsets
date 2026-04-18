# CLAUDE.md

This file provides guidance and important rules working with code in this repository.

## When coding / building plan

 - Use a progressive disclosure approach for agent coding in this repository: start from high-level information in Serena memories, and only locate/read specific files or symbols when necessary to avoid expanding too much context at once.

#### Serena memories (Keep context clean)

- Perfer use serena mcp tools to understand the architecture and code hierarchy quickly.
- **ALWAYS** Call Serena's `activate_project` before reading memories.

#### When Memories Are Insufficient (On-Demand Querying and Reading)

- Check `READMD.md`

## IDA Pro MCP Tools Reference

### ida-pro-mcp.rename Usage

`rename` is a unified renaming tool that supports renaming functions, global variables, local variables, and stack variables.

#### Parameter Structure

```json
{
  "batch": {
    "func": [...],      // Function renaming
    "data": [...],      // Global/data variable renaming
    "local": [...],     // Local variable renaming
    "stack": [...]      // Stack variable renaming
  }
}
```

#### 1. Function renaming (`func`)

```json
{
  "batch": {
    "func": {
      "addr": "0x12345678",   // Function address (hex or decimal)
      "name": "NewFuncName"   // New function name
    }
  }
}
```

#### 2. Global / Data variable renaming (`data`)

```json
{
  "batch": {
    "data": {
      "old": "old_global_name",  // Current variable name
      "new": "new_global_name"   // New variable name
    }
  }
}
```

#### 3. Local variable renaming (`local`)

```json
{
  "batch": {
    "local": {
      "func_addr": "0x12345678",  // Function address containing the local variable
      "old": "v1",                 // Current variable name
      "new": "playerIndex"         // New variable name
    }
  }
}
```

#### 4. Stack variable renaming (`stack`)

```json
{
  "batch": {
    "stack": {
      "func_addr": "0x12345678",  // Function address containing the stack variable
      "old": "var_20",             // Current variable name
      "new": "bufferSize"          // New variable name
    }
  }
}
```

#### Batch Operation Example

Multiple rename operations can be performed simultaneously:

```json
{
  "batch": {
    "func": [
      {"addr": "0x1000", "name": "InitPlayer"},
      {"addr": "0x2000", "name": "UpdateHealth"}
    ],
    "local": [
      {"func_addr": "0x1000", "old": "a1", "new": "pPlayer"},
      {"func_addr": "0x1000", "old": "v5", "new": "healthValue"}
    ]
  }
}
```

### ida-pro-mcp.get_bytes Usage

`get_bytes` reads bytes from memory addresses in the binary.

#### Parameter Structure

```json
{
  "regions": {
    "addr": "0x12345678",  // Address to read from (hex or decimal)
    "size": 16             // Number of bytes to read
  }
}
```

#### Single Region Example

Read 16 bytes from a single address:

```json
{
  "regions": {
    "addr": "0x140001000",
    "size": 16
  }
}
```

#### Multiple Regions Example

Read bytes from multiple addresses simultaneously:

```json
{
  "regions": [
    {"addr": "0x140001000", "size": 16},
    {"addr": "0x140002000", "size": 32},
    {"addr": "0x140003000", "size": 8}
  ]
}
```

### ida-pro-mcp.int_convert Usage

`int_convert` converts numbers between different formats (hex, decimal, binary, ASCII).

#### Parameter Structure

```json
{
  "inputs": {
    "text": "0x41424344",  // Number string to convert (hex, decimal, or binary)
    "size": 4              // Byte size for conversion (omit for auto-detect)
  }
}
```

#### Single Conversion Example

Convert a hex number:

```json
{
  "inputs": {
    "text": "0x41424344"
  }
}
```

Returns decimal, hexadecimal, bytes (little-endian), binary, and ASCII representation (if printable).

#### Multiple Conversions Example

Convert multiple numbers simultaneously:

```json
{
  "inputs": [
    {"text": "0xFF"},
    {"text": "12345"},
    {"text": "0b11001100"}
  ]
}
```

#### With Explicit Size

Force a specific byte size for the conversion:

```json
{
  "inputs": {
    "text": "0x90",
    "size": 4
  }
}
```

### ida-pro-mcp.get_int Usage

`get_int` reads integer values from memory addresses. Use `ty` to specify the integer type including signedness and byte order.

#### Type Format

`ty` follows the pattern `{sign}{bits}{endian}`:

- **sign**: `i` (signed) or `u` (unsigned)
- **bits**: `8`, `16`, `32`, `64`
- **endian** (optional): `le` (little-endian, default) or `be` (big-endian)

Examples: `i8`, `u64`, `i16le`, `i16be`, `u32be`

#### Parameter Structure

```json
{
  "queries": {
    "addr": "0x12345678",  // Address to read from (hex or decimal)
    "ty": "u32"            // Integer type (i8/u64/i16le/i16be/etc)
  }
}
```

#### Single Read Example

Read an unsigned 32-bit integer:

```json
{
  "queries": {
    "addr": "0x140001000",
    "ty": "u32"
  }
}
```

#### Multiple Reads Example

Read multiple integers simultaneously:

```json
{
  "queries": [
    {"addr": "0x140001000", "ty": "u32"},
    {"addr": "0x140001004", "ty": "i16le"},
    {"addr": "0x140001008", "ty": "u64"}
  ]
}