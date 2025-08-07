#include "PtxTranslator.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cassert>

// --- Constructor & Main Entry ---
PtxTranslator::PtxTranslator(ptxParser::AstContext* tree, std::ofstream& out, const std::string& outFilename)
    : astContext(tree), outputFile(out), outputFilename(outFilename) {}

void PtxTranslator::translate() {
    if (astContext) {
        visitAst(astContext);
    }
    outputFile.flush();
    // Optional: Add validation call here
    // if (validateOutput()) { ... }
}

// --- Helper Functions ---
std::string PtxTranslator::getOperandText(ptxParser::OperandContext* operandCtx) {
    if (!operandCtx) return "";
    if (operandCtx->imm()) {
        return operandCtx->imm()->getText();
    } else if (operandCtx->var()) {
        return operandCtx->var()->getText();
    } else if (operandCtx->reg()) {
        return operandCtx->reg()->getText(); // Handles %r1, %tid.x
    } else if (operandCtx->vector()) {
        return operandCtx->getText(); // Simplified
    } else if (operandCtx->fetchAddress()) {
        return operandCtx->getText(); // Simplified
    }
    return operandCtx->getText();
}

std::string PtxTranslator::translateQualifierToSuffix(ptxParser::QualifierContext* qualifierCtx) {
    if (!qualifierCtx || !qualifierCtx->getStart()) return "";
    std::string text = qualifierCtx->getStart()->getText();
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    if (!text.empty() && text[0] == '.') text = text.substr(1);
    if (text == "u8" || text == "u16" || text == "u32" || text == "u64" ||
        text == "s8" || text == "s16" || text == "s32" || text == "s64" ||
        text == "f16" || text == "f32" || text == "f64" ||
        text == "b8" || text == "b16" || text == "b32" || text == "b64" ||
        text == "pred") {
        return text;
    }
    return "";
}

std::string PtxTranslator::translateQualifiersToSuffix(const std::vector<ptxParser::QualifierContext*>& qualifiers) {
    for (auto* q : qualifiers) {
        std::string suffix = translateQualifierToSuffix(q);
        if (!suffix.empty()) {
            return suffix;
        }
    }
    return "u32"; // Default
}

void PtxTranslator::recordSpecialRegisterUsage(const std::string& specialRegName) {
    // Assuming special regs contain a dot, like %tid.x
    if (specialRegName.find('.') != std::string::npos) {
         usedSpecialRegs.insert(specialRegName);
    }
}

std::string PtxTranslator::mapPtxOperandToCoasm(const std::string& ptxOperandText) {
    // Record special register usage
    recordSpecialRegisterUsage(ptxOperandText);
    // In this simplified model, operands map directly.
    // The key difference is skipping ld.param emission.
    return ptxOperandText;
}

uint32_t PtxTranslator::calculateKernelCtrl() {
    // Based on typical COASM CTRL_BIT definitions (example values)
    const std::unordered_map<std::string, uint32_t> CTRL_BITS = {
        {"%tid.x", 1U << 0}, {"%tid.y", 1U << 1}, {"%tid.z", 1U << 2},
        {"%ntid.x", 1U << 3}, {"%ntid.y", 1U << 4}, {"%ntid.z", 1U << 5},
        {"%ctaid.x", 1U << 6}, {"%ctaid.y", 1U << 7}, {"%ctaid.z", 1U << 8},
        {"%nctaid.x", 1U << 9}, {"%nctaid.y", 1U << 10}, {"%nctaid.z", 1U << 11},
        // Add others as needed based on coasm.md
    };

    uint32_t ctrl = 0;
    for (const auto& reg : usedSpecialRegs) {
        auto it = CTRL_BITS.find(reg);
        if (it != CTRL_BITS.end()) {
            ctrl |= it->second;
        }
    }
    return ctrl;
}

// --- Visitor Implementations ---
antlrcpp::Any PtxTranslator::visitAst(ptxParser::AstContext *context) {
    return visitChildren(context);
}

