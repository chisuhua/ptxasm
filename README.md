# ptxasm

A tool to translate NVIDIA PTX assembly code to a custom `coasm` format.

## Prerequisites

- CMake (>= 3.12)
- ANTLR 4 C++ Target (>= 4.13 recommended)
- C++17 compatible compiler (e.g., GCC, Clang)

### Installing ANTLR 4 C++

1.  **Download ANTLR:** Get the complete ANTLR jar (e.g., `antlr-4.13.1-complete.jar`) from [https://www.antlr.org/download.html](https://www.antlr.org/download.html).
2.  **Install Java:** Ensure Java is installed (`java -version`).
3.  **Install C++ Runtime:**
    *   **Option 1 (Package Manager - Easier):** On Ubuntu/Debian: `sudo apt install libantlr4-runtime-dev`. On others, find the equivalent package.
    *   **Option 2 (Manual Build - More Control):**
        *   Download the ANTLR C++ runtime source code.
        *   Follow the build instructions (usually involves CMake). Place the built library (e.g., `libantlr4-runtime.a` or `libantlr4-runtime.so`) in a known location (e.g., `/usr/local/lib`).

## Building

1.  Edit `CMakeLists.txt`:
    *   Set `ANTLR_EXECUTABLE` to the path of your downloaded `antlr-4.13.1-complete.jar`.
    *   Set `target_link_directories` and `target_link_libraries` to point to your installed ANTLR C++ runtime library. Adjust paths and library names as needed based on your installation method.
2.  Create a build directory and run CMake:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

## Usage

```bash
./ptxasm <input.ptx> <output.asm>
