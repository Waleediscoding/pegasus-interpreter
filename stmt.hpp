#pragma once

#include <initializer_list>
#include <memory>
#include <utility>

#include "token.hpp"
#include "expr.hpp"
#include "litval.hpp"

class ExprStmt;
class PrintStmt;
class InputStmt;
class VarStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class BlockStmt;

class StmtVisitor
{
public:
	virtual void visitExprStmt(ExprStmt& stmt) = 0;

	virtual void visitPrintStmt(PrintStmt& stmt) = 0;

	virtual void visitInputStmt(InputStmt& stmt) = 0;

	virtual void visitVarStmt(VarStmt& stmt) = 0;

	virtual void visitIfStmt(IfStmt& stmt) = 0;

	virtual void visitWhileStmt(WhileStmt& stmt) = 0;

	virtual void visitForStmt(ForStmt& stmt) = 0;

	virtual void visitBlockStmt(BlockStmt& stmt) = 0;
};

class Stmt
{
public:
	virtual ~Stmt() = default;

	virtual void accept(StmtVisitor& visitor) = 0;
};

class PrintStmt : public Stmt
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const ExprPtr m_expression;

	PrintStmt(ExprPtr expression)
		: m_expression{ std::move(expression) }
	{
	}

	void accept(StmtVisitor& visitor) override
	{
		visitor.visitPrintStmt(*this);
	}
};

class InputStmt : public Stmt
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const ExprPtr m_expression;

	InputStmt(ExprPtr expression)
		: m_expression{ std::move(expression) }
	{
	}

	void accept(StmtVisitor& visitor) override
	{
		visitor.visitInputStmt(*this);
	}
};

class ExprStmt : public Stmt
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	const ExprPtr m_expression;

	ExprStmt(ExprPtr expression)
		: m_expression{ std::move(expression) }
	{ }

	void accept(StmtVisitor& visitor) override
	{
		visitor.visitExprStmt(*this);
	}
};

class VarStmt : public Stmt
{
public:
	using ExprPtr = std::unique_ptr<Expr>;
	using VarCoords = std::pair<int, int>;

	const Token m_name;
	const ExprPtr m_initialiser{};
	VarCoords m_coords;

	VarStmt(Token name, ExprPtr initialiser, VarCoords coords)
		: m_name{ name }, m_initialiser{ std::move(initialiser) }, m_coords{ coords }
	{
	}

	void accept(StmtVisitor& visitor) override
	{
		visitor.visitVarStmt(*this);
	}

	VarCoords coords() const
	{
		return m_coords;
	}

	void setCoords(VarCoords coords)
	{
		m_coords = coords;
	}
};

class IfStmt : public Stmt
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	using StmtPtr = std::unique_ptr<Stmt>;

	const ExprPtr m_condition;
	const StmtPtr m_body;

	const StmtPtr m_next{ nullptr };

	const int m_lineNum;

	IfStmt(ExprPtr condition, StmtPtr body, StmtPtr next, int lineNum)
		: m_condition{ std::move(condition) }, m_body{ std::move(body) }, m_next{ std::move(next) }, m_lineNum{ lineNum }
	{
	}

	void accept(StmtVisitor& visitor) override
	{
		visitor.visitIfStmt(*this);
	}
};

class WhileStmt : public Stmt
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	using StmtPtr = std::unique_ptr<Stmt>;

	ExprPtr m_condition;
	StmtPtr m_body;

	const int m_lineNum;

	WhileStmt(ExprPtr condition, StmtPtr body, int lineNum)
		: m_condition{ std::move(condition) }, m_body{ std::move(body) }, m_lineNum{ lineNum }
	{
	}

	void accept(StmtVisitor& visitor) override
	{
		visitor.visitWhileStmt(*this);
	}
};

class ForStmt : public Stmt
{
public:
	using ExprPtr = std::unique_ptr<Expr>;

	using StmtPtr = std::unique_ptr<Stmt>;
	
	using VarStmtPtr = std::unique_ptr<VarStmt>;
	using ExprStmtPtr = std::unique_ptr<ExprStmt>;

	VarStmtPtr m_init;
	ExprStmtPtr m_condition;
	ExprPtr m_expr;
	StmtPtr m_body;

	const int m_lineNum;

	ForStmt(VarStmtPtr init, ExprStmtPtr condition, ExprPtr expr, StmtPtr body, int lineNum)
		: m_init{ std::move(init) }, m_condition{ std::move(condition) }, m_expr{ std::move(expr) }
		, m_body{ std::move(body) }, m_lineNum{ lineNum }
	{
	}

	void accept(StmtVisitor& visitor) override
	{
		visitor.visitForStmt(*this);
	}
};

class BlockStmt : public Stmt
{
public:
	using StmtPtr = std::unique_ptr<Stmt>;
	using StmtPtrs = std::vector<StmtPtr>;

	const StmtPtrs m_stmts;
	const int m_depth;

	BlockStmt(StmtPtrs stmts, int depth)
		: m_stmts{ std::move(stmts) }, m_depth{ depth }
	{
	}

	void accept(StmtVisitor& visitor) override
	{
		visitor.visitBlockStmt(*this);
	}
};