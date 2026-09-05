#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "environment.hpp"
#include "evaluator.hpp"
#include "exception_handler.hpp"
#include "expr.hpp"
#include "parser.hpp"
#include "scanner.hpp"
#include "stmt.hpp"
#include "token.hpp"

class Interpreter
{
  private:
    using StmtPtrs = std::vector<std::unique_ptr<Stmt>>;

    void run(std::string_view source, int lineNum)
    {
        try
        {
            Scanner scanner{source, lineNum};

            std::vector<Token> tkns{scanner.scanTokens()};

            Parser parser{tkns};

            Evaluator evaluator{};

            StmtPtrs stmts{parser.parse()};

            evaluator.interpret(stmts);
        } catch (InterpreterException& e)
        {
            e.report();
        }
    }

    void run(std::string_view source) { run(source, 1); }

    void runREPL()
    {
        std::string line{};
        int lineNum{};

        for (;;)
        {
            ++lineNum;
            std::cout << "> ";
            std::getline(std::cin, line);
            if (!std::cin)
                break;

            run(line, lineNum);

            ErrorFlag::errorRaised = false;
        }
    }

    void runFile(char* path)
    {
        std::ifstream fileBinary(path, std::ios::binary | std::ios::ate);

        if (!fileBinary.is_open())
        {
            throw InterpreterException(EXIT_CODE::FILE_ERR, "File could not be opened", true);
        }

        // size of file
        std::streamsize size{fileBinary.tellg()};
        fileBinary.seekg(0, std::ios::beg);

        std::string strBinary(size, '\0');

        fileBinary.read(&strBinary[0], size);

        run(strBinary);

        fileBinary.close();
    }

    void introMessageREPL() { std::cout << "REPL environment booted up successfully.\n"; }

    void terminateProgram()
    {
        EXIT_CODE exitCode{ErrorFlag::errorRaised ? EXIT_CODE::FAILURE : EXIT_CODE::SUCCESS};

        int exitCodeInt{static_cast<int>(exitCode)};

        std::cout << "\nExited Succesfully. Exit status " << exitCodeInt;
        std::exit(exitCodeInt);
    }

  public:
    void main(int argc, char** argv)
    {
        if (argc > 2)
        {
            throw InterpreterException(EXIT_CODE::USAGE_ERR, "Usage: <pegasus file path> <file>", true);
        }

        else if (argc == 1)
        {
            introMessageREPL();
            runREPL();
        }
        else
        {
            runFile(argv[1]);
        }

        terminateProgram();
    }
};