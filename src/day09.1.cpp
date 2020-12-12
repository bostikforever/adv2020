#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace {

template <typename T, typename... Less>
class SlidingWindowOrder
{
    template <typename K, typename DUMMY>
    using expand = K;
    using Window = std::deque<T>;

    Window m_window;
    std::tuple<Less...> m_less;
    std::tuple<expand<Window, Less>...> m_seq_ord;

    template <typename FUNC, std::size_t...I>
    void forEachImpl(FUNC&& func, std::index_sequence<I...>)
    {
        (void) std::initializer_list<int> {
                    (func(std::get<I>(m_seq_ord), std::get<I>(m_less)), 0)...};
    }

    template <typename FUNC>
    void forEach(FUNC&& func)
    {
        forEachImpl(std::forward<FUNC>(func),
                    std::make_index_sequence<
                                    std::tuple_size_v<std::tuple<Less...>>>{});
    }

  public:
    T pop_front()
    {
        T ret = m_window.front();
        forEach([=](Window& orderWindow, const auto& less) {
            assert(!orderWindow.empty());
            if (!less(orderWindow.front(), ret) &&
                !less(ret, orderWindow.front())) {
                orderWindow.pop_front();
            }
        });
        m_window.pop_front();
        return ret;
    }

    template <typename Arg>
    void push_back(Arg&& arg)
    {
        forEach([=](Window& orderWindow, const auto& less) {
            while(!orderWindow.empty() &&
                  less(arg, orderWindow.back())) {
                orderWindow.pop_back();
            }
            orderWindow.push_back(arg);
        });
        m_window.emplace_back(arg);
    }

    std::size_t size() const
    {
        return m_window.size();
    }

    template<std::size_t... I>
    auto getExtremumsImpl(std::index_sequence<I...>) const
    {
        assert(!m_window.empty());
        return std::make_tuple(std::get<I>(m_seq_ord).front()...);
    }

    auto getExtremums() const
    {
        assert(!m_window.empty());
        return getExtremumsImpl(std::make_index_sequence<
                                    std::tuple_size_v<std::tuple<Less...>>>{});

    }
};

class Solution {

    uint64_t solution = 0;
    std::size_t calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop, uint64_t target)
    {
        ++calls;
        uint64_t sum = 0;
        SlidingWindowOrder<uint64_t, std::less<>, std::greater<>> window;
        const auto ret = loop([&](uint64_t arg) {
            while (sum + arg > target) {
                const auto popped = window.pop_front();
                sum -= popped;
            }

            if (sum + arg == target && window.size() >= 1) {
                const auto extremums = window.getExtremums();
                const auto min_val = std::min(std::get<0>(extremums), arg);
                const auto max_val = std::max(std::get<1>(extremums), arg);
                solution = max_val + min_val;
                return false; // terminate loop early
            }
            window.push_back(arg);
            sum += arg;
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
    },
    /* put solution from part 0 here */);
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
