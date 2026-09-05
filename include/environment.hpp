#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "exception_handler.hpp"
#include "litval.hpp"
#include "token.hpp"

class Environment
{
  private:
    using Datamap = std::map<std::string, LitVal>;

    Datamap m_env{};
    std::shared_ptr<Environment> m_parent{nullptr};

  public:
    Environment() = default;

    Environment(std::shared_ptr<Environment> parent) : m_parent{parent} {}

    Environment(std::shared_ptr<Environment> curr, std::shared_ptr<Environment> parent)
        : m_parent{parent}
    {
        m_env = curr->m_env;
    }

    void define(const std::string& nameStr, LitVal value) { m_env[nameStr] = value; }

    void assign(const Token& name, LitVal value)
    {
        if (m_env.contains(name.m_lexeme))
            define(name.m_lexeme, value);

        else if (m_parent)
            m_parent.get()->assign(name, value);

        else
        {
            throwVarError(name);
        }
    }

    LitVal value(const Token& name)
    {
        if (m_env.contains(name.m_lexeme))
            return m_env[name.m_lexeme];

        else if (m_parent)
            return m_parent->value(name);

        else
        {
            throwVarError(name);
        }
    }

    void throwVarError(const Token& name)
    {
        std::string message = "Undeclared variable '" + name.m_lexeme + "'.";

        throw RuntimeException(EXIT_CODE::UNDCLD_VAR, message, name.m_lineNum);
    }
};