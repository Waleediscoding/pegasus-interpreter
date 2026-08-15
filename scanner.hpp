#pragma once

#include <iostream>
#include <string_view>
#include <vector>
#include <variant>
#include <unordered_map>

#include "exception_handler.hpp"
#include "tokentype.hpp"
#include "token.hpp"
#include "litval.hpp"

class Scanner
{
private:
	using enum TokenType;
	LitVal loxNil{ nullptr };

	std::unordered_map<std::string, TokenType> keywords =
	{
		{"and", AND}, {"class", CLASS}, {"else", ELSE},
		{"elif", ELIF}, {"false", FALSE}, {"fn", FN},
		{"for", FOR}, {"if", IF}, {"nil", NIL}, {"or", OR},
		{"print", PRINT}, {"input", INPUT}, {"return", RETURN}, {"super", SUPER},
		{"this", THIS}, {"true", TRUE}, {"var", VAR},
		{"while", WHILE}
	};

	// Data
	std::string m_source;
	std::vector<Token> tokens{};

	// Indices
	int m_start{};
	int m_current{};
	int m_lineNum{ 1 };

	void scanToken()
	{
		char c{ advance() };

		switch (c)
		{
		case '(': addToken(LPAREN); break;
		case ')': addToken(RPAREN); break;
		case '{': addToken(LBRACE); break;
		case '}': addToken(RBRACE); break;
		case ',': addToken(COMMA); break;
		case '.': addToken(DOT); break;
		case '?': addToken(QUESTION); break;
		case ':': addToken(COLON); break;

		case '+':
			addToken(match({ '+' }) ? PLUS_PLUS : match({ '=' }) ? PLUS_EQUAL : PLUS);
			break;

		case '-': 
			addToken(match({ '-' }) ? MINUS_MINUS : match({ '=' }) ? MINUS_EQUAL : MINUS);
			break;

		case '*':
			addToken(match({ '=' }) ? ASTK_EQUAL : ASTK);
			break;

		case '/':
			if (match('/'))
			{
				while (peek() != '\n' && !isAtEnd()) advance();
			}
			else if (match('*'))
			{
				multiLine();
			}
			else
			{
				addToken(match({ '=' }) ? SLASH_EQUAL : SLASH);
			}
			break;

		case ';': addToken(SEMICOLON); break;
		case '!':
			addToken(match('=') ? BANG_EQUAL : BANG);
			break;
		case '=':
			addToken(match('=') ? EQUAL_EQUAL : EQUAL);
			break;
		case '<':
			addToken(match('=') ? LESS_EQUAL : LESS);
			break;
		case '>':
			addToken(match('=') ? GREATER_EQUAL : GREATER);
			break;
		case ' ':
		case '\r':
		case '\t':
			break;
		case '\n':
			++m_lineNum;
			break;
		case '"':
			string();
			break;
		default:
			if (isdigit(c))
			{
				number();
			}
			else if (isalpha(c))
			{
				identifier();
			}
			else
			{
				throw InterpreterException(EXIT_CODE::INVALID_CHAR, "Invalid character.", m_lineNum, false);
			}
			break;
		}
	}

	bool isAtEnd()
	{
		return m_current >= m_source.length();
	}

	char advance()
	{
		return m_source[m_current++];
	}

	void addToken(TokenType type)
	{
		addToken(type, loxNil);
	}

	void addToken(TokenType type, LitVal literal)
	{
		int length{ m_current - m_start };

		std::string text{ m_source.substr(m_start, length) };

		tokens.push_back(Token(type, text, literal, m_lineNum));
	}

	bool match(char expected)
	{
		bool matches{ (m_source[m_current] == expected) && !isAtEnd() };

		if (matches) advance();

		return matches;
	}

	char peek()
	{
		if (isAtEnd()) return '\0';
		return m_source[m_current];
	}

	void string()
	{
		while (peek() != '"' && !isAtEnd())
		{
			if (peek() == '\n') ++m_lineNum;
			advance();
		}

		if (peek() != '"')
		{
			throw InterpreterException(EXIT_CODE::UNTERMINATED_STR, "Expected '\"' at the end of string.", m_lineNum, false);
		}

		// Closing '"'
		advance();

		int length{ m_current - m_start - 2 };

		std::string value{ m_source.substr(m_start + 1, length) };

		addToken(STRING, value);
	}

	void number()
	{
		// Advance through numbers
		while (isdigit(peek()) && !isAtEnd())
			advance();

		// Advance through numbers after decimal
		if (peek() == '.' && isdigit(peekNext()))
		{
			advance();
			while (isdigit(peek()) && !isAtEnd())
				advance();
		}

		int length{ m_current - m_start };

		// Extract number
		std::string value{ m_source.substr(m_start, length) };
		double valueDb{ std::stod(value) };

		addToken(NUMBER, valueDb);
	}

	void identifier()
	{
		// Advance through alphanumerics
		while (isalphaNumeric(peek())) advance();

		int length{ m_current - m_start };

		// Extract identifier
		std::string value{ m_source.substr(m_start, length) };

		// Find identifier from keywords
		auto it{ keywords.find(value) };
		TokenType type{ (it != keywords.end()) ? it->second : IDENTIFIER };

		addToken(type);
	}

	void multiLine()
	{
		// Omit characters between multi-line comment
		while (!(peek() == '*' && peekNext() == '/') && !isAtEnd())
		{
			if (peek() == '\n') ++m_lineNum;

			advance();
		}

		if ((peek() != '*' && peekNext() != '/'))
		{
			throw InterpreterException(EXIT_CODE::UNTERMINATED_CMT, "Expected \"*/\" at the end of multi-line comment.", m_lineNum, false);
		}

		// Omit */
		advance();
		advance();
	}

	bool isdigit(char c)
	{
		return c >= '0' && c <= '9';
	}

	bool isalpha(char c)
	{
		c = static_cast<char>(std::tolower(c));

		return c >= 'a' && c <= 'z';
	}

	bool isalphaNumeric(char c)
	{
		return isalpha(c) || isdigit(c);
	}

	char peekNext()
	{
		if (m_current + 1 >= m_source.length()) return '\0';
		return m_source[m_current + 1];
	}
public:
	Scanner(std::string_view source)
		: m_source{ source }
	{
	}

	Scanner(std::string_view source, int lineNum)
		: m_source{ source }, m_lineNum{ lineNum }
	{
	}

	std::vector<Token> scanTokens()
	{
		while (!isAtEnd())
		{
			m_start = m_current;
			scanToken();
		}

		tokens.push_back(Token(END, "", loxNil, m_lineNum));

		return tokens;
	}
};