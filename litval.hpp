#pragma once

#include <variant>
#include <string>

using LitVal = std::variant<double, bool, std::string, std::nullptr_t>;