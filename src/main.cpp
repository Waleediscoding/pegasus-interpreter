#include <iostream>

#include "interpreter.hpp"
#include "exception_handler.hpp"

int main(int argc, char** argv)
{
	Interpreter pegasus{};

	try
	{
		pegasus.main(argc, argv);
	}
	catch (InterpreterException& e)
	{
		// Terminal errors
		e.report();
	}
	
	return 0;
}
