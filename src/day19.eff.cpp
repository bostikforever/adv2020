#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

namespace hash_tuple { // http://coliru.stacked-crooked.com/a/fcc23b6af1db5a99
template <typename TT>
struct hash {
  size_t operator()(TT const& tt) const
  {
      return std::hash<TT>()(tt);
  }
};

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
  seed ^= hash_tuple::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace details {
  struct HashValueImpl {
    template <typename Tuple, std::size_t I = 0>
    static void hash(std::size_t& seed, Tuple const& tuple)
    {
      hash_combine(seed, std::get<I>(tuple));
      constexpr auto Size = std::tuple_size_v<Tuple>;
      if constexpr(I + 1 < Size) {
        details::HashValueImpl::hash<Tuple, I + 1>(seed, tuple);
      }
    }
  };
}

template <typename ... TT>
struct hash<std::tuple<TT...>> {
  std::size_t operator()(std::tuple<TT...> const& tt) const
  {
    std::size_t seed = 0;
    details::HashValueImpl::hash(seed, tt);
    return seed;
  }
};
}

namespace detail {

class Parser {
  public:
    virtual std::string_view name() const = 0;
    virtual bool parse(std::string_view compare) const = 0;
    // virtual ~Parser() = default;
};

class Constant final : public Parser {
    std::string constant;

  public:
    Constant(std::string_view constant)
    : constant(constant)
    {
    }

    virtual std::string_view name() const
    {
        return constant;
    }

    virtual bool parse (std::string_view compare) const
    {
        return compare == constant;
    }
};

class Alt final : public Parser {
    static constexpr char separator[] = " | ";

    std::string parser_name;
    Parser *lhs;
    Parser *rhs;

  public:
    Alt(Parser *lhs, Parser *rhs)
    : parser_name(std::size(lhs->name()) +
                  std::size(separator) +
                  std::size(rhs->name()),
                  0)
    , lhs(lhs)
    , rhs(rhs)
    {
        parser_name.assign(lhs->name());
        parser_name.append(separator);
        parser_name.append(rhs->name());
    }

    virtual std::string_view name() const
    {
        return parser_name;
    }

    virtual bool parse(std::string_view compare) const
    {
        return lhs->parse(compare) || rhs->parse(compare);
    }
};

class Concat final : public Parser {
    static constexpr char separator[] = " ";

    std::string parser_name;
    Parser *lhs;
    Parser *rhs;

  public:
    Concat(Parser *lhs, Parser *rhs)
    : parser_name(std::size(lhs->name()) +
                  std::size(separator) +
                  std::size(rhs->name()),
                  0)
    , lhs(lhs)
    , rhs(rhs)
    {
        parser_name.assign(lhs->name());
        parser_name.append(separator);
        parser_name.append(rhs->name());
    }

    virtual std::string_view name() const
    {
        return parser_name;
    }

    virtual bool parse(std::string_view compare) const
    {
        auto it = std::cbegin(compare);
        for (; it != std::cend(compare); ++it)
        {
            const auto res = lhs->parse(std::string_view(
                                                 std::cbegin(compare),
                                                 it - std::cbegin(compare))) &&
                             rhs->parse(std::string_view(
                                                 it,
                                                 std::cend(compare) - it));
            if (res) {
                return res;
            }
        }
        return lhs->parse(std::string_view(std::cbegin(compare),
                                           it - std::cbegin(compare))) &&
               rhs->parse(std::string_view(it,
                                           std::cend(compare) - it));
    }
};

// class Reference final : public Parser {

//     using NameOrParser = std::variant<std::string, Parser *>;
//     NameOrParser nameOrParser;

//   public:
//     Reference(std::string_view name)
//     : nameOrParser(std::string(name))
//     {
//     }

//     Reference(Parser *parser)
//     : nameOrParser(parser)
//     {
//     }

//     void setParser(Parser *parser) {
//         nameOrParser.emplace<Parser *>(parser);
//     }

//     virtual std::string_view name() const
//     {
//         return std::visit([](auto&& arg) {
//           using T = std::decay_t<decltype(arg)>;
//           if constexpr(std::is_same_v<T, Parser *>) {
//             return arg->name();
//           }
//           else if constexpr(std::is_same_v<T, std::string>) {
//             return std::string_view(arg.data(), arg.length());
//           }
//         },
//         nameOrParser);
//     }

//     virtual bool parse(std::string_view compare) const
//     {
//         // std::cout << name() << ", " << compare << '\n';
//         assert(std::holds_alternative<Parser *>(nameOrParser));
//         return std::get<Parser *>(nameOrParser)->parse(compare);
//     }

// };

class Reference final : public Parser {