antlrcpp::Any PtxTranslator::visitKernels(ptxParser::KernelsContext *context) {
    return visitChildren(context);
}

antlrcpp::Any PtxTranslator::visitKernel(ptxParser::KernelContext *context) {
    if (!context->ID()) return nullptr;
    currentKernelName = context->ID()->getText();
    currentKernelMetadata = std::make_unique<KernelMetadata>();
    currentKernelMetadata->name = currentKernelName;
    ptxParamToCoasmReg.clear();
    ptxSharedToCoasmReg.clear();
    usedSpecialRegs.clear();

    emitKernelHeader();

    if (context->params()) {
        visitChildren(context->params());
    }
    if (context->compoundStatement()) {
        visitCompoundStatement(context->compoundStatement());
    }

    currentKernelMetadata->kernel_ctrl = calculateKernelCtrl();
    emitKernelFooter();
    currentKernelMetadata.reset();
    return nullptr;
}

antlrcpp::Any PtxTranslator::visitParam(ptxParser::ParamContext *context) {
    if (context->ID()) {
        std::string ptxParamName = context->ID()->getText();
        std::string coasmRegName = "%" + ptxParamName; // Symbolic register
        ptxParamToCoasmReg[ptxParamName] = coasmRegName;

        std::map<std::string, std::string> argInfo;
        argInfo[".address_space"] = "global"; // Simplified
        argInfo[".name"] = ptxParamName;
        argInfo[".offset"] = "0"; // Needs proper calculation
        argInfo[".size"] = "8";   // Needs proper calculation
        argInfo[".value_kind"] = "global_buffer";
        currentKernelMetadata->args.push_back(argInfo);
    }
    return nullptr;
}

antlrcpp::Any PtxTranslator::visitStatements(ptxParser::StatementsContext *context) {
    return visitChildren(context);
}

antlrcpp::Any PtxTranslator::visitStatement(ptxParser::StatementContext *context) {
    return visitChildren(context);
}

antlrcpp::Any PtxTranslator::visitCompoundStatement(ptxParser::CompoundStatementContext *context) {
    if (context->statements()) {
        return visitStatements(context->statements());
    }
    return nullptr;
}

// --- Instruction Translation ---

antlrcpp::Any PtxTranslator::visitLdStatement(ptxParser::LdStatementContext *context) {
    // ldStatement : LD qualifier* operandTwo SEMI ;
    bool isParamLoad = false;
    for (auto* qual : context->qualifier()) {
        if (qual && qual->getStart() && qual->getStart()->getType() == ptxLexer::PARAM) {
            isParamLoad = true;
            break;
        }
    }

    if (isParamLoad) {
        // Skip ld.param instruction emission
        // The address is provided via metadata/symbolic register
        return nullptr;
    } else {
        // Translate ld.global/shared/local/const to coasm ld
        if (context->operandTwo() && context->operandTwo()->operand().size() >= 2) {
            const std::vector<ptxParser::QualifierContext*>& qualifiers = context->qualifier();
            std::string typeSuffix = translateQualifiersToSuffix(qualifiers);
            // TODO: Refine type/space handling for coasm if needed
            std::string dst = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(0)));
            std::string addr = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(1)));
            // Simplified: assuming coasm ld combines space/type into suffix
            outputFile << "\tld." << typeSuffix << "\t" << dst << ", " << addr << "\n";
        }
        return nullptr;
    }
}

antlrcpp::Any PtxTranslator::visitStStatement(ptxParser::StStatementContext *context) {
    // stStatement : ST qualifier* operandTwo SEMI ;
    if (context->operandTwo() && context->operandTwo()->operand().size() >= 2) {
        const std::vector<ptxParser::QualifierContext*>& qualifiers = context->qualifier();
        std::string typeSuffix = translateQualifiersToSuffix(qualifiers);
        std::string addr = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(0)));
        std::string src = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(1)));
        outputFile << "\tst." << typeSuffix << "\t" << addr << ", " << src << "\n";
    }
    return nullptr;
}

