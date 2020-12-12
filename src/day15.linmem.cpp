#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

namespace {

class Solution {

    static constexpr std::size_t stop = 30000000;
    uint64_t solution = 0;
    std::size_t calls = 0;

    bool solve(std::initializer_list<int> start_seq)
    {
        ++calls;
        std::unique_ptr memPtr = std::make_unique<
                       std::array<std::optional<std::pair<int, int>>, stop>>();
        auto& memory = *memPtr;
        int idx = 0;
        int last_num;
        for (; idx < start_seq.size(); ++idx) {
            const auto it = start_seq.begin() + idx;
            memory[*it].emplace(0, idx);
            last_num = *it;
        }
        while (idx < stop) {
            const auto curr_num = memory[last_num]->first;
            auto& prev = memory[curr_num];
            if (!prev) {
                prev.emplace(0, idx);
            }
            // std::cout << curr_num << '\n';
            *prev = {idx - prev->second, idx};
            ++idx;
            last_num = curr_num;
        }
        solution = last_num;
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
    assert(argc > 1);
    std::string filename = argv[1];
    Solution solution;
    solution(std::initializer_list<int>{0, 3, 6});
    // solution(std::initializer_list<int>{10, 16, 6, 0, 1, 17});
    std::cout << solution.output() << '\n';
    return 0;
}
