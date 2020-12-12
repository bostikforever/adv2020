#include <cctype>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

namespace {

/*
    byr (Birth Year) - four digits; at least 1920 and at most 2002.
    iyr (Issue Year) - four digits; at least 2010 and at most 2020.
    eyr (Expiration Year) - four digits; at least 2020 and at most 2030.
    hgt (Height) - a number followed by either cm or in:
        If cm, the number must be at least 150 and at most 193.
        If in, the number must be at least 59 and at most 76.
    hcl (Hair Color) - a # followed by exactly six characters 0-9 or a-f.
    ecl (Eye Color) - exactly one of: amb blu brn gry grn hzl oth.
    pid (Passport ID) - a nine-digit number, including leading zeroes.
    cid (Country ID) - ignored, missing or not.
*/

bool byr_validate(const std::string& value)
{
    std::istringstream is(value);
    int year;
    is >> year;
    return 1920 <= year && year <= 2002;
}

bool iyr_validate(const std::string& value)
{
    std::istringstream is(value);
    int year;
    is >> year;
    return 2010 <= year && year <= 2020;
}

bool eyr_validate(const std::string& value)
{
    std::istringstream is(value);
    int year;
    is >> year;
    return 2020 <= year && year <= 2030;
}

bool hgt_validate(const std::string& value)
{
    std::istringstream is(value);
    int height;
    std::string unit;
    is >> height;
    is >> unit;
    if (unit == "cm") {
        return 150 <= height && height <= 193;
    }
    if (unit == "in") {
        return 59 <= height && height <= 76;
    }
    return false;
}

bool hcl_validate(const std::string& value)
{
    std::istringstream is(value);
    char beg;
    is >> beg;
    if (beg != '#') return false;
    std::string rest;
    is >> rest;
    return std::all_of(rest.begin(),
                       rest.end(),
                       [](char ch) {
                          const char a_f[] = "abcdef";
                          return std::isdigit(ch) or
                                 std::find(std::begin(a_f), std::end(a_f), ch)
                                                              != std::end(a_f);
                    });

}

bool ecl_validate(const std::string& value)
{
    constexpr const char * colors[] =
                         { "amb",  "blu",  "brn", "gry", "grn", "hzl", "oth" };
    return std::count(std::begin(colors), std::end(colors), value) > 0;
}

bool pid_validate(const std::string& value)
{
    return value.size() == 9 &&
                         std::all_of(value.begin(),
                                     value.end(),
                                     static_cast<int(*)(int)>(std::isdigit));
}

bool cid_validate(const std::string& value)
{
    return true;
}

class Solution {

    using Validator = bool(const std::string& );
    static const std::unordered_map<std::string, Validator *> validators;

    static const std::string exempted;

    int solution = 0;
    int calls = 0;

    bool solve(const std::unordered_map<std::string, std::string>& values)
    {
        ++calls;
        // for (const auto it: values) {
        //     std::cout <<it.first << ": " << it.second << '\n';
        // }
        // std::cout <<'\n';
        solution += std::all_of(validators.begin(),
                                validators.end(),
                                [&values](
                                     decltype(*validators.cbegin()) keyValue) {
                                    auto it = values.find(keyValue.first);
                                    if (it != values.end()) {
                                       // std::cout << keyValue.first <<": "
                                       //           << it->second<<'\n';
                                       return keyValue.second(it->second);
                                    }
                                    return keyValue.second("");
                                });
        // std::cout <<'\n';
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


bool trivialValidate(const std::string& val) {
    return !val.empty();
}

const std::string Solution::exempted = "cid";
const std::unordered_map<std::string, Solution::Validator *>
                                                           Solution::validators
                                                 { { "byr", &byr_validate },
                                                   { "iyr", &iyr_validate },
                                                   { "eyr", &eyr_validate },
                                                   { "hgt", &hgt_validate },
                                                   { "hcl", &hcl_validate },
                                                   { "ecl", &ecl_validate },
                                                   { "pid", &pid_validate },
                                                   { "cid", &cid_validate } };

bool readValues(std::unordered_map<std::string, std::string> *values,
                const std::string&                            line)
{
    auto begin = line.cbegin();
    while (begin != line.cend()) {
        auto end = std::find(begin, line.cend(), ':');
        const std::string key(begin, end);

        begin = std::next(end);
        end = std::find(end, line.cend(), ' ');
        const std::string value(begin, end);
        values->emplace(key, value);
        // assert(token.first->size() == 3)
        // std::cout << key << ' ' << value << ' ';
        // discard value associated with key
        begin = end;
        if (begin != line.cend()) {
            ++begin;
        }
    }
    // std::cout << '\n';
    return true; // always a success?
}

template <typename CALLABLE>
void readLine(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;
    std::unordered_map<std::string, std::string> values;
    while (getline(inputFile, line)) {
        if (line.empty()) {
            callable(values);
            values.clear();
            continue;
        }
        readValues(&values, line);
    }
    if(!values.empty()) {
        callable(values);
    }
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
