#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

namespace {

class Solution {

    static constexpr std::size_t N = 20;

    uint64_t solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        using Range = std::pair<uint16_t, uint16_t>;
        const auto makeRule = [](Range range) {
            return [range](auto num) {
                return range.first <= num && num <= range.second;
            };
        };
        using RangeTest = decltype(makeRule(std::declval<Range>()));
        std::vector<RangeTest> rules;
        const auto ret = loop([&](auto&& innerRules,
                                  auto&& innerOwnTicket,
                                  auto&& innerOtherTickets) {
            using Ranges = std::initializer_list<Range>;
            innerRules([&](const std::string& name, Ranges ranges) {
                // std::cout << name << ": ";
                for (auto range : ranges) {
                    // std::cout << range.first << ", " << range.second << " ";
                    rules.emplace_back(makeRule(range));
                }
            });

            innerOwnTicket([](const std::vector<uint16_t>& nums) {
            });

            innerOtherTickets([&](std::vector<uint16_t>& nums) {
                // std::cout << nums.size();
                // for (auto num: nums) std::cout << num << " ";
                // std::cout << '\n';
                solution += std::accumulate(
                                std::partition(
                                          nums.begin(),
                                          nums.end(),
                                          [&](auto num) {
                                            return std::any_of(
                                                          rules.begin(),
                                                          rules.end(),
                                                          [num](auto rule) {
                                                              return rule(num);
                                                          });
                                         }),
                                nums.end(),
                                0);
            });
            // std::cout << inner_calls << '\n';
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

template <typename T>
void readNums(std::vector<T> *nums, const std::string& line)
{
    nums->clear();
    T num;
    char ch;
    std::istringstream lineStream(line);
    while(lineStream >> num) {
        nums->emplace_back(num);
        lineStream >> ch; // throw-away
    }
}

template <typename CALLABLE>
void readBatches(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;
    std::vector<uint16_t> lineNumbers;

    const std::regex rules_re(
                           R"<kk>(((\w| )+): (\d+)-(\d+) or (\d+)-(\d+))<kk>");
    while (inputFile && callable([&](auto&& outer) {
        bool res = true;
        std::smatch match;
        // std::cout << line << '\n';
        res = outer(
          [&](auto&& rules) {
            while (getline(inputFile, line) &&
                                std::regex_match(line, match, rules_re)) {
              rules(match[1].str(),
                    { { std::stoi(match[3].str()),
                        std::stoi(match[4].str()) },
                      { std::stoi(match[5].str()),
                        std::stoi(match[6].str()) } });
            }
          },
          [&](auto&& ownTicket) {
              getline(inputFile, line);  // throw-away
              getline(inputFile, line);
              readNums(&lineNumbers, line);
              ownTicket(lineNumbers);
          },
          [&](auto&& otherTickets) {
            getline(inputFile, line);  // throw-away
            getline(inputFile, line);  // throw-away
            while (getline(inputFile, line)) {
              readNums(&lineNumbers, line);
              otherTickets(lineNumbers);
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
