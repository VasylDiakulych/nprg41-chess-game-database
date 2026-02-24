#ifndef TOKENS_
#define TOKENS_

#include <string>
namespace Pgn::Tokens{
    constexpr char COMMENT_OPEN = '{';
    constexpr char COMMENT_CLOSE = '}';
    constexpr char VARIATION_OPEN = '(';
    constexpr char VARIATION_CLOSE = ')';

    constexpr char WHITESPACE = ' ';

    constexpr char TAG_OPEN = '[';
    constexpr char TAG_CLOSE = ']';

    inline bool is_termination(const std::string& token) {
        return (token == "1-0" || token == "0-1" || token == "1/2-1/2" || token == "*"); 
    }
}

#endif