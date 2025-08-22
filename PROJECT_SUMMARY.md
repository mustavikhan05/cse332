# CSE332 MIPS CPU Project - Implementation Summary

## Project Overview
This project implements a complete 32-bit MIPS CPU system as part of CSE332 Computer Organization and Architecture course. The implementation follows the three-phase approach outlined in the project requirements:

1. **ISA Study** - Understanding MIPS instruction formats and requirements
2. **Assembler** - Translating assembly language to machine code  
3. **CPU Hardware** - Verilog implementation of datapath and control unit

## Problem Statement & Solution

### Original Issue
The provided assembler from the GitHub repository could not successfully assemble the test file `insttest2.s`, which is specifically recommended in the project outline for CPU testing.

### Root Cause Analysis
After thorough investigation, we identified three critical syntax incompatibilities:

1. **JAL instruction with 2 operands**: `jal $31,compute_stats` (line 6)
   - Original assembler only supported: `jal label`
   - Test file used: `jal register,label`

2. **LW instruction with direct label addressing**: `lw $5,size` and `lw $6,arr` (lines 10-11)
   - Original assembler expected: `lw $reg,offset($base)`
   - Test file used: `lw $reg,label`

3. **ADDI instruction with label as immediate**: `addi $11,$11,arr` (line 20)
   - Original assembler only accepted numeric immediates
   - Test file used data label as immediate value

## Implementation Details

### Assembler Modifications

#### 1. JAL Instruction Enhancement
**File Modified**: `finalassembler.cpp`, `mips.h`

**Changes Made**:
- Created new `type11` function to handle JAL with register+label syntax
- Updated instruction mapping in `mips.h` to route JAL to type 11
- Maintained backward compatibility with single-operand JAL
- Added fallback logic for standard JAL format

**Technical Implementation**:
```cpp
// New type11 function handles: jal $reg,label
void type11(string opname, string operand, int operand_count, ...)
{
    if(operand_count == 2) {
        // Handle register + label format
        // Generate JAL instruction with 26-bit address field
    }
    else if(operand_count == 1) {
        // Fallback to standard JAL with just label
    }
}
```

#### 2. LW Instruction Enhancement  
**File Modified**: `finalassembler.cpp`

**Changes Made**:
- Modified `type5` function to detect label vs (offset)register format
- Added logic to resolve data label addresses directly
- Uses $zero register as base for absolute addressing

**Technical Implementation**:
```cpp
// Detection logic for label format
bool is_label_format = (str2.find('(') == string::npos);

if(is_label_format) {
    // Handle "lw $reg, label" format
    // Use $zero as base register
    // Convert label address to 16-bit offset
}
```

#### 3. ADDI Instruction Enhancement
**File Modified**: `finalassembler.cpp`

**Changes Made**:
- Updated `type3` function to accept labels as immediate values
- Added label validation and address resolution
- Converts data label addresses to 16-bit immediate values

**Technical Implementation**:
```cpp
// Label detection and resolution
if(datalabels.count(imm) > 0) {
    is_label = true;
    imm_int = DATALABEL[imm]; // Get label address
}
```

### Compilation Fix
**Issue**: Forward declaration error when compiling
**Solution**: Inlined `type10` logic within `type11` to eliminate dependency

## File Structure and Roles

### Core Assembler Files

#### `finalassembler.cpp` (Main Implementation)
**Role**: Primary assembler logic
- **Two-pass assembly**: First pass for label resolution, second pass for code generation
- **Instruction processing**: Handles all MIPS instruction types (R, I, J formats)
- **Symbol table management**: Tracks labels, data labels, and addresses
- **Machine code generation**: Converts assembly to 32-bit binary instructions
- **Error handling**: Syntax validation and error reporting

**Key Functions**:
- `string_type()`: Classifies input lines (instruction, label, directive)
- `type1-type11()`: Handles different instruction syntaxes
- `datalabel_process()`: Processes data section labels and directives
- `instruction_process()`: Routes instructions to appropriate type handlers

#### `mips.h` (ISA Definitions)
**Role**: MIPS ISA specification and mappings
- **Instruction set**: Defines all supported MIPS instructions
- **Register mappings**: Maps register names to 5-bit binary codes
- **Opcode mappings**: Maps instruction names to binary opcodes
- **Type classifications**: Categorizes instructions by operand syntax

**Key Data Structures**:
- `instruction`: Set of all valid instruction names
- `instr_type`: Maps instructions to syntax type handlers
- `reg_op`: Maps register names to binary codes
- `type2_op`, `type3_op1`, etc.: Opcode mappings for different instruction formats

#### `data_symbol_table.h` (Data Section Handler)
**Role**: Manages data section symbols and code generation
- **Symbol tracking**: Handles .word, .space, .ascii, .asciiz directives
- **Address calculation**: Computes data memory layout
- **Code generation**: Produces binary data for memory initialization

### CPU Hardware Files

#### `datapath.v` (CPU Datapath)
**Role**: Implements the MIPS datapath - the "body" of the CPU
- **PC logic**: Program counter management and next PC calculation
- **Register file**: 32 general-purpose registers with read/write ports
- **ALU**: 32-bit arithmetic and logic unit with multiple operations
- **Memory interface**: Connections to instruction and data memory
- **Multiplexers**: Data path selection for different instruction types
- **Jump/Branch logic**: Target address calculation and PC updates

