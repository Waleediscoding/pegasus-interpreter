#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "token.hpp"
#include "litval.hpp"

class Visitor;

class CommaExpr;
class Ternary;
class Binary;
class Assignment;
class Unary;
class Grouping;
class Literal;
class Variable;
class Keyword;
class Call;

class Visitor {
public:
	virtual ~Visitor() = default;

	virtual void visitCommaExpr(CommaExpr& expr) = 0;

	virtual void visitTernaryExpr(Ternary& expr) = 0;

	virtual void visitBinaryExpr(Binary& expr) = 0;

	virtual void visitAssignExpr(Assignment& expr) = 0;

	virtual void visitGroupingExpr(Grouping& expr) = 0;

	virtual void visitUnaryExpr(Unary& expr) = 0;

	virtual void visitVarExpr(Variable& expr) = 0;

	virtual void visitLiteralExpr(Literal& expr) = 0;

	virtual void visitKeywordExpr(Keyword& expr) = 0;

	virtual void visitCallExpr(Call& expr) = 0;
};

class Expr
{
public:

	virtual ~Expr() = default;

	virtual void accept(Visitor& visitor) = 0;
};


class CommaExpr : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;
	using ExprPtrs = std::vector<ExprPtr>;

	const ExprPtrs m_exprPtrs;
	const int m_lineNum;

	CommaExpr(ExprPtrs exprPtrs, int lineNum)
		: m_exprPtrs{ std::move(exprPtrs) }, m_lineNum{ lineNum }
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitCommaExpr(*this);
	}
};

class Ternary : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const ExprPtr m_condition;
	const ExprPtr m_left;
	const ExprPtr m_right;
	const int m_lineNum;

	Ternary(ExprPtr condition, ExprPtr left, ExprPtr right, int lineNum)
		: m_condition{std::move(condition)}, m_left{std::move(left)}, m_right{std::move(right)}
		, m_lineNum{ lineNum }
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitTernaryExpr(*this);
	}
};

class Binary : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const ExprPtr m_left;
	const Token m_op;
	const ExprPtr m_right;

	Binary(ExprPtr left, Token op, ExprPtr right)
		: m_left{ std::move(left) }, m_op{ op }, m_right{ std::move(right) }
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitBinaryExpr(*this);
	}

};

class Assignment : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const Token m_name;
	const Token m_op;
	const ExprPtr m_expression;

	Assignment(Token name, Token op, ExprPtr expression)
		: m_name{ name }, m_op{ op }, m_expression{ std::move(expression) }
	{
	}

	void accept(Visitor& visitor)
	{
		visitor.visitAssignExpr(*this);
	}
};

class Grouping : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const ExprPtr m_expression;

	Grouping(ExprPtr expression)
		: m_expression{ std::move(expression) }
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitGroupingExpr(*this);
	}

};

class Unary : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const Token m_op;
	const ExprPtr m_right;

	Unary(Token op, ExprPtr right)
		: m_op{ op }, m_right{ std::move(right) }
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitUnaryExpr(*this);
	}
};

class Literal : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const LitVal m_value;

	Literal(LitVal value)
		: m_value{ value }
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitLiteralExpr(*this);
	}
};

class Variable : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	Token m_name;

	Variable(Token name)
		: m_name{ name }
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitVarExpr(*this);
	}
};

class Keyword : public Expr
{
public:
	Token m_name;

	Keyword(Token name)
		: m_name{name}
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitKeywordExpr(*this);
	}
};

class Call : public Expr
{
public:
	using ExprPtr = std::unique_ptr<Expr>;
	using CommaExprPtr = std::unique_ptr<CommaExpr>;

	ExprPtr m_callee;

	Token m_paren;
	CommaExprPtr m_args;

	Call(ExprPtr callee, Token paren, CommaExprPtr args)
		: m_callee{ std::move(callee) }, m_paren{ paren }, m_args{ std::move(args) }
	{
	}

	void accept(Visitor& visitor) override
	{
		visitor.visitCallExpr(*this);
	}
};