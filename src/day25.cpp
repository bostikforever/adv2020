#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

namespace {

class Solution {

    static constexpr auto MOD = 20201227;

    uint64_t solution;
    int calls = 0;

    bool solve(uint64_t key1, uint64_t key2)
    {
      const auto mulmod = [](auto lhs, auto rhs, auto mod) {
         assert(mod > 0);
         assert(lhs >= 0 && rhs >= 0);
         std::tie(lhs, rhs) = std::minmax(lhs % mod, rhs % mod);
         decltype(lhs) accum = 0;
         while(rhs > 0) {
             if (rhs % 2  == 1) {
                 accum = (accum + lhs) % mod;
             }
             lhs = (lhs * 2) % mod;
             rhs /= 2;
         }
         return accum;
      };

      const auto powmod = [&mulmod](auto base, auto power, auto mod) {
         assert(mod > 0);
         assert(base >= 0 && power >= 0);
         decltype(base) accum = 1;
         while(power > 0) {
             if (power % 2  == 1) {
                 accum = mulmod(accum, base, mod);
             }
             base = mulmod(base, base, mod);
             power /= 2;
         }
         return accum;
      };

      const auto logmod = [&powmod](auto num, auto base, auto mod) {
        for (int i = 0; i <= mod; ++i) {
          if (powmod(base, i, mod) == num) {
            return i;
          }
        }
        assert(false);
        return -1;
      };

      const auto n1 = logmod(key1, 7, MOD);
      const auto n2 = logmod(key2, 7, MOD);
      solution = powmod(key2, n1, MOD);
      const auto solutionAlt = powmod(key1, n2, MOD);

      assert(solution == solutionAlt);
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
    solution(18356117, 5909654);
    std::cout << solution.output() << '\n';
    return 0;
}