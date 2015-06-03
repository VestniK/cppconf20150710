#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <vector>
#include <tuple>
#include <stdexcept>

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

struct Context {
    std::map<std::string, std::string> vars;

    std::string set(const std::string &var, const std::string &val) {
        vars[var] = val;
        return "OK";
    }

    template<typename T>
    T parse(const std::string &val) {
        std::istringstream iss;
        if (val.find("${") == 0 && val.back() == '}')
            iss.str(vars[val.substr(2, val.size() - 3)]);
        else
            iss.str(val);
        T res;
        iss >> res;
        if (!iss)
            throw std::invalid_argument("%s: Failed to parse argument '%s'"_fmt(__PRETTY_FUNCTION__, val.c_str()));
        return std::move(res);
    }
};

struct Command {
    std::string name;
    std::vector<std::string> params;
};

class CommandRunner {
public:
    CommandRunner(Context &ctx, std::initializer_list<std::pair<std::string, HandlerFunc>> handlers):
        mHandlers(handlers.begin(), handlers.end())
    {
        mHandlers["list"] = [this](const std::vector<std::string>&) {
            std::ostringstream oss;
            oss << "Available functions:";
            for (const auto kv: mHandlers)
                oss << "\n * " << kv.first;
            return oss.str();
        };
        using namespace std::placeholders;
        mHandlers["set"] = Adaptor<Context, std::string(const std::string &, const std::string&)>{
            std::bind(&Context::set, &ctx, _1, _2), ctx
        };
    }

    std::string run(const Command &cmd) const {
        auto it = mHandlers.find(cmd.name);
        if (it == mHandlers.end())
            throw std::invalid_argument("No such command '%s'"_fmt(cmd.name.c_str()));
        return it->second(cmd.params);
    }

private:
    std::map<std::string, HandlerFunc> mHandlers;
};

namespace math {

int add(int a, int b) {return a + b;}
int sub(int a, int b) {return a - b;}
int mul(int a, int b) {return a*b;}
int div(int a, int b) {return a/b;}
int mod(int a, int b) {return a%b;}

}

class SubstrMatcher {
public:
    SubstrMatcher(const std::string &ptrn): mPtrn(ptrn) {}
    bool match(const std::string &str) const {return str.find(mPtrn) != std::string::npos;}

private:
    std::string mPtrn;
};

bool readCmd(std::istream &input, Command &cmd) {
    std::string line;
    while (!input.eof()) {
        std::getline(input, line);
        if (line.empty())
            continue;
        std::istringstream iss(line);
        iss >> cmd.name;
        while (!iss.eof()) {
            std::string arg;
            iss >> arg;
            cmd.params.push_back(arg);
        }
        return true;
    }
    return false;
}

int main() {
    SubstrMatcher fooMatcher("foo");
    SubstrMatcher barMatcher("bar");

    Context ctx;
    const CommandRunner runner = {ctx, {
        {"add", handler(ctx, math::add)},
        {"sub", handler(ctx, math::sub)},
        {"mul", handler(ctx, math::mul)},
        {"div", handler(ctx, math::div)},
        {"mod", handler(ctx, math::mod)},
        {"containsFoo", handler(ctx, fooMatcher, &SubstrMatcher::match)},
        {"containsBar", handler(ctx, barMatcher, &SubstrMatcher::match)},
    }};

    Command cmd;
    while (readCmd(std::cin, cmd)) try {
        std::cout << runner.run(cmd) << std::endl;
        cmd = {};
    } catch (const std::exception &err) {
        std::cerr << "Error: " << err.what() << std::endl;
    }
    return EXIT_SUCCESS;
}
