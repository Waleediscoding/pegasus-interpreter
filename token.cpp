#include "token.hpp"

namespace TokenFxns
{
	std::string literalToStr(LitVal literal)
	{
		// Convert data into string at runtime
		std::string ret = std::visit([](const auto& arg) -> std::string
			{
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, std::nullptr_t>)
					return "nil";
				else if constexpr (std::is_same_v<T, bool>)
					return arg ? "true" : "false";
				else if constexpr (std::is_same_v<T, double>)
					return std::format("{}", static_cast<double>(arg));
				else
					return arg;
			},
			literal);

		return ret;
	}

	std::string toString(Token token)
	{
		std::string literalStr{ literalToStr(token.m_literal) };

		std::string ret{ std::format("| {} | {} | {} |", static_cast<int>(token.m_type), token.m_lexeme, literalStr) };
		return ret;
	}
}