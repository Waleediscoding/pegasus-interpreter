#pragma once

#include <vector>
#include <variant>
#include <memory>
#include <initializer_list>

#include "token.hpp"
#include "tokentype.hpp"
#include "expr.hpp"
#include "stmt.hpp"
#include "environment.hpp"
#include "exception_handler.hpp"
#include "litval.hpp"

class Parser
{
private:
	using ExprPtr = std::unique_ptr<Expr>;
	using ExprPtrs = std::vector<ExprPtr>;

	using StmtPtr = std::unique_ptr<Stmt>;
	using StmtPtrs = std::vector<StmtPtr>;

	using VarStmtPtr = std::unique_ptr<VarStmt>;
	using ExprStmtPtr = std::unique_ptr<ExprStmt>;

	using Tokens = std::vector<Token>;
	using TokenTypes = std::initializer_list<TokenType>;

	using enum TokenType;

	const Tokens m_tokens;
	int m_current{0};

	ExprPtr nullExpr()
	{
		return std::make_unique<Literal>("");
	}

	StmtPtr nullStmt()
	{
		return std::make_unique<ExprStmt>(nullExpr());
	}

	StmtPtr declaration()
	{
		try
		{
			if (match({VAR}))
				return varDeclaration();
			if (match({FN}))
				return funcDeclaration();
			return statement();
		}
		catch (InterpreterException &e)
		{
			e.report();

			synchronise();

			return nullStmt();
		}
	}

	StmtPtr varDeclaration()
	{
		Token name{consume(IDENTIFIER, EXIT_CODE::EXPECTED_IDNTF, "Expected an identifier after 'var' keyword.")};

		ExprPtr initialiser = std::make_unique<Literal>(nullptr);

		if (match({EQUAL}))
			initialiser = expression();

		consume(SEMICOLON, EXIT_CODE::EXPECTED_SEMICLN, "Expected ';' after expression.");

		return std::make_unique<VarStmt>(name, std::move(initialiser));
	}

	StmtPtr funcDeclaration()
	{
		Token name{consume(IDENTIFIER, EXIT_CODE::EXPECTED_IDNTF, "Expected an identifier after 'fn' keyword.")};

		consume(LPAREN, EXIT_CODE::MISSING_OPEN_BRACKET, "Expected '(' after identifier.");

		Tokens parameters{};

		if (!check(RPAREN))
		{
			do
			{
				parameters.push_back(consume(IDENTIFIER, EXIT_CODE::EXPECTED_IDNTF, "Expected an identifier after 'fn' keyword."));
			} while (match({COMMA}));
		}

		consume(RPAREN, EXIT_CODE::MISSING_CLOSED_BRACKET, "Expected ')' after arguments.");

		consume(LBRACE, EXIT_CODE::MISSING_OPEN_BRACE, "Expected '{' to initiate a block.");

		StmtPtr block{blockStatement()};

		return std::make_unique<FunctionStmt>(name, std::move(parameters), std::move(block));
	}

	StmtPtr statement()
	{
		try
		{
			if (match({LBRACE}))
				return blockStatement();
			if (match({RETURN}))
				return returnStatement();
			if (match({IF}))
				return ifStatement();
			if (match({WHILE}))
				return whileStatement();
			if (match({FOR}))
				return forStatement();
			if (match({PRINT}))
				return printStatement();
			if (match({INPUT}))
				return inputStatement();
			if (match({BREAK, CONTINUE}))
				return keywordStatement();

			if (match({ELIF, ELSE}))
			{
				int lineNum{peek().m_lineNum};

				skipBlock();
				throw InterpreterException(EXIT_CODE::INVALID_KEYW, "Invalid if-statement.", lineNum, false);
			}

			return exprStatement();
		}
		catch (InterpreterException &e)
		{
			e.report();

			return nullStmt();
		}
	}

