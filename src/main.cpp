#include <iostream>
#include <fstream>
#include <string>
#include "antlr4-runtime.h"
#include "ptxLexer.h"
#include "ptxParser.h"
#include "PtxTranslator.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.ptx> <output.asm>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    std::ifstream stream(input_file);
    if (!stream.good()) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }

    antlr4::ANTLRInputStream input(stream);
    ptxLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ptxParser parser(&tokens);

    // 解析输入，生成解析树
    ptxParser::AstContext* tree = parser.ast(); // 假设顶层规则是 'ast'

    if (parser.getNumberOfSyntaxErrors() > 0) {
        std::cerr << "Error: Syntax errors encountered during PTX parsing." << std::endl;
        return 1;
    }

    // 打开输出文件
    std::ofstream outFile(output_file);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file " << output_file << std::endl;
        stream.close();
        return 1;
    }

    // 创建翻译器并执行翻译 (传递输出文件名)
    PtxTranslator translator(tree, outFile, output_file); // <--- 更新构造函数调用
    translator.translate();

    outFile.close();
    stream.close();

    std::cout << "Translation completed. Output written to " << output_file << std::endl;

    return 0;
}
