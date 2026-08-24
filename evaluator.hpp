#pragma once

#include <variant>
#include <vector>

#include "exception_handler.hpp"
#include "expr.hpp"
#include "stmt.hpp"
#include "token.hpp"
#include "environment.hpp"
#include "litval.hpp"

class Evaluator : public Visitor, public StmtVisitor
{
private:
	using StmtPtr = std::unique_ptr<Stmt>;
	using StmtPtrs = std::vector<StmtPtr>;

	LitVal m_result;

	std::shared_ptr<Environment> m_env{std::make_shared<Environment>()};

	bool inLoop{ false };

	// Statement evaluation
	LitVal getExprValue(PrintStmt& stmt)
	{
		stmt.m_expression.get()->accept(*this);
		return m_result;
	}

	LitVal getExprValue(ExprStmt& stmt)
	{
		stmt.m_expression.get()->accept(*this);
		return m_result;
	}

	void visitVarStmt(VarStmt& stmt) override
	{
		LitVal value = eval(stmt.m_initialiser.get());

		m_env->define(stmt.m_name, value);
	}

	void visitPrintStmt(PrintStmt& stmt) override
	{
		LitVal value{ getExprValue(stmt) };

		std::cout << TokenFxns::literalToStr(value) << '\n';
	}

	void visitInputStmt(InputStmt& stmt) override
	{
		auto* pVar = static_cast<Variable*>(stmt.m_expression.get());

		Token name{ (*pVar).m_name };

		std::string line{ };

		std::getline(std::cin >> std::ws, line);

		LitVal value{ strToLit(line) };

		m_env->assign(name, value);
	}

	void visitBlockStmt(BlockStmt& stmt) override
	{
		// Reference to enclosing scope
		std::shared_ptr<Environment> prev{ m_env };

		if (!inLoop)
		{
			try
			{
				// Assigning new scope with the enclosing scope reference
				m_env = std::make_shared<Environment>(prev);

				for (auto& substmt : stmt.m_stmts)
				{
					substmt->accept(*this);
				}
			}
			catch (...)
			{
				m_env = prev;
				throw;
			}
		}
		else
		{
			try
			{
				for (auto& substmt : stmt.m_stmts)
				{
					substmt->accept(*this);
				}
			}
			catch (...)
			{
				m_env = prev;
				throw;
			}
		}

		m_env = prev;
	}

	void visitIfStmt(IfStmt& stmt) override
	{
		LitVal value{ eval(stmt.m_condition.get()) };

		const auto& pBool = std::get_if<bool>(&value);

		if (!pBool)
			throw InterpreterException(EXIT_CODE::TYPE_MISMATCH, "Expression must evaluate to a boolean.", stmt.m_lineNum, false);

		if (*pBool)
		{
			stmt.m_body.get()->accept(*this);
			return;
		}
		else if (stmt.m_next)
		{
			stmt.m_next.get()->accept(*this);
			return;
		}

	}

	void visitWhileStmt(WhileStmt& stmt) override
	{
		inLoop = true;

		std::shared_ptr<Environment> prev{ m_env };

		try
		{
			m_env = std::shared_ptr<Environment>(prev);

			LitVal condition{ eval(stmt.m_condition.get()) };

			auto* pCondition = std::get_if<bool>(&condition);

			if (!pCondition)
				throw InterpreterException(EXIT_CODE::TYPE_MISMATCH, "Expression must evaluate to a boolean.", stmt.m_lineNum, false);

			while (*pCondition)
			{
				try
				{
					stmt.m_body->accept(*this);
				}
				catch (ContinueSignal&)
				{
					continue;
				}


				condition = eval(stmt.m_condition.get());

				pCondition = std::get_if<bool>(&condition);
			}
		}
		catch (BreakSignal&)
		{
			// Exits out of loop
		}
		catch (...)
		{
			m_env = prev;
			throw;
		}

		m_env = prev;

		inLoop = false;
	}

	void visitForStmt(ForStmt& stmt) override
	{
		inLoop = true;

		std::shared_ptr<Environment> prev{ m_env };

		try
		{
			m_env = std::make_shared<Environment>(prev);

			stmt.m_init->accept(*this);

			LitVal condition{ eval(stmt.m_condition->m_expression.get()) };

			LitVal initValue{ eval(stmt.m_init->m_initialiser.get()) };

			auto* pCondition = std::get_if<bool>(&condition);

			auto* pInitValue = std::get_if<double>(&initValue);

			if (!pCondition)
				throw InterpreterException(EXIT_CODE::TYPE_MISMATCH, "Expression must evaluate to a boolean.", stmt.m_lineNum, false);

			if (!pInitValue)
				throw InterpreterException(EXIT_CODE::TYPE_MISMATCH, "Expression must evaluate to an integer/float.", stmt.m_lineNum, false);

			for (double i{ *pInitValue }; *pCondition; eval(stmt.m_expr.get()))
			{
				condition = eval(stmt.m_condition->m_expression.get());

				pCondition = std::get_if<bool>(&condition);

				if (!(*pCondition)) break;

				try
				{
					stmt.m_body->accept(*this);
				}
				catch (ContinueSignal&)
				{
					continue;
				}
			}
		}
		catch (BreakSignal&)
		{
			// Exits out of loop
		}
		catch (...)
		{
			m_env = prev;
			throw;
		}

		m_env = prev;

		inLoop = false;
	}