	StmtPtr blockStatement()
	{
		StmtPtrs stmtptrs{};

		while (!check({RBRACE}) && !isAtEnd())
		{
			auto stmt{declaration()};

			stmtptrs.push_back(std::move(stmt));
		};

		consume(RBRACE, EXIT_CODE::MISSING_CLOSED_BRACE, "Missing '}' after block instantiation.");

		return std::make_unique<BlockStmt>(std::move(stmtptrs));
	}

	StmtPtr returnStatement()
	{
		if (match({SEMICOLON}))
			return std::make_unique<ReturnStmt>(nullptr, peek().m_lineNum);

		ExprPtr expr{expression()};

		consume(SEMICOLON, EXIT_CODE::EXPECTED_SEMICLN, "Expected ';' after expression.");

		return std::make_unique<ReturnStmt>(std::move(expr), peek().m_lineNum);
	}

	StmtPtr ifStatement()
	{
		consume(LPAREN, EXIT_CODE::MISSING_OPEN_BRACKET, "Expected a '(' before expression");

		ExprPtr condition{expression()};

		consume(RPAREN, EXIT_CODE::MISSING_CLOSED_BRACKET, "Expected a ')' after expression");

		StmtPtr body{declaration()};

		StmtPtr elif{nullptr};

		if (match({ELIF}))
		{
			elif = ifStatement();
		}

		if (match({ELSE}))
		{
			StmtPtr elseStmt{declaration()};

			return std::make_unique<IfStmt>(std::move(condition), std::move(body), std::move(elseStmt), peek().m_lineNum);
		}

		return std::make_unique<IfStmt>(std::move(condition), std::move(body), std::move(elif), peek().m_lineNum);
	}

	StmtPtr whileStatement()
	{
		consume(LPAREN, EXIT_CODE::MISSING_OPEN_BRACKET, "Expected a '(' before expression");

		ExprPtr condition{expression()};

		consume(RPAREN, EXIT_CODE::MISSING_CLOSED_BRACKET, "Expected a ')' after expression");

		StmtPtr body{declaration()};

		return std::make_unique<WhileStmt>(std::move(condition), std::move(body), peek().m_lineNum);
	}

	StmtPtr forStatement()
	{
		consume(LPAREN, EXIT_CODE::MISSING_OPEN_BRACKET, "Expected a '(' before expression");

		int lineNum{peek().m_lineNum};

		StmtPtr init{declaration()};
		VarStmtPtr initStmt{dynamic_cast<VarStmt *>(init.get()) ? static_cast<VarStmt *>(init.release()) : nullptr};

		StmtPtr condition{exprStatement()};
		ExprStmtPtr condStmt{static_cast<ExprStmt *>(condition.release())};

		ExprPtr update{expression()};

		consume(RPAREN, EXIT_CODE::MISSING_CLOSED_BRACKET, "Expected a ')' after expression");

		StmtPtr body{statement()};

		return std::make_unique<ForStmt>(std::move(initStmt), std::move(condStmt), std::move(update), std::move(body), lineNum);
	}

	StmtPtr printStatement()
	{
		ExprPtr value{expression()};

		consume(SEMICOLON, EXIT_CODE::EXPECTED_SEMICLN, "Expected ';' after expression.");

		return std::make_unique<PrintStmt>(std::move(value));
	}

	StmtPtr inputStatement()
	{
		ExprPtr value{expression()};

		auto *pVar = dynamic_cast<Variable *>(value.get());

		if (!pVar)
			throw InterpreterException(EXIT_CODE::EXPECTED_IDNTF, "Expected an identifier.", peek().m_lineNum, false);

		consume(SEMICOLON, EXIT_CODE::EXPECTED_SEMICLN, "Expected ';' after identifier.");

		return std::make_unique<InputStmt>(std::move(value));
	}

	StmtPtr keywordStatement()
	{
		ExprPtr keyw = std::make_unique<Keyword>(previous());

		consume(SEMICOLON, EXIT_CODE::EXPECTED_SEMICLN, "Expected ';' after keyword.");

		return std::make_unique<KeywordStmt>(std::move(keyw));
	}

