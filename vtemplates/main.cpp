#include <exception>
#include <sstream>
#include <iostream>
#include <string>

#include "commandrunner.h"

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
    cmd = {};
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
    } catch (const std::exception &err) {
        std::cerr << "Error: " << err.what() << std::endl;
    }
    return EXIT_SUCCESS;
}