    std::string parser_name;
    Parser *parser;

  public:
    Reference(std::string_view name)
    : parser_name(name)
    , parser(nullptr)
    {
    }

    Reference(Parser *parser)
    : parser_name(parser->name())
    , parser(parser)
    {
    }

    void setParser(Parser *par) {
        parser = par;
    }

    virtual std::string_view name() const
    {
        return parser_name;
    }

    virtual bool parse(std::string_view compare) const
    {
        // std::cout << name() << ", " << compare << '\n';
        assert(parser);
        return parser->parse(compare);
    }

};

class Cache {

    struct KeyLess {

      template <typename K1, typename K2>
      bool operator()(K1&& key1, K2&& key2) const
      {
        return std::less<>()(key1, key2);
      }

    };

    using KeyType = std::tuple<const void *, std::string_view>;
    // std::map<KeyType, const bool, KeyLess> cache;
    std::unordered_map<KeyType,
                       const bool,
                       hash_tuple::hash<KeyType>> cache;

  public:
    template <typename F>
    bool getOrSet(const void *parser, std::string_view compare, F&& compute)
    {
        // constexpr auto CACHE_THRESHOLD = 2;
        // if (compare.size() < CACHE_THRESHOLD)
        // {
        //     return compute();
        // }

        auto&& args = std::make_tuple(parser, compare);
        // const auto args = std::make_tuple(parser, std::string(compare));
        const auto it = cache.find(args);
        if (it != std::end(cache)) {
            return it->second;
        }
        // const auto stringsIt = strings.emplace(compare).first;
        // args.second = std::string_view(stringsIt->data(),
        //                                stringsIt->size());
        return cache.emplace(args, compute()).first->second;
    }

    void clear()
    {
        cache.clear();
    }
};

template <typename T>
struct deferred {
    static constexpr bool value = false;
};

class CachedParser final : public Parser {
    using ParserUnion = std::variant<Constant, Alt, Concat>;
    Cache&  cache;
    ParserUnion parser;

    template <typename F>
    auto visit(F&& func) const
    {
        return std::visit([func](auto&& par)
        {
            using T = std::decay_t<decltype(par)>;
            if constexpr(std::is_base_of_v<Parser, T>) {
                return std::invoke(func, par);
            }
            else {
                static_assert(deferred<T>::value, "Unexpected type");
            }
        },
        parser);
    }

  public:
    template <typename ParserType>
    CachedParser(Cache *cache, ParserType&& parser)
    : cache(*cache)
    , parser(std::move(parser))
    {
    }

    virtual std::string_view name() const
    {
        return visit(&Parser::name);
    }

    virtual bool parse(std::string_view compare) const
    {
        // std::cout << name() << ": " << compare << '\n';
        return cache.getOrSet(this,
                              compare,
                              [compare, this]
                              {
                                return visit([compare](const Parser& par)
                                {
                                    return par.parse(compare);
                                });
                              });
    }
};

using ParserUnion = std::variant<Constant,
                                 Alt,
                                 Concat,
                                 CachedParser,
                                 Reference>;
using ParserStore = std::deque<ParserUnion>;
using ParserCache = std::unordered_map <std::string, Parser *>;

class ParserUtil {
  public:
    enum Op { e_Concat, e_Alt };

