# Advanced CS2 Signature Research System

## Overview

This is a comprehensive signature research and dumping system that goes far beyond basic pattern matching. It uses multiple reverse engineering techniques to find and verify CS2 function signatures with high accuracy.

## Research Methods

### 1. **Known Pattern Analysis** 
- Uses patterns from UnknownCheats forum research (Raphilaa, koz11, March 2026)
- Multiple pattern alternatives for different CS2 builds
- Confidence scoring based on pattern reliability
- **Sources**: UC Forum, reverse engineering research

### 2. **String Reference Analysis**
- Finds functions by analyzing nearby string literals
- Looks for debug strings, error messages, function names
- Cross-references string usage with function locations
- **Example**: "loadout", "inventory", "equip_item" → EquipItemInLoadout

### 3. **RTTI & Vtable Analysis**
- Uses Run-Time Type Information to find class methods
- Scans vtables for method pointers
- Identifies functions by class membership
- **Example**: CCSInventoryManager vtable → inventory methods

### 4. **Cross-Reference Analysis**
- Finds functions by analyzing call patterns
- Looks for specific instruction sequences
- Resolves call targets and references
- **Example**: E8 call followed by inventory access → EquipItemInLoadout

### 5. **Code Flow Analysis**
- Analyzes function call patterns and context
- Identifies functions by their usage patterns
- Looks for specific instruction combinations
- **Example**: Call + test + jz pattern → SetBodyGroup

### 6. **Function Prologue Analysis**
- Identifies functions by unique prologue patterns
- Analyzes register usage and stack setup
- Matches against known function signatures
- **Example**: Specific push/mov sequence → CreateNewPaintKit

### 7. **Import Table Analysis**
- Finds functions that call specific Windows/Steam APIs
- Analyzes import usage patterns
- Identifies functions by external dependencies
- **Example**: SteamAPI calls → GetInventoryManager

## Verification System

### Multi-Layer Verification
1. **Prologue Matching**: Verify function starts match expected patterns
2. **Cross-Reference Validation**: Ensure functions are called from correct contexts
3. **Function Structure**: Analyze size, instruction patterns, flow
4. **Runtime Safety**: Check if addresses are executable and safe
5. **Known Characteristics**: Verify function-specific behaviors

### Confidence Scoring
- **90-100%**: RTTI/Vtable analysis (most reliable)
- **80-90%**: Known patterns from research
- **70-80%**: String references + verification
- **60-70%**: Cross-reference analysis
- **50-60%**: Basic pattern matching

## Target Functions

### Core Skinchanger Functions
- **EquipItemInLoadout**: Forces item equip in loadout slot
- **GetItemInLoadout**: Gets item from specific loadout slot
- **SetBodyGroup**: Required for glove mesh updates
- **SetModel**: Changes entity model (knives)
- **CreateNewPaintKit**: Creates paint kit for skins
- **RegenerateWeaponSkin**: Forces skin regeneration
- **UpdateCompositeMaterial**: Updates weapon materials
- **GetInventoryManager**: Gets Steam inventory manager

### Material System Functions
- **CreateMaterial**: Creates new materials for chams
- **FindParameter**: Finds material parameters
- **UpdateParameter**: Updates material parameters
- **PrepareSceneMaterial**: Prepares materials for rendering

## Usage

### 1. Run Advanced Research
```bash
cd dumper
cargo build --release
.\target\release\cs2-dumper.exe --research
```

### 2. Check Research Results
```bash
# View detailed research results
cat output/research_signatures.json

# View verification report
cat output/verification_report.json
```

### 3. Integration
The research system generates multiple signature formats:
- **C++ Headers**: For direct integration
- **JSON Data**: For automated processing
- **Verification Reports**: For confidence analysis

## Research Sources

### Primary Sources
- **UnknownCheats Forum**: Raphilaa, koz11 research (March 2026)
- **CS2 Reverse Engineering**: Community research and analysis
- **Pattern Analysis**: Multi-build pattern comparison
- **RTTI Research**: Class structure analysis

### Verification Methods
- **Static Analysis**: Code structure and pattern analysis
- **Dynamic Analysis**: Runtime behavior verification
- **Cross-Validation**: Multiple method confirmation
- **Community Validation**: UC forum verification

## Advanced Features

### 1. **Multi-Build Support**
- Patterns for different CS2 builds
- Automatic fallback to alternative patterns
- Build-specific confidence adjustments

### 2. **Automated Verification**
- Function prologue validation
- Cross-reference checking
- Runtime safety verification
- Confidence scoring

### 3. **Research Integration**
- Incorporates latest UC forum research
- Updates patterns based on community findings
- Validates against known working signatures

### 4. **Comprehensive Reporting**
- Detailed verification reports
- Confidence analysis
- Method comparison
- Success rate tracking

## Benefits Over Manual IDA Analysis

### ✅ **Automated Research**
- No manual IDA Pro work required
- Automated pattern discovery and verification
- Multiple research methods in parallel

### ✅ **High Accuracy**
- Multi-method verification system
- Confidence scoring and validation
- Research-based pattern selection

### ✅ **Always Updated**
- Incorporates latest community research
- Automatic pattern updates
- Build-specific adaptations

### ✅ **Comprehensive Coverage**
- Multiple signature discovery methods
- Fallback patterns for reliability
- Cross-validation for accuracy

## Future Enhancements

### Planned Features
1. **Machine Learning**: Pattern recognition and classification
2. **Community Integration**: Automatic UC forum updates
3. **Build Tracking**: Automatic CS2 update detection
4. **Advanced RTTI**: Enhanced class analysis
5. **Fuzzy Matching**: Approximate pattern matching

### Research Areas
1. **New Function Discovery**: Automated function identification
2. **Pattern Evolution**: Tracking pattern changes across builds
3. **Verification Enhancement**: Improved validation methods
4. **Performance Optimization**: Faster scanning algorithms

This advanced signature research system provides a **comprehensive, automated solution** for finding and verifying CS2 function signatures, eliminating the need for manual IDA Pro analysis while providing higher accuracy and reliability than basic pattern matching.