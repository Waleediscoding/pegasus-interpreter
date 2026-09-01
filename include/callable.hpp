#pragma once

#include <ctime>
#include <string>
#include <format>

#include "expr.hpp"
#include "stmt.hpp"
#include "environment.hpp"
#include "litval.hpp"

class Evaluator;

class Callable
{
public:
	std::string m_callee;

	Callable(std::string callee)
		: m_callee{ callee }
	{
	}

	virtual ~Callable() = default;

	virtual LitVal call(Evaluator& ev, const std::vector<LitVal>& args) = 0;

	virtual int arity() = 0;

	const std::string toString()
	{
		return std::string(std::format("<Function {}>", m_callee));
	}
};

class Clock : public Callable
{
public:
	Clock(std::string callee)
		: Callable(callee)
	{
	}

	LitVal call(Evaluator& ev, const std::vector<LitVal>& args) override;

	int arity() override { return 0; };
};

class Function : public Callable
{
public:

	const FunctionStmt* m_declaration;

	Function(std::string callee, const FunctionStmt* declaration)
		: Callable(callee), m_declaration{ std::move(declaration) }
	{
	}

	LitVal call(Evaluator& ev, const std::vector<LitVal>& args) override;

	int arity() override
	{
		return m_declaration->m_params.size();
	}
};