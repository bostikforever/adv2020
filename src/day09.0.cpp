#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <array>
#include <cstdint>
#include <type_traits>

namespace {

template <typename T, std::size_t N>
class Buffer {
    std::size_t next = 0;
    std::size_t length = 0;
    std::array<T, N> buffer;

  public:
    std::size_t size() const
    {
        return length;
    }

    template <typename Arg, typename FUNC>
    std::size_t push_back(Arg&& arg, FUNC&& func)
    {
        const auto id = next;

        if (length == N) {
            func(std::move(buffer[next]), id);
        }
        else {
            ++length;
        }

        buffer[next] = std::move(arg);
        next = (next + 1) % N;

        return id;
    }

    template<typename FUNC>
    void forEach(FUNC&& func) const
    {
        const std::size_t start = length == N ? next : 0;
        for (std::size_t i = 0; i < length; ++i) {
            auto id = (start + i) % N;
            func(buffer[id], id);
        }
    }
};

class Solution {

    static constexpr std::size_t lookback = 25;

    uint64_t solution = 0;
    std::size_t calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        Buffer<uint64_t, lookback> buffer;
        std::unordered_map<std::size_t, std::array<uint64_t, lookback>>
                                                                     posToSums;
        std::unordered_map<uint64_t, std::size_t> sumsCount;

        const auto ret = loop([&](uint64_t arg) {
            if (buffer.size() == lookback && !sumsCount.count(arg)) {
                solution = arg;
                return false;
            }
            const auto newId = buffer.push_back(arg, [&](auto old, auto id) {
                const auto it = posToSums.find(id);
                for (std::size_t i = 0; i < it->second.size(); ++i) {
                    if (i == id) {
                        continue;
                    }
                    auto sum = it->second[i];
                    auto countIt = sumsCount.find(sum);
                    if (countIt == sumsCount.end()) {
                        continue;
                    }
                    auto& count = --countIt->second;
                    if (count == 0) {
                        sumsCount.erase(countIt);
                    }
                }
                posToSums.erase(it);
            });
            buffer.forEach([&](auto value, auto id){
                if (newId != id && arg != value) {
                    auto sum = arg + value;
                    posToSums[id][newId] = sum;
                    ++sumsCount[sum];
                }
            });
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

template <typename CALLABLE>
void readBatches(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    uint64_t arg;

    const auto ret = callable([&](auto&& body) {
        bool res;
        while (inputFile >> arg && (res = body(arg)))
            ;
        return res;
    });
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
