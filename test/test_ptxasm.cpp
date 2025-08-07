#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib> // for system
#include <filesystem> // C++17, ensure compiler supports it, or use dirent.h/FindFirstFile

namespace fs = std::filesystem;

const std::string PTXASM_EXECUTABLE = "../build/ptxasm"; // Adjust path as needed
const std::string TEST_PTX_FILE = "test_vector_add.ptx";
const std::string GENERATED_COASM_FILE = "generated_vector_add.asm";
const std::string EXPECTED_COASM_FILE = "expected_vector_add.asm";

bool fileExists(const std::string& filename) {
    return fs::exists(filename);
}

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for reading.\n";
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void runTest() {
    std::cout << "Starting ptxasm test...\n";

    // 1. Check if ptxasm executable exists
    if (!fileExists(PTXASM_EXECUTABLE)) {
        std::cerr << "Error: ptxasm executable not found at " << PTXASM_EXECUTABLE << "\n";
        std::exit(EXIT_FAILURE);
    }

    // 2. Check if test PTX file exists
    if (!fileExists(TEST_PTX_FILE)) {
        std::cerr << "Error: Test PTX file not found at " << TEST_PTX_FILE << "\n";
        std::exit(EXIT_FAILURE);
    }

    // 3. Run ptxasm
    std::string command = PTXASM_EXECUTABLE + " " + TEST_PTX_FILE + " " + GENERATED_COASM_FILE;
    std::cout << "Running command: " << command << "\n";
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Error: ptxasm execution failed with exit code " << result << "\n";
        std::exit(EXIT_FAILURE);
    }

    // 4. Check if generated COASM file exists
    if (!fileExists(GENERATED_COASM_FILE)) {
        std::cerr << "Error: Generated COASM file not found at " << GENERATED_COASM_FILE << "\n";
        std::exit(EXIT_FAILURE);
    }

    // 5. Read generated and expected COASM files
    std::string generatedContent = readFileToString(GENERATED_COASM_FILE);
    std::string expectedContent = readFileToString(EXPECTED_COASM_FILE);

    if (generatedContent.empty() || expectedContent.empty()) {
        std::cerr << "Error: Failed to read generated or expected COASM file content.\n";
        std::exit(EXIT_FAILURE);
    }

    // 6. Compare content (simple string comparison)
    // Note: This is a very basic comparison. For more robust testing,
    // a diff tool or a more sophisticated comparison logic might be needed
    // to handle whitespace, comment differences, or order-insensitive sections.
    if (generatedContent == expectedContent) {
        std::cout << "Test PASSED: Generated COASM matches expected output.\n";
    } else {
        std::cout << "Test FAILED: Generated COASM does not match expected output.\n";
        std::cout << "--- Generated Content ---\n" << generatedContent << "\n--- End Generated ---\n";
        std::cout << "\n--- Expected Content ---\n" << expectedContent << "\n--- End Expected ---\n";
        // Write diff to a file for easier inspection?
        std::system(("diff -u " + EXPECTED_COASM_FILE + " " + GENERATED_COASM_FILE + " > test_diff.patch").c_str());
        std::cout << "Diff written to test_diff.patch\n";
        std::exit(EXIT_FAILURE);
    }

    // 7. Cleanup (optional)
    // fs::remove(GENERATED_COASM_FILE); // Uncomment if you want to clean up

    std::cout << "Test completed successfully.\n";
}

int main() {
    try {
        runTest();
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred during test: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
