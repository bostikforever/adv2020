#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>

namespace {

class Solution {

    std::string solution;
    int calls = 0;

    constexpr static int PICKUP_LENGTH = 3;
    bool solve(std::string input, int rounds)
    {
      const auto max_el = *std::max_element(input.begin(), input.end());
      const auto min_el = *std::min_element(input.begin(), input.end());


      const auto dec = [min_el, max_el](auto arg) {
        if (arg == min_el) {
          return max_el;
        }
        return --arg;
      };

      int curr = 0;
      while (rounds --> 0) {
        auto currIt = std::begin(input) + curr;
        const auto rightCurr = std::end(input) - currIt - 1;
        if (rightCurr < PICKUP_LENGTH) {
          const auto fixLength = PICKUP_LENGTH - rightCurr;
          std::rotate(std::begin(input),
                      std::begin(input) + fixLength,
                      std::end(input));
          currIt -= fixLength;
          curr -= fixLength;
        }
        const auto pickupRange = std::make_pair(currIt + 1,
                                                currIt + PICKUP_LENGTH + 1);
        auto destEl = dec(*currIt);
        while (std::find(pickupRange.first, pickupRange.second, destEl) !=
                                                          pickupRange.second) {
          destEl = dec(destEl);
        }
        const auto destinationIt = std::find(std::begin(input),
                                             std::end(input),
                                             destEl);
        assert(destinationIt != std::end(input));
        if (destinationIt >= pickupRange.second) {
          std::rotate(pickupRange.first,
                      pickupRange.second,
                      std::next(destinationIt));
        }
        else {
          std::rotate(std::next(destinationIt),
                      pickupRange.first,
                      pickupRange.second);
          curr += PICKUP_LENGTH;
        }
        curr = (curr + 1) % std::size(input);
      }

      const auto oneIt = std::find(std::begin(input), std::end(input), '1');
      std::rotate(std::begin(input), oneIt, std::end(input));

      solution.assign(std::begin(input) + 1, std::end(input));
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
    solution("389125467", 100);
    // solution("974618352", 100);
    std::cout << solution.output() << '\n';
    return 0;
}