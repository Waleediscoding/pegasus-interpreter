# Pegasus Interpreter

**Pegasus** is an interpreted, dynamically-typed, procedural programming language. It is named for its lightweight design and minimal payload.

## Niche & Purpose
**Pegasus** is a general-purpose language, meaning it can theoretically implement any computable algorithm, as the language is **Turing-complete** (to a pragmatic degree).

I created the interpreter for it as a personal project to learn more about software design, sharpen my programming skills in a systems-level programming language such as C++ (which is the language the interpreter is fully written in), and to become comfortable with the process of building software in general.

As I still am an amatuer, the program does consist of bugs which weren't identified in testing, but I am actively working on it to fix them as of now.

I advise **not** to run the interpreter on devices without an **OS-level Memory Manager/RAM Cleaner**, due to there potentially being unnoticed **Segmentation Fault/Memory Access Violation** errors.

## Building Process
A `CMakeLists.txt` file is present along with the source code to build the binary locally. As an example, you can use tools such as VS Code with its CMake extension to build the program.