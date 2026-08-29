#pragma once

#include <variant>
#include <string>
#include <memory>

class Callable;

using LitVal = std::variant<double, bool, std::string, std::shared_ptr<Callable>, std::nullptr_t>;