  private:
    static bool place(ParserCache      *cache,
                      std::string_view  ruleName,
                      Parser           *par)
    {
        return cache->emplace(ruleName, par).second;
    }

    static bool place(ParserCache *cache,
                      Parser      *par)
    {
        return place(cache, par->name(), par);
    }

    template <typename ParserType>
    static Parser *place(ParserCache *cache,
                         ParserStore *store,
                         ParserType&  parser)
    {
        const std::string name(parser.name());
        const auto cacheIt = cache->find(name);
        if (cacheIt != std::end(*cache)) {
            return cacheIt->second;
        }
        auto& storeRef = store->emplace_back(parser);
        Parser *ret = &std::get<ParserType>(storeRef);
        ParserUtil::place(cache, ret);
        return ret;
    }

  public:
    static Parser *makeParser(Cache       *cache,
                              ParserCache *parserCache,
                              ParserStore *store,
                              Op           op,
                              Parser      *lhs,
                              Parser      *rhs)
    {
        switch (op) {
          case e_Concat: {
            // CachedParser parser(cache, Concat(lhs, rhs));
            Concat parser(lhs, rhs);
            return ParserUtil::place(parserCache, store, parser);
          }
          case e_Alt: {
            CachedParser parser(cache, Alt(lhs, rhs));
            // Alt parser(lhs, rhs);
            return ParserUtil::place(parserCache, store, parser);
          }
        }
        assert(false);
    }

    static Parser *makeParser(Cache            *cache,
                              ParserCache      *parserCache,
                              ParserStore      *store,
                              std::string_view  ruleName,
                              Op                op,
                              Parser           *lhs,
                              Parser           *rhs)
    {
        Parser *res = ParserUtil::makeParser(cache,
                                             parserCache,
                                             store,
                                             op,
                                             lhs,
                                             rhs);
        ParserUtil::place(parserCache, ruleName, res);
        return res;
    }

    static Parser *makeParser(ParserCache      *cache,
                              ParserStore      *store,
                              std::string_view  constant)
    {
        Constant parser(constant);
        return ParserUtil::place(cache, store, parser);
    }

    static Parser *makeParser(ParserCache      *cache,
                              ParserStore      *store,
                              std::string_view  ruleName,
                              std::string_view  constant)
    {
        Parser *res = ParserUtil::makeParser(cache,
                                             store,
                                             constant);
        ParserUtil::place(cache, ruleName, res);
        return res;
    }

    static Parser *getOrPlaceholder(ParserCache      *cache,
                                    ParserStore      *store,
                                    std::string_view  ruleName)
    {
        const auto it = cache->find(std::string(ruleName));
        if (it != cache->end()) {
            return it->second;
        }
        auto& ref = store->emplace_back(Reference(ruleName));
        auto *ret = &std::get<Reference>(ref);
        cache->emplace(ruleName, ret);
        return ret;
    }

    static void setOverwrite(ParserCache      *cache,
                             ParserStore      *store,
                             std::string_view  ruleName,
                             Parser           *parser)
    {
        const auto it = cache->find(std::string(ruleName));
        assert(it == cache->end() ||
                             dynamic_cast<Reference *>(it->second) != nullptr);
        if (it == cache->end()) {
          auto& ref = store->emplace_back(Reference(parser));
          auto *ret = &std::get<Reference>(ref);
          cache->emplace(ruleName, ret);
          return;
        }

        auto* referenceParser = static_cast<Reference *>(it->second);
        referenceParser->setParser(parser);
    }
};

}

