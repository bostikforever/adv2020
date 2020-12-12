#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <vector>

namespace {

constexpr auto one_million = 1'000'000;
class Solution {

    uint64_t solution = 0;
    int calls = 0;

    constexpr static int PICKUP_LENGTH = 3;

    bool solve(const std::string& start, int rounds)
    {
      using NumType = uint32_t;
      // std::list<NumType> input(start.size());
      std::list<NumType> input(one_million);

      const auto last = std::transform(std::cbegin(start),
                                       std::cend(start),
                                       std::begin(input),
                                       [](auto val) {
                                         return val - '0';
                                       });
      const auto min_el = *std::min_element(std::begin(input), last);
      std::iota(last, std::end(input), std::size(start) + 1);
      const auto max_el = std::size(input);
      assert(max_el == *std::max_element(last, input.end()));

      std::vector<decltype(input)::const_iterator> tracker(one_million + 1);
      for (auto it = std::cbegin(input); it != std::cend(input); ++it) {
        tracker[*it] = it;
      }

      const auto dec = [min_el, max_el](auto arg) {
        if (arg == min_el) {
          return decltype(arg)(max_el);
        }
        return --arg;
      };

      auto currIt = std::cbegin(input);

      while (rounds --> 0) {
        auto rightIt = currIt;
        for (int i = 0; i < PICKUP_LENGTH; ++i) {
          std::advance(rightIt, 1);
          if (rightIt == std::cend(input)) {
            rightIt = std::cbegin(input);
            input.splice(std::cend(input), input, rightIt);
            assert(std::next(rightIt) == std::cend(input));
          }
        }
        const auto pickupRange = std::make_pair(std::next(currIt),
                                                std::next(rightIt));
        auto destEl = dec(*currIt);
        while (std::find(pickupRange.first, pickupRange.second, destEl) !=
                                                          pickupRange.second) {
          destEl = dec(destEl);
        }
        const auto destinationIt = tracker[destEl];
        input.splice(std::next(destinationIt),
                     input,
                     pickupRange.first,
                     pickupRange.second);
        ++currIt;
        if (currIt == std::cend(input)) {
          currIt = std::cbegin(input);
        }
      }

      const auto oneIt = std::find(std::cbegin(input), std::cend(input), 1);
      if (oneIt != std::cbegin(input)) {
        input.splice(std::cbegin(input),
                     input,
                     oneIt,
                     std::cend(input));
      }
      // std::cout << *std::next(oneIt) << '\n';
      // std::cout << *std::next(oneIt, 2) << '\n';
      // for (auto el: input) {
      //   std::cout << el << " ";
      // }
      // std::cout << '\n';
      solution = *std::next(oneIt);
      solution *= *std::next(oneIt, 2);

      return true;
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

}  // close unnamed namespace

int main(int argc, const char* argv[])
{
    Solution solution;
    // solution("389125467", 10 * one_million);
    solution("974618352", 10 * one_million);
    std::cout << solution.output() << '\n';
    return 0;
}