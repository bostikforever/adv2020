#include <array>
#include <cassert>
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
        const auto makeRule = [](const std::pair<Range, Range>& ranges) {
            return [=](auto num) {
                return ( ranges.first.first <= num &&
                         num <= ranges.first.second ) ||
                       ( ranges.second.first <= num &&
                         num <= ranges.second.second );
            };
        };

        using Rule = decltype(makeRule(std::declval<std::pair<Range,
                                                              Range>>()));
        using RulePos = std::bitset<N>;

        std::vector< Rule> rules;
        std::vector<RulePos> rulesPos;
        std::vector<std::string> ruleNames;
        std::vector<uint16_t> ownTicket;

        const auto checkRule = [](const auto& test, const auto& nums) {
            assert (nums.size() <= N);
            RulePos res(0);
            for (int i = 0; i < nums.size(); ++i) {
                const bool valid = test(nums[i]);
                res |= valid << i;
            }
            return res;
        };

        const auto checkRules = [&rules, &checkRule](auto&       testResults,
                                                     const auto& nums) {
            RulePos accum(0);
            assert(rules.size() == testResults.size());
            for (int i = 0; i < testResults.size(); ++i) {
                const auto res = checkRule(rules[i], nums);
                // std:cout << res << '\n';
                if (res == 0) return false; // optimisation
                accum |= res;
                testResults[i] = res;
            }
            return accum.flip() == RulePos(0);
        };

        const auto combine = [](auto& lhs, const auto& rhs) {
            assert(lhs.size() == rhs.size());
            for (int i = 0; i < rhs.size(); ++i) {
                lhs[i] &= rhs[i];
            }
        };

        const auto ret = loop([&](auto&& innerRules,
                                  auto&& innerOwnTicket,
                                  auto&& innerOtherTickets) {
            using Ranges = std::initializer_list<Range>;
            innerRules([&](const std::string& name, Ranges ranges) {
                // std::cout << name << ": ";
                ruleNames.emplace_back(name);
                rules.emplace_back(makeRule(std::make_pair(
                                                      *(ranges.begin() + 0),
                                                      *(ranges.begin() + 1))));
                rulesPos.emplace_back(~uint64_t(0));
            });
            std::vector<RulePos> results;
            innerOwnTicket([&](const std::vector<uint16_t>& nums) {
                ownTicket = nums;
            });

            innerOtherTickets([&](const std::vector<uint16_t>& nums) {
                // std::cout << nums.size();
                // for (auto num: nums) std::cout << num << " ";
                // std::cout << '\n';
                results.resize(nums.size());
                if(checkRules(results, nums)) {
                    combine(rulesPos, results);
                }
                // else { invalid, skip }
            });
            // std::cout << inner_calls << '\n';
            return true;
        });

        std::vector<int> ordering(rulesPos.size());
        for (int i = 0; i < rulesPos.size(); ++i) {
            const auto it = std::find_if(rulesPos.begin(),
                                         rulesPos.end(),
                                         [](const auto& val) {
                return val.count() == 1;
            });

            int position = 0;
            uint64_t bits = it->to_ullong();
            while (bits >>= 1) {
                ++position;
            }

            const auto idx = it - rulesPos.cbegin();
            ordering[idx] = position;

            auto mask = *it;
            mask.flip();
            std::transform(rulesPos.cbegin(),
                           rulesPos.cend(),
                           rulesPos.begin(),
                           [&mask](const auto& val) {
                               return val & mask;
                           });
        }

        solution = 1;
        for (int i = 0; i < ruleNames.size(); ++i) {
            constexpr char prefix[] = "departure";
            if (std::strncmp(prefix,
                             ruleNames[i].c_str(),
                             std::strlen(prefix)) == 0) {
                const auto idx = ordering[i];
                // std::cout << "idx: " << idx << ", " << ownTicket[idx] << " ";
                solution *= ownTicket[idx];
            }
        }
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

    const std::regex rules_re(R"<kk>(((\w| )+): (\d+)-(\d+) or (\d+)-(\d+))<kk>");
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
