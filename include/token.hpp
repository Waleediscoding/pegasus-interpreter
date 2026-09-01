#pragma once

#include <variant>
#include <string>
#include <format>
#include <type_traits>
#include <any>
#include <memory>

#include "tokentype.hpp"
#include "litval.hpp"

class Token
{
public:
	TokenType m_type;
	std::string m_lexeme;

	LitVal m_literal;
	int m_lineNum;

	Token(TokenType type, std::string lexeme, LitVal literal, int lineNum)
		: m_type{ type }, m_lexeme{ lexeme }, m_literal{ literal }, m_lineNum{ lineNum }
	{

	}
};

namespace TokenFxns
{
	std::string literalToStr(LitVal literal);

	std::string toString(Token token);
}