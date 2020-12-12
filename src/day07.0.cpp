#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
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
        std::map<std::string, std::set<std::string>> parents;
        const auto ret = loop([&](const std::string& outer, auto&& inner) {
            // std::cout << outer << ' ';
            int inner_calls = 0;
            inner([&](int cardinality, const std::string& bag) {
                parents[bag].emplace(outer);
                // std::cout << cardinality << ' ' << bag << ' ';
                ++inner_calls;
            });
            // std::cout << inner_calls << '\n';
            return true;
        });
        std::set<std::string> res;
        auto& keyParents = parents[key];
        while (!keyParents.empty()) {
            auto pIt = keyParents.begin();
            res.emplace(*pIt);
            // std::cout << *pIt << '\n';
            auto it = parents.find(*pIt);
            pIt = keyParents.erase(pIt);
            if (it == parents.end()) {
                continue;
            }
            keyParents.insert(it->second.begin(), it->second.end());
        }
        solution = res.size();
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

    while (inputFile && callable([&](auto&& outer) {
        bool res = true;
        while (getline(inputFile, line) && !line.empty() && res) {
            const std::regex bag_re(R"<seq>((^|\d+ )(\w+ \w+))<seq>");
            std::sregex_token_iterator it(line.begin(),
                                          line.end(),
                                          bag_re,
                                          {1, 2});
            // advance empty beginning match
            ++it;
            std::string parent_color = *(it++);
            res = outer(parent_color, [&](auto&& inner) {
                while (it != std::sregex_token_iterator()) {
                    int cardinality = std::stoi(*(it++));
                    std::string child_color = *(it++);
                    inner(cardinality, child_color);
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
