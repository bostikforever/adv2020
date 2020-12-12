#include <bitset>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>

namespace {

class Solution {

    static constexpr std::size_t N = 36;

    uint64_t solution = 0;
    int calls = 0;

    template <typename FUNC>
    void forEach(std::string& wildCard, FUNC&& func, int idx = 0)
    {
        while(idx < wildCard.size() && wildCard[idx] != 'X') {
            ++idx;
        }
        if (idx == wildCard.size()) {
            return func(std::bitset<N>(wildCard));
        }
        wildCard[idx] = '0';
        forEach(wildCard, func, idx + 1);
        wildCard[idx] = '1';
        forEach(wildCard, func, idx + 1);
        wildCard[idx] = 'X';
    }

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::unordered_map<uint64_t, uint64_t> memValue;
        std::string clearMask;
        std::string setMask;
        std::string wildCardMask;
        const auto ret = loop([&](const std::string& mask, auto&& inner) {
            // std::cout << mask << ' ';
            int inner_calls = 0;
            clearMask.resize(mask.size());
            std::transform(std::begin(mask),
                           std::end(mask),
                           std::begin(clearMask),
                           [](char ch) {
                               if (ch == 'X') {
                                   return '0';
                               }
                               return '1';
                           });
            wildCardMask.resize(mask.size());
            std::transform(std::begin(mask),
                           std::end(mask),
                           std::begin(wildCardMask),
                           [](char ch) {
                               if (ch != 'X') {
                                   return '0';
                               }
                               return ch;
                           });
            setMask.resize(mask.size());
            std::transform(std::begin(mask),
                           std::end(mask),
                           std::begin(setMask),
                           [](char ch) {
                               if (ch == 'X') {
                                   return '0';
                               }
                               return ch;
                           });
            // std::cout << clearMask << '\n';
            // std::cout << setMask << '\n';
            std::bitset<N> clearMaskBits(clearMask);
            std::bitset<N> setMaskBits(setMask);
            inner([&](uint64_t address, uint64_t value) {
                // std::cout << address << ' ' << value << ' ';
                address = ((setMaskBits | std::bitset<N>(address)) &
                           clearMaskBits).to_ullong();
                forEach(wildCardMask, [&](auto wildCardMaskBits){
                   memValue[(std::bitset<N>(address) |
                             wildCardMaskBits).to_ullong()] = value;
                });
                ++inner_calls;
            });
            // std::cout << inner_calls << '\n';
            return true;
        });
        solution = std::accumulate(memValue.begin(),
                                   memValue.end(),
                                   uint64_t(0),
                                   [](auto accum, auto val) {
                                       return accum + val.second;
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

    const std::regex mask_re(R"<seq>(mask = ([01X]+))<seq>");
    const std::regex assign_re(R"<seq>(mem\[(\d+)\] = (\d+))<seq>");
    while (inputFile && callable([&](auto&& outer) {
        bool res = true;
        getline(inputFile, line);
        while (!line.empty() && res) {
            std::smatch match;
            const bool matched = std::regex_match(line, match, mask_re);
            assert(matched);
            res = outer(match[1].str(), [&](auto&& inner) {
                while (getline(inputFile, line) &&
                                    std::regex_match(line, match, assign_re)) {
                    inner(std::stoll(match[1].str()),
                          std::stoll(match[2].str())); // ideally should use
                                                       // strtoull
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
