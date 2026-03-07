#ifndef CLI_
#define CLI_

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Pgn {
    namespace Cli {
        constexpr size_t ARGS_VEC_RESERVE = 8;
        constexpr size_t ARG_STR_RESERVE = 32;
        
        struct ParsedCommand {
            std::string name;
            std::unordered_map<std::string, std::string> flags;
            std::vector<std::string> args;
        };

        class Application{
        private:
            bool running_ = false;
            
            std::string trim_(std::string_view s) const;
            std::vector<std::string> split_args_(std::string_view line) const;
            ParsedCommand parse_command_line_(const std::vector<std::string>& args) const;
            
            void print_prompt_() const;
            void handle_command_(const ParsedCommand& cmd);
            
            void cmd_quit_();
            void cmd_help_(const ParsedCommand& cmd);

        public:
            Application() = default;
            ~Application() = default;

            void run();
        };
    }
}

#endif