#include <concepts>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <variant>

using std::string;

// Allows for clean handling of std::variant dispatch
template <typename... T> struct Overload : T... {
    using T::operator()...;
};


// print range
template <std::ranges::range R>
    requires(!std::convertible_to<R, std::string_view>)
inline void rprintln(string fmt, R &r) {
    std::copy(r.begin(), r.end(),
              std::ostream_iterator<string>(std::cout, ", "));
    std::cout << fmt << std::endl;
}

template <typename T> inline void print_variant(T &val) {
    std::visit([&](const auto &value) { std::println("`{}`", value); }, val);
}

// Constants
constexpr std::string_view
    ERR_ARGS("Error: Please supply arguments: klc <FILE.knv>");
constexpr std::string_view ERR_FILE("Error: Could not open file.");
