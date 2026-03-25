#ifndef EXCEPTION
#define EXCEPTION

#include <string>
#include <format>

namespace Pgn {
    /// @brief Custom simple exception class
    class Exception{
    private:
        int code_;
        std::string text_;

    public:
        
        template <typename... Args>
        /// @brief Constructor which takes message and set of arguments to put inside 
        Exception(int code, const std::format_string<Args...>& message, Args&&... args)
            : code_(code), 
              text_(std::format(message, std::forward<Args>(args)...)) 
        {}

        /// @return Returns message of the exception
        const char* what() const noexcept {
            return text_.c_str();
        }

        /// @return Returns code of the exception
        int code() const { return code_; }
    };

    /// @brief Named constant for file exception
    constexpr const char* FILE_EXCEPTION = "Unable to open file <{}>";
}

#endif