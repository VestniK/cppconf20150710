#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "formatutils.h"
#include "handler.h"

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
