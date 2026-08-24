#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "litval.hpp"
#include "token.hpp"
#include "exception_handler.hpp"
#include "stmt.hpp"

class Environment
{
private:
	using Datamap = std::map<std::string, LitVal>;

	Datamap m_env{};
	std::shared_ptr<Environment> m_parent{ nullptr };

public:
	Environment() = default;

	Environment(std::shared_ptr<Environment> parent)
		: m_parent{ parent }
	{
	}

	void define(Token name, LitVal value)
	{
		m_env[name.m_lexeme] = value;
	}

	void assign(const Token& name, LitVal value)
	{
		if (m_env.contains(name.m_lexeme))
			define(name, value);

		else if (m_parent)
			m_parent.get()->assign(name, value);

		else
		{
			std::string message = "Undeclared variable '" + name.m_lexeme + "'.";

			throw RuntimeException(name, message, name.m_lineNum, EXIT_CODE::UNDCLD_VAR);
		}
	}

	LitVal value(const Token& name)
	{
		if (m_env.contains(name.m_lexeme))
			return m_env[name.m_lexeme];

		if (m_parent)
			return m_parent->value(name);
		
		
		std::string message = "Undeclared variable '" + name.m_lexeme + "'.";

		throw RuntimeException(name, message, name.m_lineNum, EXIT_CODE::UNDCLD_VAR);
	}

	bool has(const Token& name)
	{
		if (m_env.contains(name.m_lexeme))
			return true;

		if (m_parent)
			m_parent->has(name);

		return false;
	}
};