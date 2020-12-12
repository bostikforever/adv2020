#include <algorithm>
#include <cassert>
#include <climits>
#include <fstream>
#include <iostream>
#include <string_view>

namespace {

class Solution {

    static constexpr int row_length = 7;
    static constexpr int col_length = 3;
    static constexpr char row_hi = 'B';
    static constexpr char row_lo = 'F';
    static constexpr char col_hi = 'R';
    static constexpr char col_lo = 'L';


    int min_id = INT_MAX;
    int max_id = 0;
    int agg = 0;

    int decode(const std::string_view str, char lo, char hi)
    {
        int ret = 0;
        for (auto ch: str) {
            ret <<= 1;
            const bool bit = [&] {
                if (ch == lo) {
                    return 0;
                }
                else {
                    assert(ch == hi);
                    return 1;
                }
            }();
            ret += bit;
        }

        return ret;
    }

    int computeID(int row, int col)
    {
        return row * 8 + col;
    }

    bool solve(const std::string& line)
    {

        const auto col_begin = &*line.cbegin() + row_length;
        const int row = decode({&*line.cbegin(), row_length}, row_lo, row_hi);
        const int col = decode({col_begin, col_length}, col_lo, col_hi);

        // std::cout << "row: " << row << " col: " << col << '\n';

        const int ID = computeID(row, col);
        agg ^= ID;
        min_id = std::min(min_id, ID);
        max_id = std::max(max_id, ID);

        return true;
    }

  public:
    template <typename ...Args>
    bool operator()(Args... args)
    {
        return solve(args...);
    }

    int output() const
    {
        /*
            agg contains all the bits from the range (apart from the missing
            value) xor'ed together.

            To find the missing value, we need to know what the xor'ing of the
            complete range would be:

            Given a range, for each bit position in the min value of the range,
            we know the number of values *till* the next power of two (and
            hence the number of times the current value of the bit will be
            applied).

            Similarly, for the max value we know the number of times the
            current value has been applied *from* the last power of two.

            Both values are related to the value of the lower bits (from the
            current bit). Note that since we are ultimately interested in just
            xor'ing stuff together, actually only the polarity of the lowest
            bit matters all the time and not the actual number of applications.

            With these two values, we can compute what the actual xor'ing of
            the full range should be and then with that derive the value of the
            missing number in the range.
        */
        constexpr int all_ones_10bit = 0x3FF; // our IDs are 10 bits long
        static_assert(all_ones_10bit == (1 << (row_length + col_length)) - 1);

        const int min_id_flag = all_ones_10bit & (min_id & 1) * all_ones_10bit;
            // The result of xor'ing all the bits from min_id to the next power
            // of two is determined by the value of the least significant bit
            // in min_id.
        const int max_id_flag = all_ones_10bit & (~max_id & 1) *
                                                                all_ones_10bit;
            // The result of xor'ing all the bits from the last power of two to
            // the max_id is determined by the the inverse of the least
            // significant bit of max_id.

        const int full_range_xor = (min_id & min_id_flag) ^
                                                        (max_id & max_id_flag);
            // For the full range we are interested in the xor'ing:
            // From min_id to the next power of two, from that power of two to
            // the last power of two just before max_id, and from that last
            // power of two till max_id. However we do not need to xor the
            // values between powers of two since they result to zero.
            // This also works in the degenerate case where both min and max
            // both belong to the same power of two range; in that case it
            // might be easier to think of min as being a count from the
            // preceeding power of two  that this is being subtracted from the
            // the corresponding count for max and noting that both addition
            // and subtraction are equivalent under xor arithmetic.

        const int res = full_range_xor ^ agg;
        return res;
    }
};

template <typename CALLABLE>
void readLine(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;
    while (inputFile >> line && callable(line))
        ;
}

}  // close unnamed namespace

int main(int argc, const char* argv[])
{
    assert(argc > 1);
    std::string filename = argv[1];
    Solution solution;
    readLine(filename, solution);
    std::cout << solution.output() << '\n';
    return 0;
}
