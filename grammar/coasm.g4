grammar coasm;

prog: line* EOF ;

/**
 * 一行内容 (基于 coasm.g4.md, 扩展以处理更多行类型)
 * 允许指令行、标签行、指令块行、独立注释行、空行
 */
line
    : directive line_comment? NEWLINE?             # DirectiveLine
    | label line_comment? NEWLINE?                 # LabelLine
    | label? instruction line_comment? NEWLINE?    # InstructionLine // 指令通常以分号结尾
    | block line_comment? NEWLINE?                 # BlockLine // 如果块被视为独立行元素
    | line_comment NEWLINE?                        # CommentLine // 独立的注释行
    | NEWLINE                                      # EmptyLine   // 空行
    ;

// --- Directives ---
// Directives are commands to the assembler/runtime, not CPU instructions.
// Based on coasm.g4.md structure
directive
    : asm_directive
    | mem_directive
    | section_directive
    // Add specific rules for metadata directives if needed for complex parsing
    // Simple directives can be handled by lexer tokens in the 'instruction' rule too.
    ;

// Placeholder definitions for directive categories from coasm.g4.md
// These can be expanded based on specific needs.
asm_directive: START_KERNEL IDENTIFIER | kernel_option_directive ; // Example
mem_directive: ALIGN_DIRECTIVE INTEGER_LITERAL | SIZE_DIRECTIVE INTEGER_LITERAL ; // Example
section_directive: TEXT_DIRECTIVE | GLOBAL_DIRECTIVE IDENTIFIER | TYPE_DIRECTIVE IDENTIFIER ',' TYPE_ATTR ;

// Example of a specific kernel option directive (can be expanded)
kernel_option_directive
    : SHARED_MEMSIZE_DIRECTIVE INTEGER_LITERAL
    | KERNEL_CTRL_DIRECTIVE INTEGER_LITERAL
    // ... other options
    ;

// --- Labels (from coasm.g4.md) ---
label: IDENTIFIER ':' ;

// --- Instructions and Blocks ---
instruction: OPCODE (instruction_type_suffix)? (operands)? SEMICOLON? ; // Instructions may or may not end with ;
instruction_type_suffix: DOT IDENTIFIER ; // e.g., .u32, .f32
operands: operand (COMMA operand)* ;
block: LEFT_BRACE (line)* RIGHT_BRACE ; // Nested lines

// --- Operands ---
operand
    : REGISTER              # RegisterOperand     // %r1, %rd2, %p3, %_Z10vectorSwapPiS__param_0
    | SPECIAL_REGISTER      # SpecialRegisterOperand // %tid.x, %nctaid.x
    | immediate             # ImmediateOperand    // 123, 0x10, 3.14
    | memory_operand        # MemoryOperand       // [reg], [reg + offset]
    | IDENTIFIER            # IdentifierOperand   // Label names, symbol names used as operands
    ;

immediate: INTEGER_LITERAL | HEX_LITERAL | FLOAT_LITERAL ;
memory_operand: LEFT_BRACKET memory_address RIGHT_BRACKET ;
memory_address: operand (PLUS operand)* ; // Simplified, e.g., reg, reg+reg, reg+imm

// --- Lexer Rules (Tokens) ---

// --- Keywords based on coasm.md examples ---
START_KERNEL: '.start_kernel' ;
TEXT_DIRECTIVE: '.text' ;
GLOBAL_DIRECTIVE: '.global' ;
TYPE_DIRECTIVE: '.type' ;
ALIGN_DIRECTIVE: '.align' ;
SIZE_DIRECTIVE: '.size' ;
SHARED_MEMSIZE_DIRECTIVE: '.shared_memsize' ;
KERNEL_CTRL_DIRECTIVE: '.kernel_ctrl' ;
// Add more directive keywords as needed from coasm.md examples
// opu.kernels:, opu.version: are multi-token sequences, handled by parser rules or lexer modes if complex

// --- Instruction Opcodes ---
OPCODE: IDENTIFIER_PART ; // Generic opcode identifier

// --- Unified and Symbolic Registers ---
// This implements the specific naming scheme discussed:
// %<any_valid_identifier> where identifier can be r1, rd2, p3, or _Z10...
REGISTER: PERCENT IDENTIFIER_PART ;

// --- Special Registers (PTX-style built-ins) ---
SPECIAL_REGISTER: PERCENT (IDENTIFIER_PART '.')+ IDENTIFIER_PART ; // %tid.x, %nctaid.x

// --- Identifiers and Literals ---
fragment IDENTIFIER_PART: [a-zA-Z_$][a-zA-Z_$0-9]* ; // Allow $ for assembler temporaries
IDENTIFIER: IDENTIFIER_PART ;

INTEGER_LITERAL: SIGN? [0-9]+ ;
HEX_LITERAL: '0' [xX] [0-9a-fA-F]+ ;
FLOAT_LITERAL: SIGN? [0-9]* '.' [0-9]+ ([eE] SIGN? [0-9]+)? ;
SIGN: [+-] ;

// --- Symbols and Punctuation ---
PERCENT: '%' ;
DOT: '.' ;
COLON: ':' ;
COMMA: ',' ;
SEMICOLON: ';' ; // Optional semicolon at end of instruction line
LEFT_BRACKET: '[' ;
RIGHT_BRACKET: ']' ;
LEFT_BRACE: '{' ;
RIGHT_BRACE: '}' ;
PLUS: '+' ;
AT_SYMBOL: '@' ; // Used in .type directive

// --- Type Attributes ---
TYPE_ATTR: AT_SYMBOL FUNCTION_KEYWORD ;
FUNCTION_KEYWORD: 'function' ;

// --- Comments and Whitespace ---
line_comment: LINE_COMMENT ;
LINE_COMMENT: '//' ~[\r\n]* -> channel(HIDDEN) ;
BLOCK_COMMENT: '/*' .*? '*/' -> channel(HIDDEN) ;
NEWLINE: '\r'? '\n' -> skip ; // Skip newlines, handled in parser line rules
WS: [ \t]+ -> skip ;

// --- Error Handling ---
// ANTLR's default error recovery is usually sufficient.
// ERROR_TOKEN: . -> channel(HIDDEN); // Example, often not necessary