// --- Macro for Binary Operations (operandThree) ---
#define TRANSLATE_BINARY_OP(OP_NAME, PTX_CONTEXT_TYPE) \
antlrcpp::Any PtxTranslator::visit##PTX_CONTEXT_TYPE(ptxParser::PTX_CONTEXT_TYPE##Context *context) { \
    if (context->operandThree() && context->operandThree()->operand().size() >= 3) { \
        const std::vector<ptxParser::QualifierContext*>& qualifiers = context->qualifier(); \
        std::string typeSuffix = translateQualifiersToSuffix(qualifiers); \
        std::string dst = mapPtxOperandToCoasm(getOperandText(context->operandThree()->operand(0))); \
        std::string src1 = mapPtxOperandToCoasm(getOperandText(context->operandThree()->operand(1))); \
        std::string src2 = mapPtxOperandToCoasm(getOperandText(context->operandThree()->operand(2))); \
        outputFile << "\t" << OP_NAME << "." << typeSuffix << "\t" << dst << ", " << src1 << ", " << src2 << "\n"; \
    } \
    return nullptr; \
}
TRANSLATE_BINARY_OP("add", AddStatement)
TRANSLATE_BINARY_OP("mul", MulStatement)
TRANSLATE_BINARY_OP("div", DivStatement)
TRANSLATE_BINARY_OP("sub", SubStatement)
TRANSLATE_BINARY_OP("shl", ShlStatement)
TRANSLATE_BINARY_OP("shr", ShrStatement)
TRANSLATE_BINARY_OP("max", MaxStatement)
TRANSLATE_BINARY_OP("min", MinStatement)
TRANSLATE_BINARY_OP("and", AndStatement)
TRANSLATE_BINARY_OP("or", OrStatement)
TRANSLATE_BINARY_OP("xor", XorStatement)

// --- visitMadStatement (operandFour) ---
antlrcpp::Any PtxTranslator::visitMadStatement(ptxParser::MadStatementContext *context) {
    // madStatement : MAD qualifier* operandFour SEMI ;
    if (context->operandFour() && context->operandFour()->operand().size() >= 4) {
        const std::vector<ptxParser::QualifierContext*>& qualifiers = context->qualifier();
        std::string typeSuffix = translateQualifiersToSuffix(qualifiers);
        std::string dst = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(0)));
        std::string src1 = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(1)));
        std::string src2 = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(2)));
        std::string src3 = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(3)));
        outputFile << "\tmad." << typeSuffix << "\t" << dst << ", " << src1 << ", " << src2 << ", " << src3 << "\n";
    }
    return nullptr;
}

// --- visitMovStatement (Single Qualifier) ---
antlrcpp::Any PtxTranslator::visitMovStatement(ptxParser::MovStatementContext *context) {
    // movStatement : MOV qualifier operandTwo SEMI ;
    // Based on output.md and standard ANTLR for `rule : TOKEN qualifier ...`, qualifier() returns single ptr
    if (context->operandTwo() && context->operandTwo()->operand().size() >= 2) {
        ptxParser::QualifierContext* singleQualifier = context->qualifier(); // Single pointer
        std::vector<ptxParser::QualifierContext*> tempQualifierVec;
        if (singleQualifier) {
            tempQualifierVec.push_back(singleQualifier);
        }
        std::string typeSuffix = translateQualifiersToSuffix(tempQualifierVec);
        std::string dst = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(0)));
        std::string src = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(1)));
        outputFile << "\tmov." << typeSuffix << "\t" << dst << ", " << src << "\n";
    }
    return nullptr;
}

