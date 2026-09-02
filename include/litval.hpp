#pragma once

#include <memory>
#include <string>
#include <variant>

class Callable;

using LitVal = std::variant<double, bool, std::string, std::shared_ptr<Callable>, std::nullptr_t>;