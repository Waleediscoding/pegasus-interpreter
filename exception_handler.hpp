#pragma once

#include <iostream>
#include <exception>
#include <string>

#include "token.hpp"


enum class EXIT_CODE : int
{
	SUCCESS = 0,
	FAILURE,

	// CLI
	USAGE_ERR = 100,

	// data/syntax
	INVALID_CHAR,
	INVALID_KEYW,
	UNTERMINATED_STR,
	UNTERMINATED_CMT,
	MISSING_CLOSED_BRACKET,
	MISSING_OPEN_BRACKET,
	MISSING_CLOSED_BRACE,
	MISSING_COLON,
	EXPECTED_OPD,
	EXPECTED_EXPR,
	EXPECTED_SEMICLN,
	EXPECTED_IDNTF,
	EXPECTED_VAR,
	EXPECTED_COND,
	BREAK_NOT_IN_LOOP,
	// runtime
	TYPE_MISMATCH,
	STR_MANIPULATION,
	DIV_ZERO,
	UNDCLD_VAR,

	// file system
	FILE_ERR,

	UNIDENTIFIABLE
};

namespace ErrorFlag
{
	inline bool errorRaised{ false };
}

struct BreakSignal {};
struct ContinueSignal{};

class InterpreterException : public std::exception
{
private:
	EXIT_CODE m_code{ EXIT_CODE::UNIDENTIFIABLE };
	std::string m_message{ "Unidentifiable error." };
	int m_lineNum{ -1 };
	bool m_terminate{ true };

public:
	InterpreterException() = default;

	InterpreterException(EXIT_CODE code, std::string message)
		: m_code{ code }, m_message{ std::move(message) }
	{
	}

	InterpreterException(EXIT_CODE code, std::string message, int lineNum, bool terminate)
		: m_code{ code }, m_message{ std::move(message) },
		m_lineNum{ lineNum }, m_terminate{ terminate }
	{
	}

	void report() const noexcept
	{
		ErrorFlag::errorRaised = true;
		if (!shouldTerminate())
		{
			std::cerr << "[Line " << lineNum() << "] Error: " << what() << " (E" << exitCodeInt() << ")\n";
		}
		else
		{
			std::cerr << what() << " (E" << exitCodeInt() << ")\n";
			std::exit(exitCodeInt());
		}
	}

	const char* what() const noexcept override
	{
		return m_message.c_str();
	}

	EXIT_CODE exitCode() const noexcept
	{
		return m_code;
	}

	int exitCodeInt() const noexcept
	{
		return static_cast<int>(m_code);
	}

	int lineNum() const noexcept
	{
		return m_lineNum;
	}

	bool shouldTerminate() const noexcept
	{
		return m_terminate;
	}
};

class RuntimeException : public std::exception
{
private:
	Token m_op;
	std::string m_message;
	int m_lineNum{ -1 };
	EXIT_CODE m_exitCode;

public:
	RuntimeException(Token op, std::string message, int lineNum, EXIT_CODE exitCode)
		: m_op{ op }, m_message{ message }, m_lineNum{ lineNum }, m_exitCode{ exitCode }
	{
	}

	void report() const noexcept
	{
		ErrorFlag::errorRaised = true;
		std::cerr << "[Line " << lineNum() << "] Error: " << what() << " (E" << exitCodeInt() << ")\n";
	}

	const char* what() const noexcept override
	{
		return m_message.c_str();
	}

	Token op() const noexcept
	{
		return m_op;
	}

	int lineNum() const noexcept
	{
		return m_lineNum;
	}

	int exitCodeInt() const noexcept
	{
		return static_cast<int>(m_exitCode);
	}
};