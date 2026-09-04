// Author: Waleed

#include <iostream>

#include "exception_handler.hpp"
#include "interpreter.hpp"

int main(int argc, char** argv)
{
    Interpreter pegasus{};

    try
    {
        pegasus.main(argc, argv);
    } catch (InterpreterException& e)
    {
        // Terminal errors
        e.report();
    }

    return 0;
}
