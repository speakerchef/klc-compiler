#include <concepts>
#include <iostream>
#include <iterator>
#include <string>

using std::string;

template <std::ranges::range R>
    requires(!std::convertible_to<R, std::string_view>)
// print range
inline void rprintln(string fmt, R &r) {
    std::copy(r.begin(), r.end(),
              std::ostream_iterator<string>(std::cout, ", "));
    std::cout << fmt << std::endl;
}

// Constants
constexpr std::string_view
    ERR_ARGS("Error: Please supply arguments: klc <FILE.knv>");
constexpr std::string_view ERR_FILE("Error: Could not open file.");