**Key Components**:
- PC register with reset capability
- 32-bit adders for PC+4 and branch target calculation
- Register file with dual read ports and one write port
- ALU with configurable operations
- Multiplexers for register destination, ALU source selection
- Sign extension unit for immediate values
- Shift left unit for address calculation

**JAL/JR Support**:
- JAL: Saves PC+4 to $ra (register 31) and jumps to target
- JR: Jumps to address in specified register
- Dedicated control signals and datapath routing

#### `control.v` (CPU Control Unit)
**Role**: Implements the control unit - the "brain" of the CPU
- **Instruction decoding**: Interprets opcode and function fields
- **Control signal generation**: Produces signals to control datapath
- **ALU control**: Determines ALU operation based on instruction
- **Memory control**: Manages memory read/write operations
- **Branch control**: Handles conditional branch decisions

**Key Signals**:
- `RegWrite`: Enable register file write
- `RegDst`: Select register destination (rd vs rt)
- `ALUSrc`: Select ALU second operand (register vs immediate)
- `MemWrite`: Enable data memory write
- `MemtoReg`: Select write data source (ALU vs memory)
- `Jump`: Enable jump operation
- `JAL`/`JR`: Special control for function calls
- `ALUControl`: 4-bit signal specifying ALU operation

**Instruction Support**:
- R-type: ADD, SUB, AND, OR, XOR, SLT, shifts, JR
- I-type: ADDI, ANDI, ORI, XORI, SLTI, LW, SW, branches
- J-type: J, JAL

### Test and Documentation Files

#### `insttest2.s` (Primary Test File)
**Role**: Comprehensive test program for CPU functionality
- **Function calls**: Tests JAL/JR instruction implementation
- **Data processing**: Array operations (min, max, sum calculations)
- **Memory operations**: Load/store with various addressing modes
- **Control flow**: Loops, branches, conditional execution

**Program Structure**:
```assembly
.data
arr: .word 3,4,6,10,2,8    # Test array
size: .word 6              # Array size

.text
main:
    jal $31,compute_stats  # Function call with JAL
    j end

compute_stats:
    # Array processing logic
    # Min/max finding algorithm
    # Uses LW with labels, ADDI with labels
    jr $31                 # Return with JR
```

#### `pervious_assembly_original.txt` (Working Example)
**Role**: Reference implementation that assembles successfully
- **Basic instructions**: Simple arithmetic and logic operations
- **Standard syntax**: Uses conventional MIPS assembly format
- **Compatibility check**: Verifies assembler works with standard code

#### `CSE332 Project Outline_.pdf` (Project Requirements)
**Role**: Official project specification
- **Requirements**: Detailed project phases and deliverables
- **Test methodology**: How the CPU will be evaluated
- **ISA specifications**: Required instruction set features
- **Documentation guidelines**: Expected submission format

## Technical Achievements

### 1. Backward Compatibility
- All existing assembly programs continue to work
- No breaking changes to standard MIPS syntax
- Maintains compatibility with original instruction formats

### 2. Enhanced Instruction Support
- **JAL enhancement**: Supports both `jal label` and `jal $reg,label`
- **Direct label addressing**: Enables `lw $reg,label` syntax
- **Label arithmetic**: Allows labels as immediate values in ADDI

### 3. Robust Error Handling
- **Syntax validation**: Comprehensive error checking and reporting
- **Label resolution**: Proper handling of forward and backward references
- **Type checking**: Validates register names, immediate ranges, label existence

### 4. Code Generation Quality
- **Correct encoding**: Generates proper 32-bit MIPS machine code
- **Address calculation**: Handles PC-relative and absolute addressing
- **Binary output**: Produces both addressed and non-addressed binary files

## Testing and Validation

### Assembly Testing
1. **Compilation**: Successfully compiles with g++ on Linux/WSL
2. **Basic functionality**: Assembles simple programs correctly
3. **Enhanced features**: Handles insttest2.s syntax extensions
4. **Error cases**: Properly reports syntax and semantic errors

### Expected CPU Integration
1. **Machine code loading**: Binary files ready for Verilog memory initialization
2. **Instruction execution**: Support for all required MIPS instructions
3. **Function calls**: JAL/JR implementation enables procedure calls
4. **Data access**: Label-based addressing simplifies data manipulation

## Project Significance

This implementation enables the complete CSE332 CPU project by:

1. **Bridging assembly and hardware**: Translates high-level assembly to machine code
2. **Supporting modern syntax**: Handles practical assembly programming patterns
3. **Enabling testing**: Provides working test programs for CPU validation
4. **Educational value**: Demonstrates real assembler implementation challenges

The modifications ensure that students can focus on CPU design and implementation rather than struggling with assembler limitations, while maintaining the educational integrity of the MIPS ISA learning experience.

## Repository Structure
```
UpgradedMIPS32Assembler/
├── finalassembler.cpp      # Main assembler implementation
├── mips.h                  # MIPS ISA definitions
├── data_symbol_table.h     # Data section handling
├── control.v               # CPU control unit (Verilog)
├── datapath.v             # CPU datapath (Verilog)
├── insttest2.s            # Primary test program
├── pervious_assembly_original.txt  # Reference test
├── CSE332 Project Outline_.pdf     # Project requirements
└── PROJECT_SUMMARY.md     # This documentation
```

---
*This implementation was developed as part of CSE332 Computer Organization and Architecture course at North South University, enabling students to complete the full CPU design project with a working assembler and comprehensive test suite.*