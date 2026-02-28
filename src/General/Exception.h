#ifndef EXCEPTION
#define EXCEPTION

#include <string>
#include <format>

namespace Pgn {
    class Exception{
    private:
        int code_;
        std::string text_;

    public:
        
        template <typename... Args>
        Exception(int code, const std::format_string<Args...>& message, Args&&... args)
            : code_(code), 
              text_(std::format(message, std::forward<Args>(args)...)) 
        {}

        const char* what() const noexcept {
            return text_.c_str();
        }

        int code() const { return code_; }
    };

    constexpr const char* FILE_EXCEPTION = "Unable to open file <{}>";
}

#endif