// --- Macro for Unary Operations (operandTwo, qualifier*) ---
#define TRANSLATE_UNARY_OP(OP_NAME, PTX_CONTEXT_TYPE) \
antlrcpp::Any PtxTranslator::visit##PTX_CONTEXT_TYPE(ptxParser::PTX_CONTEXT_TYPE##Context *context) { \
    if (context->operandTwo() && context->operandTwo()->operand().size() >= 2) { \
        const std::vector<ptxParser::QualifierContext*>& qualifiers = context->qualifier(); \
        std::string typeSuffix = translateQualifiersToSuffix(qualifiers); \
        std::string dst = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(0))); \
        std::string src = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(1))); \
        outputFile << "\t" << OP_NAME << "." << typeSuffix << "\t" << dst << ", " << src << "\n"; \
    } \
    return nullptr; \
}
TRANSLATE_UNARY_OP("neg", NegStatement)
TRANSLATE_UNARY_OP("not", NotStatement)
TRANSLATE_UNARY_OP("abs", AbsStatement)
TRANSLATE_UNARY_OP("rcp", RcpStatement)
TRANSLATE_UNARY_OP("sqrt", SqrtStatement)
TRANSLATE_UNARY_OP("rsqrt", RsqrtStatement)

// --- visitCvtStatement (Special Type Handling) ---
antlrcpp::Any PtxTranslator::visitCvtStatement(ptxParser::CvtStatementContext *context) {
    // cvt.u32.f32 dst, src
    std::string toType = "u32";
    std::string fromType = "u32";
    int typeCount = 0;
    for (auto* q : context->qualifier()) {
        std::string suffix = translateQualifierToSuffix(q);
        if (!suffix.empty()) {
            if (typeCount == 0) toType = suffix;
            else if (typeCount == 1) { fromType = suffix; break; }
            typeCount++;
        }
    }

    if (context->operandTwo() && context->operandTwo()->operand().size() >= 2) {
        std::string dst = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(0)));
        std::string src = mapPtxOperandToCoasm(getOperandText(context->operandTwo()->operand(1)));
        outputFile << "\tcvt." << toType << "." << fromType << "\t" << dst << ", " << src << "\n";
    }
    return nullptr;
}

// --- visitSetpStatement ---
antlrcpp::Any PtxTranslator::visitSetpStatement(ptxParser::SetpStatementContext *context) {
    // setp.eq.u32 %p1, %r1, %r2;
    std::string cmpOp = "eq";
    for (auto* q : context->qualifier()) {
         if (q && q->getStart()) {
             std::string text = q->getStart()->getText();
             std::transform(text.begin(), text.end(), text.begin(), ::tolower);
             if (!text.empty() && text[0] == '.') text = text.substr(1);
             if (text == "eq" || text == "ne" || text == "lt" || text == "le" ||
                 text == "gt" || text == "ge" || text == "lo" || text == "ls" ||
                 text == "hi" || text == "hs" || text == "equ" || text == "neu" ||
                 text == "ltu" || text == "leu" || text == "gtu" || text == "geu") {
                 cmpOp = text;
                 break;
             }
         }
    }

    if (context->operandThree() && context->operandThree()->operand().size() >= 3) {
        std::string predDst = mapPtxOperandToCoasm(getOperandText(context->operandThree()->operand(0)));
        std::string src1 = mapPtxOperandToCoasm(getOperandText(context->operandThree()->operand(1)));
        std::string src2 = mapPtxOperandToCoasm(getOperandText(context->operandThree()->operand(2)));
        outputFile << "\tsetp." << cmpOp << ".u32\t" << predDst << ", " << src1 << ", " << src2 << "\n";
    }
    return nullptr;
}

// --- visitBraStatement ---
antlrcpp::Any PtxTranslator::visitBraStatement(ptxParser::BraStatementContext *context) {
    // bra BB0_3; or @%p1 bra BB0_3;
    if (context->ID()) {
         std::string targetLabel = context->ID()->getText();
         // TODO: Handle predicate from context if present (e.g., check qualifiers for pred)
         outputFile << "\ts_branch " << targetLabel << "\n"; // Placeholder coasm branch
    }
    return nullptr;
}