namespace {

using namespace detail;

using Rule = std::vector<std::string>;

void buildParser(Cache       *parsingCache,
                 ParserCache *parserCache,
                 ParserStore *store,
                 int          ruleNum,
                 const Rule&  rule)
{
  std::stack<ParserUtil::Op> opStack;
  std::stack<Parser *> argStack;

  const auto apply = [&](ParserUtil::Op op, Parser *lhs, Parser *rhs)
  {
    return ParserUtil::makeParser(parsingCache,
                                  parserCache,
                                  store,
                                  op,
                                  lhs,
                                  rhs);
  };

  const auto consume = [&apply](auto& argStack, auto& opStack) {
    auto accum = argStack.top();
    argStack.pop();
    while (!argStack.empty()) {
      assert(!opStack.empty());
      assert(opStack.top() == ParserUtil::e_Alt);
      accum = apply(opStack.top(), argStack.top(), accum);
      opStack.pop();
      argStack.pop();
    }
    return accum;
  };

  for (const auto& token: rule) {
    Parser *arg;
    if (token.size() == 1) {
      switch(token[0]) {
        case ' ':
          opStack.push(ParserUtil::e_Concat);
          assert(!argStack.empty());
          continue;
        case '|':
          opStack.push(ParserUtil::e_Alt);
          assert(!argStack.empty());
          continue;
        default: // number
          arg = ParserUtil::getOrPlaceholder(parserCache, store, token);
      }
    }
    else {
      if (token[0] == '"') { // string literal
        assert(token[token.length() - 1] == '"');
        std::string_view constant(std::data(token) + 1,
                                  std::size(token) - 2);
        arg = ParserUtil::makeParser(parserCache,
                                     store,
                                     constant);
      }
      else { // number
        arg = ParserUtil::getOrPlaceholder(parserCache, store, token);
      }
    }

    if (!opStack.empty() && opStack.top() == ParserUtil::e_Concat) {
                                                                 // can combine
      assert(!argStack.empty());
      const ParserUtil::Op op = opStack.top();
      apply(op, argStack.top(), arg);
      arg = ParserUtil::makeParser(parsingCache,
                                   parserCache,
                                   store,
                                   op,
                                   argStack.top(),
                                   arg);
      opStack.pop();
      argStack.pop();
    }
    argStack.emplace(arg);
  }
  Parser *res = consume(argStack, opStack);
  ParserUtil::setOverwrite(parserCache,
                           store,
                           std::to_string(ruleNum),
                           res);
  assert(argStack.empty());
  assert(opStack.empty());
}

template<class... Ts> struct overloaded :
                                        Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class Solution {

    uint64_t solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        Cache parsingCache;
        ParserCache parserCache;
        ParserStore store;

        const auto ret = loop(overloaded {
          // build parser
          [&](int ruleNum, auto&& inner) {
            // std::cout << ruleNum << " ";
            Rule rule;
            inner([&](const std::string& token) {
              rule.emplace_back(token);
              // std::cout << token << " ";
            });
            buildParser(&parsingCache,
                        &parserCache,
                        &store,
                        ruleNum,
                        rule);
            // std::cout << '\n';
            return true;
          },
          // parse input
          [&](auto&& inner) {
            // std::cout << "====== break ======" << '\n';
            Parser& parser = *parserCache.at("0");
            parserCache.clear();
            inner([&](const std::string& line) {
              // return false;
              parsingCache.clear();
              const auto valid = parser.parse(line);
              // std::cout << line << " " << valid << '\n';
              solution += valid;
            });
            return true;
          }
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
    std::vector<uint16_t> lineNumbers;

    while (inputFile && callable([&](auto&& outer) {
      bool res = true;
      while (getline(inputFile, line) && !line.empty() && res) {
        const std::regex
              token_re(R"<kk>(((\d+): )|((\d+)|( (\|) )|( )|(\"[ab]\")))<kk>");
        std::sregex_token_iterator it(line.begin(),
                                      line.end(),
                                      token_re,
                                      {2, 4, 6, 7, 8});
        res = outer(
          std::stoi(*(it++)),
          [&](auto&& inner) {
            while (it != std::sregex_token_iterator()) {
              if (it->length() == 0) {
                ++it;
                continue;
              }
              inner(*(it++));
            }
        });
      }
      res = outer([&](auto&& inner) {
        while (getline(inputFile, line) && !line.empty() && res) {
          inner(line);
        }
      });
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
