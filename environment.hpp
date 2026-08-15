#pragma once

#include <map>
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
	using VarCoords = std::pair<int, int>;
	using VarData = std::pair<std::string, LitVal>;
	using Datamap = std::map<VarCoords, VarData>;

	Datamap m_env{};

	VarCoords resolveCoords(Token varName)
	{
		VarCoords ret;
		for (auto it = m_env.rbegin(); it != m_env.rend(); ++it)
		{
			const auto& [name, value] = it->second;

			if (varName.m_lexeme == name)
			{
				return it->first;
			}
		}

		std::string message = "Undeclared variable '" + varName.m_lexeme + "'.";

		throw RuntimeException(varName, message, varName.m_lineNum, EXIT_CODE::UNDCLD_VAR);
	}

public:
	void define(Token name, LitVal value)
	{
		m_env[resolveCoords(name)] = VarData{name.m_lexeme, value};
	}

	void define(VarCoords coords, Token name, LitVal value)
	{
		m_env[coords] = VarData{ name.m_lexeme, value };
	}

	LitVal value(const Token& name)
	{
		if (m_env.contains(resolveCoords(name)))
			return m_env[resolveCoords(name)].second;
		
		std::string message = "Undeclared variable '" + name.m_lexeme + "'.";

		throw RuntimeException(name, message, name.m_lineNum, EXIT_CODE::UNDCLD_VAR);
	}
	
	LitVal value(VarStmt& var)
	{
		if (m_env.contains(var.coords()))
			return m_env[var.coords()].second;

		std::string message = "Undeclared variable '" + var.m_name.m_lexeme + "'.";

		throw RuntimeException(var.m_name, message, var.m_name.m_lineNum, EXIT_CODE::UNDCLD_VAR);
	}

	bool has(Token name)
	{

		return m_env.contains(resolveCoords(name));
	}

	Datamap* env()
	{
		return &m_env;
	}

	void pop(int depth)
	{
		for (auto it = m_env.begin(); it != m_env.end();)
		{
			VarCoords coord = it->first;

			if (coord.first == depth)
				// Sets the iterator to the idx of the element erased
				it = m_env.erase(it);
			else
				++it;
		}
	}
};