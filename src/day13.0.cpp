#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <utility>

namespace {

class Solution {

    static constexpr char key[] = "shiny gold";

    int solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        int min_wait_time = INT_MAX;
        int wait_id;
        const auto ret = loop([&](auto departure, auto&& inner) {
            // std::cout << departure << ' ';
            int inner_calls = 0;
            inner([&](auto id) {
                // std::cout << id << ' ';
                const auto earliest_departure_id =
                                              ((departure + id - 1) / id) * id;
                const auto wait_time_id = earliest_departure_id - departure;

                if (wait_time_id < min_wait_time) {
                    min_wait_time = wait_time_id;
                    wait_id = id;
                }
                ++inner_calls;
            });
            // std::cout << inner_calls << '\n';
            return true;
        });
        solution = min_wait_time * wait_id;
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

    while (inputFile && callable([&](auto&& outer) {
        bool res = true;
        int arg;
        while (inputFile >> arg && res) {
            std::string line;
            res = outer(arg, [&](auto&& inner) {
                inputFile >> line;
                const std::regex re(R"<seq>((^|,)(\d+))<seq>");
                std::sregex_token_iterator it(line.begin(),
                                              line.end(),
                                              re,
                                              2);
                while (it != std::sregex_token_iterator()) {
                    inner(std::stoi(*(it++)));
                }
            });
        }
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
