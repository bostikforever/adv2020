#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <regex>
#include <stack>
#include <string>
#include <variant>

namespace {

class Solution {

    static constexpr std::size_t N = 20;

    uint64_t solution = 0;
    int calls = 0;

    enum Paren { e_LEFT, e_RIGHT };
    enum Op { e_MUL, e_ADD };
    using Num = uint64_t;
    using NumOrParen = std::variant<Num, Paren>;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::stack<Op> opStack;
        std::stack<NumOrParen> argStack;
        auto apply = [](Op op, auto lhs, auto rhs) {
          switch (op) {
            case e_MUL:
              return lhs * rhs;
            case e_ADD:
              return lhs + rhs;
          }
        };

        const auto ret = loop([&](auto&& inner) {
          inner([&](const std::string& token) {
            // std::cout << token << '\n';
            Num arg;
            if (token.size() == 1) {
              switch (token[0]) {
                case '*':
                  assert(!argStack.empty());
                  opStack.push(e_MUL);
                  return;
                case '+':
                  assert(!argStack.empty());
                  opStack.push(e_ADD);
                  return;
                case '(':
                  argStack.emplace(e_LEFT);
                  return;
                case ')':
                  assert(!argStack.empty());
                  arg = std::get<Num>(argStack.top());
                  argStack.pop(); // remove arg
                  assert(std::get<Paren>(argStack.top()) == e_LEFT);
                  argStack.pop(); // remove left bracket
                  break;
                default: // number
                  arg = std::stoi(token);
              }
            }
            else { // number
              arg = std::stoi(token);
            }
            // for number and residue of bracket close
            if (!opStack.empty()
                              && std::holds_alternative<Num>(argStack.top())) {
              assert(!argStack.empty());
              arg = apply(opStack.top(), std::get<Num>(argStack.top()), arg);
              opStack.pop();
              argStack.pop();
            }
            // std::cout << "arg: " << arg << '\n';
            argStack.emplace(arg);
          });
          // std::cout << "res: " << std::get<Num>(argStack.top()) << '\n';
          assert(argStack.size() == 1);
          assert(opStack.size() == 0);
          solution += std::get<Num>(argStack.top());
          argStack.pop();
          return true;
        });
        return ret;
    }

  public:
    template <typename ...Args>
    bool operator()(Args... args)
    {
        return solve(args...);
    }

    auto output() const
    {
        std::cout << "calls: " << calls << '\n';
        return solution;
    }
};

template <typename CALLABLE>
void readBatches(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;
    std::vector<uint16_t> lineNumbers;

    const std::regex token_re(R"<kk>((\d+)|([()+*]))<kk>");

    while (inputFile &&
                       getline(inputFile, line) && callable([&](auto&& outer) {
        bool res = true;
        std::sregex_token_iterator it(line.begin(),
                                      line.end(),
                                      token_re);
        std::smatch match;
        // std::cout << line << '\n';
        res = outer(
          [&](auto&& inner) {
            while (it != std::sregex_token_iterator()) {
              inner(*(it++));
            }
          });
        return res;
    }));
}

}  // close unnamed namespace

int main(int argc, const char* argv[])
{
    assert(argc > 1);
    std::string filename = argv[1];
    Solution solution;
    readBatches(filename, solution);
    std::cout << solution.output() << '\n';
    return 0;
}
