#ifndef TOKENS_
#define TOKENS_

#include <string>
#include <string_view>
namespace Pgn::Tokens{
    constexpr std::string_view EVENT = "Event";
    constexpr std::string_view SITE = "Site";
    constexpr std::string_view DATE = "Date";
    constexpr std::string_view ROUND = "Round";
    constexpr std::string_view WHITE = "White";
    constexpr std::string_view BLACK = "Black";
    constexpr std::string_view RESULT = "Result";
    constexpr std::string_view WHITE_ELO = "WhiteElo";
    constexpr std::string_view BLACK_ELO = "BlackElo";
    constexpr std::string_view ECO = "ECO";
    constexpr std::string_view PLY_COUNT = "PlyCount";
    constexpr std::string_view OPENING = "Opening";
    constexpr std::string_view TIME_CONTROL = "TimeControl";

    constexpr char COMMENT_OPEN = '{';
    constexpr char COMMENT_CLOSE = '}';
    constexpr char VARIATION_OPEN = '(';
    constexpr char VARIATION_CLOSE = ')';

    constexpr char WHITESPACE = ' ';
    constexpr char VALUE_DELIM = '"';

    constexpr char TAG_OPEN = '[';
    constexpr char TAG_CLOSE = ']';

    constexpr char FLAG_PREFIX = '-';
    constexpr char EQUALS = '=';

    inline bool is_termination(const std::string& token) {
        return (token == "1-0" || token == "0-1" || token == "1/2-1/2" || token == "*"); 
    }
}

#endif