	void visitKeywordStmt(KeywordStmt& stmt) override
	{
		stmt.m_keyword->accept(*this);
	}

	void visitExprStmt(ExprStmt& stmt) override
	{
		getExprValue(stmt);
	}

	// Expression evaluation
	LitVal eval(Expr* expr)
	{
		expr->accept(*this);

		return m_result;
	}

	void visitCommaExpr(CommaExpr& expr) override
	{
		size_t penultimateIdx{ expr.m_exprPtrs.size() - 1 };

		for (size_t i{}; i < penultimateIdx; ++i)
		{
			const auto subexpr = expr.m_exprPtrs[i].get();

			eval(subexpr);
		}

		m_result = eval(expr.m_exprPtrs[penultimateIdx].get());
	}

	void visitTernaryExpr(Ternary& expr) override
	{
		LitVal condition{ eval(expr.m_condition.get()) };
		LitVal thenBranch{ eval(expr.m_left.get()) };
		LitVal elseBranch{ eval(expr.m_right.get()) };


		const auto pCondition = std::get_if<bool>(&condition);

		if (!pCondition)
			throw InterpreterException(EXIT_CODE::TYPE_MISMATCH, "First expression must evaluate to a boolean.", expr.m_lineNum, false);

		m_result = (*pCondition) ? thenBranch : elseBranch;
	}

	void visitBinaryExpr(Binary& expr) override
	{
		LitVal left{ eval(expr.m_left.get()) };
		LitVal right{ eval(expr.m_right.get()) };

		// Returns ptr to obj if true, else null ptr
		const auto pLeftDb = std::get_if<double>(&left);
		const auto pRightDb = std::get_if<double>(&right);

		const auto pLeftStr = std::get_if<std::string>(&left);
		const auto pRightStr = std::get_if<std::string>(&right);

		const auto pLeftBool = std::get_if<bool>(&left);
		const auto pRightBool = std::get_if<bool>(&right);

		switch (expr.m_op.m_type)
		{
		case TokenType::EQUAL:


		case TokenType::PLUS:
		case TokenType::PLUS_EQUAL:
			if (pLeftDb && pRightDb)
			{
				m_result = (*pLeftDb) + (*pRightDb);
				return;
			}
			else if (pLeftStr && pRightStr)
			{
				m_result = (*pLeftStr) + (*pRightStr);
				return;
			}
			else if (pLeftDb && pRightStr)
			{
				m_result = TokenFxns::literalToStr((left)) + (*pRightStr);
				return;
			}
			else if (pLeftStr && pRightDb)
			{
				m_result = (*pLeftStr) + TokenFxns::literalToStr((right));
				return;
			}
			break;

		case TokenType::MINUS:
		case TokenType::MINUS_EQUAL:
			if (pLeftDb && pRightDb)
			{
				m_result = (*pLeftDb) - (*pRightDb);
				return;
			}
			break;

		case TokenType::ASTK:
		case TokenType::ASTK_EQUAL:
			if (pLeftDb && pRightDb)
			{
				m_result = (*pLeftDb) * (*pRightDb);
				return;
			}
			break;

		case TokenType::SLASH:
		case TokenType::SLASH_EQUAL:
			if (pRightDb && ((*pRightDb) == 0.0))
				throw InterpreterException(EXIT_CODE::DIV_ZERO, "Division by zero.", expr.m_op.m_lineNum, false);
			if (pLeftDb && pRightDb)
			{
				m_result = (*pLeftDb) / (*pRightDb);
				return;
			}
			break;

		case TokenType::GREATER:
			if (pLeftDb && pRightDb)
			{
				m_result = (*pLeftDb) > (*pRightDb);
				return;
			}
			break;

		case TokenType::GREATER_EQUAL:
			if (pLeftDb && pRightDb)
			{
				m_result = (*pLeftDb) >= (*pRightDb);
				return;
			}
			break;

		case TokenType::LESS:
			if (pLeftDb && pRightDb)
			{
				m_result = (*pLeftDb) < (*pRightDb);
				return;
			}
			break;

		case TokenType::LESS_EQUAL:
			if (pLeftDb && pRightDb)
			{
				m_result = (*pLeftDb) <= (*pRightDb);
				return;
			}
			break;

		case TokenType::EQUAL_EQUAL:
			m_result = left == right;
			return;
			break;

		case TokenType::BANG_EQUAL:
			m_result = left != right;
			return;
			break;

		case TokenType::AND:
			if (pLeftBool && pRightBool)
			{
				m_result = (*pLeftBool) && (*pRightBool);
				return;
			}
			throw InterpreterException(EXIT_CODE::TYPE_MISMATCH, "Expression must evaluate to a boolean.", expr.m_op.m_lineNum, false);

			break;

		case TokenType::OR:
			if (pLeftBool && pRightBool)
			{
				m_result = (*pLeftBool) || (*pRightBool);
				return;
			}
			throw InterpreterException(EXIT_CODE::TYPE_MISMATCH, "Expression must evaluate to a boolean.", expr.m_op.m_lineNum, false);

			break;
		}

		std::string message = "Incorrect type(s) for operation '" + expr.m_op.m_lexeme + "'.";

		throw RuntimeException(expr.m_op, message, expr.m_op.m_lineNum, EXIT_CODE::TYPE_MISMATCH);
	}