// --- visitRetStatement ---
antlrcpp::Any PtxTranslator::visitRetStatement(ptxParser::RetStatementContext *context) {
    outputFile << "\tt_exit\n";
    return nullptr;
}

// --- visitSelpStatement (operandFour) ---
antlrcpp::Any PtxTranslator::visitSelpStatement(ptxParser::SelpStatementContext *context) {
    // selp.a.type d, a, b, c;
    if (context->operandFour() && context->operandFour()->operand().size() >= 4) {
        const std::vector<ptxParser::QualifierContext*>& qualifiers = context->qualifier();
        std::string typeSuffix = translateQualifiersToSuffix(qualifiers);
        std::string dst = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(0)));
        std::string src1 = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(1)));
        std::string src2 = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(2)));
        std::string pred = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(3)));
        outputFile << "\tselp." << typeSuffix << "\t" << dst << ", " << src1 << ", " << src2 << ", " << pred << "\n";
    }
    return nullptr;
}

// --- visitFmaStatement (operandFour) ---
antlrcpp::Any PtxTranslator::visitFmaStatement(ptxParser::FmaStatementContext *context) {
    // fma.rn.f32 d, a, b, c;
    if (context->operandFour() && context->operandFour()->operand().size() >= 4) {
        const std::vector<ptxParser::QualifierContext*>& qualifiers = context->qualifier();
        std::string typeSuffix = translateQualifiersToSuffix(qualifiers);
        std::string dst = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(0)));
        std::string src1 = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(1)));
        std::string src2 = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(2)));
        std::string src3 = mapPtxOperandToCoasm(getOperandText(context->operandFour()->operand(3)));
        outputFile << "\tfma." << typeSuffix << "\t" << dst << ", " << src1 << ", " << src2 << ", " << src3 << "\n";
    }
    return nullptr;
}

// --- Output Generation ---
void PtxTranslator::emitKernelHeader() {
    outputFile << "\t.text\n";
    outputFile << "\t.global " << currentKernelName << "\n";
    outputFile << "\t.type " << currentKernelName << ",@function\n";
    outputFile << currentKernelName << ":\n";
    outputFile << "bb_00:\n"; // Placeholder entry label
}

void PtxTranslator::emitKernelFooter() {
    if (!currentKernelMetadata) return;

    outputFile << "-\n";
    outputFile << "opu.kernels:\n";
    outputFile << " - .name: " << currentKernelMetadata->name << "\n";
    outputFile << "   .args:\n";
    for (const auto& arg : currentKernelMetadata->args) {
        outputFile << "     -";
        for (const auto& pair : arg) {
            outputFile << " " << pair.first << ": " << pair.second;
        }
        outputFile << "\n";
    }
    if (!currentKernelMetadata->shared.empty()) {
         outputFile << "   .shared:\n";
         for (const auto& sh : currentKernelMetadata->shared) {
             outputFile << "     -";
             for (const auto& pair : sh) {
                 outputFile << " " << pair.first << ": " << pair.second;
             }
             outputFile << "\n";
         }
    }
    outputFile << "   .shared_memsize: " << currentKernelMetadata->shared_memsize << "\n";
    outputFile << "   .private_memsize: " << currentKernelMetadata->private_memsize << "\n";
    outputFile << "   .cmem_size: " << currentKernelMetadata->cmem_size << "\n";
    outputFile << "   .bar_used: " << currentKernelMetadata->bar_used << "\n";
    outputFile << "   .local_framesize: " << currentKernelMetadata->local_framesize << "\n";
    outputFile << "   .kernel_ctrl: " << currentKernelMetadata->kernel_ctrl << "\n";
    outputFile << "   .kernel_mode: " << currentKernelMetadata->kernel_mode << "\n";

    outputFile << "opu.version:\n";
    outputFile << " - 2\n"; // Placeholder from coasm.md
    outputFile << " - 0\n"; // Placeholder
    outputFile << "...\n";
}

// --- Validation (Optional) ---
// bool PtxTranslator::validateOutput() { ... } // Implementation as before
