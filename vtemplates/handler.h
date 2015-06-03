#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "cpputils.h"
#include "formatutils.h"

using HandlerFunc = std::function<std::string(const std::vector<std::string>&)>;

template<typename T>
std::string stringify(T &&t) {
    std::ostringstream oss;
    oss << std::boolalpha << std::forward<T>(t);
    return oss.str();
}

template<typename P, typename T>
struct Adaptor;

template<typename P, typename R, typename... A>
struct Adaptor<P, R(A...)> {
    std::function<R(A...)> func;
    P &parser;

    std::string operator() (const std::vector<std::string> &args) {
        if (args.size() != sizeof...(A))
            throw std::invalid_argument("%s: Got %lu arguments while expected %lu"_fmt(__PRETTY_FUNCTION__, args.size(), sizeof...(A)));
        size_t pos = 0;
        std::tuple<typename std::decay<A>::type...> parsedArgs{
            parser.template parse<typename std::decay<A>::type>(args[pos++])...
        };
        return stringify<R>(cpputils::apply(func, parsedArgs));
    }
};

template<typename P, typename R, typename... A>
HandlerFunc handler(P &p, R (&func)(A...)) {
    return Adaptor<P, R(A...)>{func, p};
}

template<typename P, typename O, typename R, typename... A>
HandlerFunc handler(P &p, const O &o, R (O::*member)(A...) const) {
    return Adaptor<P, R(A...)>{[o,member](A... a) {
        return (o.*member)(a...);
    }, p};
}

template<typename P, typename O, typename R, typename... A>
HandlerFunc handler(P &p, const O &o, R (O::*member)(A...)) {
    static_assert(
        cpputils::is_const_member_function_pointer<decltype(member)>::value,
        "Only const member functions are allowed to be registerred as command handlers"
    );
    return {};
}
