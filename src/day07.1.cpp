#include <cassert>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <numeric>
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
        std::map<std::string, std::set<std::pair<int, std::string>>> child;
        const auto ret = loop([&](const std::string& outer, auto&& inner) {
            // std::cout << outer << ' ';
            int inner_calls = 0;
            inner([&](int cardinality, const std::string& bag) {
                child[outer].emplace(cardinality, bag);
                // std::cout << cardinality << ' ' << bag << ' ';
                ++inner_calls;
            });
            // std::cout << inner_calls << '\n';
            return true;
        });

        std::set<std::pair<int, std::string>> res;
        auto& keyChilds = child[key];
        while (!keyChilds.empty()) {
            auto cIt = keyChilds.begin();
            auto card_bag = *cIt;
            auto it = child.find(card_bag.second);
            res.emplace(*cIt);
            keyChilds.erase(cIt);

            if (it == child.end()) {
                continue;
            }

            std::transform(it->second.begin(),
                           it->second.end(),
                           std::inserter(keyChilds, keyChilds.begin()),
                           [&card_bag](const auto& val) {
                               return std::make_pair(
                                                    val.first * card_bag.first,
                                                    val.second);
                           });
        }
        solution = std::accumulate(res.begin(),
                                   res.end(),
                                   0,
                                   [](int lhs, auto& val) {
                                       return lhs + val.first;
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
