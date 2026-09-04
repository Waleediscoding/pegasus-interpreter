#include "token.hpp"
#include "callable.hpp"

namespace TokenFxns {
std::string literalToStr(LitVal literal)
{
    // Resolve LitVal runtime type into string
    std::string ret = std::visit(
        [](const auto& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::nullptr_t>)
                return "nil";
            else if constexpr (std::is_same_v<T, bool>)
                return arg ? "true" : "false";
            else if constexpr (std::is_same_v<T, double>)
                return std::format("{}", static_cast<double>(arg));
            else if constexpr (std::is_same_v<T, std::shared_ptr<Callable>>)
                return arg ? arg->toString() : "nil";
            else if constexpr (std::is_same_v<T, std::string>)
                return arg;
            else
                static_assert(!sizeof(T*), "Unresolved type in LitVal visitor.");
        },
        literal);

    return ret;
}
} // namespace TokenFxns
