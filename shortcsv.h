#ifndef SHORTCSV_H
#define SHORTCSV_H

#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <optional>
#include <variant>
#include <regex>
#include <cstdint>
#include <cctype>

namespace shortcsv
{
  enum class Field : uint8_t
  {
    Undefined = 0,
    Null,
    Boolean,
    String,
    Integer,
    Float,
  };

  struct field_t
  {
    using value_t = std::variant<std::string, long double, intmax_t, bool>;

    Field   type;
    value_t data;

    constexpr bool&         toBool  (void) noexcept { return std::get<bool       >(data); }
    constexpr intmax_t&     toNumber(void) noexcept { return std::get<intmax_t   >(data); }
    constexpr long double&  toFloat (void) noexcept { return std::get<long double>(data); }
    inline    std::string&  toString(void) noexcept { return std::get<std::string>(data); }
  };

  constexpr auto string_regex   = "^\"(((\\.)|([^\"])))*\"|'(((\\.)|([^'])))*'";
  constexpr auto integer_regex  = "^(0x[[:xdigit:]]+)|([+-]?[[:digit:]]+)";
  constexpr auto float_regex    = "^[+-]?([[:digit:]]+[.][[:digit:]]*|[.][[:digit:]]+)([eE][+-]?[[:digit:]]+)?";

  inline std::optional<std::string> RegexMatch(const std::string& expression, std::string::const_iterator& pos, std::string::const_iterator end)
  {
    std::smatch match;
    if(std::regex_search(pos, end, match, std::regex(expression, std::regex_constants::extended)))
    {
      std::advance(pos, match[0].length());
      return match[0].str();
    }
    return {};
  }

  template<char deliminator>
  inline field_t ParseField(std::string::const_iterator& pos, std::string::const_iterator end)
  {
    while(std::isspace(*pos) || *pos == deliminator)
    {
      std::advance(pos, 1);
      if(*pos == deliminator)
        return { Field::Null, {} };
    }
    if(auto ostr = RegexMatch(string_regex, pos, end); ostr)
      return { Field::String, std::string(std::next(std::begin(*ostr)), std::prev(std::end(*ostr))) };
    if(auto ostr = RegexMatch(float_regex, pos, end); ostr)
      return { Field::Float, std::stold(*ostr) };
    if(auto ostr = RegexMatch(integer_regex, pos, end); ostr)
      return { Field::Integer, std::stoll(*ostr) };
    return { Field::Undefined, {} };
  }

  template<char deliminator>
  std::vector<field_t> ParseLine(const std::string& line)
  {
    std::vector<field_t> values;
    auto pos = std::begin(line);
    auto end = std::end(line);
    while(pos != end)
    {
      values.emplace_back(ParseField<deliminator>(pos, end));
      while(pos != end && std::isspace(*pos))
        std::advance(pos, 1);
    }
    return values;
  }

  template<char deliminator = ','>
  std::vector<std::vector<field_t>> Parse(const std::string& records)
  {
    std::vector<std::vector<field_t>> data;
    std::istringstream input;
    input.str(records);
    for(std::string line; std::getline(input, line); )
      data.emplace_back(ParseLine<deliminator>(line));
    return data;
  }
}

#endif // SHORTCSV_H
