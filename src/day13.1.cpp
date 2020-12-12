#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <type_traits>
#include <utility>
#include <bitset>

namespace {

struct Constraint {
    int64_t offset;
    int64_t divisor;
    Constraint(int64_t offset, int64_t divisor)
    : offset(offset)
    , divisor(divisor)
    {}
};

struct EGCD {
    int64_t gcd;
    int64_t coeff_lhs;
    int64_t coeff_rhs;
};

EGCD extended_gcd(int64_t lhs, int64_t rhs) {
    int64_t old_r = lhs, r = rhs;
    int64_t old_s = 1, s = 0;
    int64_t old_t = 0, t = 1;

    while (r != 0) {
        auto quotient = old_r / r;
        std::tie(old_r, r) = std::make_tuple(r, old_r - quotient * r);
        std::tie(old_s, s) = std::make_tuple(s, old_s - quotient * s);
        std::tie(old_t, t) = std::make_tuple(t, old_t - quotient * t);
    }

    EGCD ret;
    ret.gcd = old_r;
    ret.coeff_lhs = old_s;
    ret.coeff_rhs = old_t;
    // std::cout << "gcd: " << ret.gcd
    //           << " coeff_lhs " << ret.coeff_lhs
    //           << " coeff_rhs " << ret.coeff_rhs
    //           << " lhs " << lhs
    //           << " rhs " << rhs
    //           << '\n';
    return ret;
}

class Solution {

    static constexpr char key[] = "shiny gold";

    uint64_t solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::vector<Constraint> constraints;
        const auto ret = loop([&](auto, auto&& inner) {
            // std::cout << departure << ' ';
            int inner_calls = 0;
            inner([&](auto index, auto id) {
                index = (id - index % id) % id;
                constraints.emplace_back(index, id);
                std::cout << index << ' ' << id << ' ';
                ++inner_calls;
            });
            std::cout << '\n';
            // std::cout << inner_calls << '\n';
            return true;
        });

        const auto normalise = [](auto num, auto mod) {
            if (0 <= num && num < mod) {
                return num;
            }
            if (0 > num) {
                auto res = num % mod;
                if (abs(res) < (mod + res)) {
                    return res;
                }
                return mod + res;
            }
            return num % mod;
        };

        const auto mulmod = [](auto lhs, auto rhs, auto mod) {
            assert(mod > 0);
            assert(lhs >= 0 || rhs >= 0);
            const bool neg = lhs < 0 || rhs < 0;
            const auto sign = neg? -1 : 1;
            std::tie(lhs, rhs) = std::minmax(std::abs(lhs), std::abs(rhs));
            decltype(lhs) accum = 0;
            while(rhs > 0) {
                if (rhs % 2  == 1) {
                    accum = (accum + lhs) % mod;
                }
                lhs = (lhs * 2) % mod;
                rhs /= 2;
            }
            return sign * accum;
        };

        const auto mul = [=](auto...num) {
            return [=](auto mod) {
                std::array<std::common_type_t<decltype(num)...>,
                           sizeof...(num)> args{ normalise(num, mod)... };
                // meh, runtime resolve;
                std::sort(std::begin(args), std::end(args));
                // std::cout << "mod: " << mod << ' ';
                // std::cout << "args: ";
                // for (auto arg: args) std::cout << arg << ' ';
                // std::cout << '\n';
                std::remove_reference_t<decltype(args.front())> accum = 1;
                for (int i = 0; i < args.size(); ++i) {
                    auto elem = (i % 2 == 0) ? args[i/2] :
                                                   args[args.size() - 1 - i/2];
                    // std::cout << "acum: " << accum
                    //           << " elem: " << elem << '\n';
                    accum = normalise(mulmod(accum, elem, mod), mod);
                    // std::cout << "accum * elem: " << accum << '\n';
                }
                return accum;
            };
        };

        Constraint reduce = constraints[0];
        for (int i = 1; i < constraints.size(); ++i) {
            for (int j = 0; j < i; ++j) {
                assert(reduce.offset % constraints[j].divisor ==
                                                        constraints[j].offset);
            }
            const auto rhs = constraints[i];
            EGCD egcd = extended_gcd(reduce.divisor,
                                     rhs.divisor);
            const auto mod = reduce.divisor * rhs.divisor;
            reduce.offset = mul(reduce.offset,
                                rhs.divisor,
                                egcd.coeff_rhs)(mod);
            reduce.offset += mul(rhs.offset,
                                 reduce.divisor,
                                 egcd.coeff_lhs)(mod);
            // std::cout << reduce.offset << '\n';
            reduce.offset = normalise(reduce.offset, mod);
            // std::cout << reduce.offset << '\n';
            reduce.offset = (mod  + reduce.offset) % mod;
            // std::cout << reduce.offset << '\n';
            reduce.divisor *= rhs.divisor;
            // std::cout << reduce.divisor << '\n';
        }
        solution = reduce.offset;
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
                const std::regex re(R"<seq>((,)|(\d+))<seq>");
                std::sregex_token_iterator it(line.begin(),
                                              line.end(),
                                              re);
                auto pos = 0;
                for(; it != std::sregex_token_iterator(); ++it) {
                    if (*it == ',') {
                        ++pos;
                        continue;
                    }
                    inner(pos, std::stoi(*it));
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