	StmtPtr exprStatement()
	{
		ExprPtr value{expression()};

		consume(SEMICOLON, EXIT_CODE::EXPECTED_SEMICLN, "Expected ';' after expression.");

		return std::make_unique<ExprStmt>(std::move(value));
	}

	ExprPtr expression()
	{
		// Empty line
		if (m_tokens.size() == 1)
			throw InterpreterException();

		return comma();
	}

	ExprPtr comma()
	{
		ExprPtr expr{ternary()};

		ExprPtrs exprs{};
		if (match({COMMA}))
		{
			exprs.push_back(std::move(expr));

			do
			{
				exprs.push_back(ternary());
			} while (match({COMMA}));

			expr = std::make_unique<CommaExpr>(std::move(exprs), peek().m_lineNum);
		}

		return expr;
	}

	ExprPtr arguments()
	{
		ExprPtrs exprs{};

		if (check({RPAREN}))
		{
			return std::make_unique<CommaExpr>(std::move(exprs), peek().m_lineNum);
		}

		try
		{
			exprs.push_back(ternary());
		}
		catch (InterpreterException &e)
		{
			e.report();

			synchronise();
		}

		if (match({COMMA}))
		{
			do
			{
				try
				{
					exprs.push_back(ternary());
				}
				catch (InterpreterException &e)
				{
					e.report();

					synchronise();
				}

			} while (match({COMMA}));
		}

		return std::make_unique<CommaExpr>(std::move(exprs), peek().m_lineNum);
	}

	ExprPtr ternary()
	{
		ExprPtr expr{assignment()};

		if (match({QUESTION}))
		{
			ExprPtr thenBranch{expression()};

			consume(COLON, EXIT_CODE::MISSING_COLON, "Expected ':' after expression.");

			ExprPtr elseBranch{ternary()};

			expr = std::make_unique<Ternary>(std::move(expr), std::move(thenBranch), std::move(elseBranch), peek().m_lineNum);
		}

		return expr;
	}

	ExprPtr assignment()
	{
		ExprPtr expr{equality()};

		if (match({EQUAL, PLUS_EQUAL, MINUS_EQUAL, ASTK_EQUAL, SLASH_EQUAL}))
		{
			Token op{previous()};

			ExprPtr value{assignment()};

			// Check if left expr is identifier
			if (auto varExpr = dynamic_cast<Variable *>(expr.get()))
			{
				Token name = varExpr->m_name;

				return std::make_unique<Assignment>(name, op, std::move(value));
			}

			throw InterpreterException(EXIT_CODE::EXPECTED_IDNTF, "Expected an identifier.", peek().m_lineNum, false);
		}

		return expr;
	}

	ExprPtr equality()
	{
		ExprPtr expr{comparison()};

		while (match({BANG_EQUAL, EQUAL_EQUAL}))
		{
			// Extract operands + operator
			Token op{previous()};
			ExprPtr right{comparison()};
			expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
		}

		return expr;
	}