	void visitAssignExpr(Assignment& expr) override
	{
		std::unique_ptr<Binary> temp = std::make_unique<Binary>(
			std::make_unique<Literal>(m_env->value(expr.m_name)),
			expr.m_op,
			std::make_unique<Literal>(eval(expr.m_expression.get()))
		);

		switch (expr.m_op.m_type)
		{
		// Assignment
		case TokenType::EQUAL:
			m_env->assign(expr.m_name, eval(expr.m_expression.get()));
			return;

		// Assignment via operator
		case TokenType::PLUS_EQUAL:
		case TokenType::MINUS_EQUAL:
		case TokenType::ASTK_EQUAL:
		case TokenType::SLASH_EQUAL:
			m_env->assign(expr.m_name, eval(temp.get()));
			return;
		}
	}

	void visitGroupingExpr(Grouping& expr) override
	{
		m_result = eval(expr.m_expression.get());
	}

	void visitUnaryExpr(Unary& expr) override
	{
		LitVal right{ eval(expr.m_right.get()) };

		switch (expr.m_op.m_type)
		{
		case TokenType::MINUS:
			// Check  for proper type
			if (const auto pDouble = std::get_if<double>(&right))
			{
				m_result = -(*pDouble);
				return;
			}
			break;

		case TokenType::BANG:
			if (const auto pBool = std::get_if<bool>(&right))
			{
				m_result = !(*pBool);
				return;
			}
			break;
		}

		std::string message = "Incorrect type(s) for operation '" + expr.m_op.m_lexeme + "'";

		throw RuntimeException(expr.m_op, message, expr.m_op.m_lineNum, EXIT_CODE::TYPE_MISMATCH);
	} 

	void visitVarExpr(Variable& expr) override
	{
		m_result = m_env->value(expr.m_name);
	}

	void visitLiteralExpr(Literal& expr) override
	{
		m_result = expr.m_value;
	}

	void visitKeywordExpr(Keyword& expr) override
	{
		if (!inLoop)
			throw InterpreterException(EXIT_CODE::BREAK_NOT_IN_LOOP, "Keyword outside of a loop.", expr.m_name.m_lineNum, false);

		if (expr.m_name.m_type == TokenType::BREAK)
			throw BreakSignal();
		else if (expr.m_name.m_type == TokenType::CONTINUE)
			throw ContinueSignal();
	}

	LitVal strToLit(const std::string& line)
	{
		if (line == "true") return true;
		if (line == "false") return false;
		if (line == "nil") return nullptr;

		bool isNum{ true };
		bool decimal{ false };

		for (size_t i{}; i < line.length(); ++i)
		{
			char c{ line[i] };

			if (i == 0 && (c == '-' || c == '+'))
			{
				if (line.length() == 1)
				{
					isNum = false;
					break;
				}
			}

			if (c == '.')
			{
				if (decimal)
				{
					isNum = false;
					break;
				}
				decimal = true;
			}

			else if (!std::isdigit(static_cast<unsigned char>(c)))
			{
				isNum = false;
				break;
			}
		}

		if (isNum && line != "-" && line != "+" && line != ".")
		{
			try
			{
				return std::stod(line);
			}
			catch (...)
			{
				return line;
			}
		}

		return line;
	}

public:
	void interpret(const StmtPtrs& stmts)
	{
		try
		{
			for (auto& stmt : stmts)
			{
				stmt.get()->accept(*this);
			}
		}
		catch (RuntimeException& e)
		{
			e.report();
		}
	}
};
