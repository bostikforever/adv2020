#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>

namespace {

class Solution {

    using Instr = void(int& pc, int& accum, int arg);
    static const std::unordered_map<std::string, Instr *> instructions;

    int solution = 0;
    int calls = 0;

    static void jump(int& pc, int& accum, int arg)
    {
        pc += arg;
    }

    static void acc(int& pc, int& accum, int arg)
    {
        accum += arg;
        pc += 1;
    }

    static void noop(int& pc, int& accum, int arg)
    {
        pc += 1;
    }

    static void execute(int& pc, int& accum, const std::string& op, int arg)
    {
         instructions.at(op)(pc, accum, arg);
    }

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::vector<std::tuple<std::string, int, int>> instructions;
        int pc = 0;
        int acc = 0;
        const auto ret = loop([&](const std::string& op, int arg) {
            instructions.emplace_back(op, arg, 0);
            while (pc < instructions.size()) {
                auto& ins_count = instructions[pc];
                if (std::get<2>(ins_count)++ != 0) {
                    return false;
                }
                execute(pc,
                        acc,
                        std::get<0>(ins_count),
                        std::get<1>(ins_count));
            }
            return true;
        });
        solution = acc;
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

const std::unordered_map<std::string, Solution::Instr *> Solution::instructions
                                                  { {"acc", &Solution::acc},
                                                    {"jmp", &Solution::jump},
                                                    {"nop", &Solution::noop} };

template <typename CALLABLE>
void readBatches(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;
    int arg;

    const auto ret = callable([&](auto&& body) {
        bool res;
        while (inputFile >> line >> arg && (res = body(line, arg)))
            ;
        return res;
    });
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