	ExprPtr comparison()
	{
		ExprPtr expr{term()};

		while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}))
		{
			// Extract operands + operator
			Token op{previous()};
			ExprPtr right{term()};
			expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
		}

		return expr;
	}

	ExprPtr term()
	{
		ExprPtr expr{factor()};

		while (match({PLUS, MINUS, OR}))
		{
			// Extract operands + operator
			Token op{previous()};
			ExprPtr right{factor()};
			expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
		}

		return expr;
	}

	ExprPtr factor()
	{
		ExprPtr expr{unary()};

		while (match({SLASH, ASTK, AND}))
		{
			// Extract operands + operator
			Token op{previous()};
			ExprPtr right{unary()};
			expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
		}

		return expr;
	}

	ExprPtr unary()
	{
		if (match({BANG, MINUS}))
		{
			// Extract operand + operator
			Token op{previous()};

			ExprPtr right{unary()};

			return std::make_unique<Unary>(op, std::move(right));
		}

		return call();
	}

	ExprPtr call()
	{
		ExprPtr expr{primary()};

		if (match({LPAREN}))
		{
			auto *pIdtf = dynamic_cast<Variable *>(expr.get());

			if (!pIdtf)
				throw RuntimeException(peek(), "Object cannot be called. Expected an identifier.", peek().m_lineNum, EXIT_CODE::EXPECTED_IDNTF);

			ExprPtr args{arguments()};

			std::unique_ptr<CommaExpr> pArgs{static_cast<CommaExpr *>(args.release())};

			Token paren{consume(RPAREN, EXIT_CODE::MISSING_CLOSED_BRACKET, "Missing ')' after argument(s).")};

			return std::make_unique<Call>(std::move(expr), paren, std::move(pArgs));
		}

		return expr;
	}

	ExprPtr primary()
	{
		// Extract boolean / nil / number / string / identifier / grouping
		if (match({TRUE}))
			return std::make_unique<Literal>(true);
		if (match({FALSE}))
			return std::make_unique<Literal>(false);
		if (match({NIL}))
			return std::make_unique<Literal>(nullptr);

		if (match({NUMBER, STRING}))
			return std::make_unique<Literal>(previous().m_literal);

		if (match({IDENTIFIER}))
			return std::make_unique<Variable>(previous());

		if (match({LPAREN}))
		{
			if (match({RPAREN}))
				return nullExpr();

			ExprPtr expr{expression()};

			// Check for matching parenthesis
			consume(RPAREN, EXIT_CODE::MISSING_CLOSED_BRACKET, "Expected ')' after expression.");

			expr = std::make_unique<Grouping>(std::move(expr));

			return expr;
		}

		if (match({PLUS, ASTK, SLASH}))
			throw InterpreterException(EXIT_CODE::EXPECTED_OPD, "Missing left operand.", peek().m_lineNum, false);

		throw InterpreterException(EXIT_CODE::EXPECTED_EXPR, "Expected an expression.", peek().m_lineNum, false);
	}

	Token consume(TokenType type, EXIT_CODE code, std::string message)
	{
		if (check(type))
			return advance();

		throw InterpreterException(code, message, peek().m_lineNum, false);
	}

	bool match(TokenTypes types)
	{
		for (TokenType type : types)
		{
			if (check(type))
			{
				advance();
				return true;
			}
		}
		return false;
	}

	void skipBlock()
	{
		if (check({LPAREN}))
		{
			while (!isAtEnd() && !match({RPAREN}))
				advance();

			if (isAtEnd())
				return;

			if (match({LBRACE}))
			{
				while (!isAtEnd() && !match({RBRACE}))
					advance();
				return;
			}
		}

		if (match({LBRACE}))
		{
			while (!isAtEnd() && !match({RBRACE}))
				advance();
			return;
		}

		while (!isAtEnd() && !match({SEMICOLON}))
			advance();
	}

	void synchronise()
	{
		advance();

		while (!isAtEnd())
		{
			if (peek().m_type == SEMICOLON)
				return;

			switch (peek().m_type)
			{
			case CLASS:
			case FN:
			case VAR:
			case FOR:
			case IF:
			case WHILE:
			case PRINT:
			case RETURN:
				return;
			}

			advance();
		}
	}

	Token advance()
	{
		if (!isAtEnd())
			++m_current;
		return previous();
	}

	bool check(TokenType type)
	{
		if (isAtEnd())
			return false;
		return peek().m_type == type;
	}

	bool isAtEnd()
	{
		return peek().m_type == END;
	}

	Token peek()
	{
		return m_tokens.at(m_current);
	}

	Token previous()
	{
		return m_tokens.at(m_current - 1);
	}

	StmtPtrs getStatements()
	{
		StmtPtrs stmts{};

		while (!isAtEnd())
		{
			stmts.push_back(declaration());
		}

		return stmts;
	}

public:
	Parser(Tokens tokens)
		: m_tokens{std::move(tokens)}
	{
	}

	StmtPtrs parse()
	{
		return getStatements();
	}
};