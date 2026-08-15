#include <iostream>

#include "interpreter.hpp"
#include "exception_handler.hpp"

int main(int argc, char** argv)
{
	Interpreter lox{};

	std::cout << "";

	try
	{
		lox.main(argc, argv);
	}
	catch (InterpreterException& e)
	{
		// Terminal errors
		e.report();
	}
	
	return 0;
}
