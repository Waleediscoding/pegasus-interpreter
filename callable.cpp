#include "callable.hpp"
#include "evaluator.hpp"

inline LitVal Clock::call(Evaluator& ev, const std::vector<LitVal>& args)
{
	return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

inline LitVal Function::call(Evaluator& ev, const std::vector<LitVal>& args)
{
	std::shared_ptr<Environment> env{ std::make_shared<Environment>(ev.globals) };

	for (size_t i{}; i < m_declaration->m_params.size(); ++i)
	{
		auto param{ m_declaration->m_params.at(i) };
		auto arg{ args.at(i) };

		env->define(param.m_lexeme, arg);
	}

	ev.executeBlock(m_declaration->m_block.get(), env);

	return nullptr;
}