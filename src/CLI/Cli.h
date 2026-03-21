#ifndef CLI_
#define CLI_

#include "../Parser/Parser.h"
#include "../Writer/Writer.h"
#include "../Database/Database.h"
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Pgn {
    namespace Commands{
        constexpr const char* EXIT_1 = "exit";
        constexpr const char* EXIT_2 = "quit";
        constexpr const char* SEARCH = "search";
        constexpr const char* HELP = "help";
        constexpr const char* LOAD = "load";
        constexpr const char* EXPORT = "export";
        constexpr const char* CLEAR = "clear";
        constexpr const char* STATS = "stats";
    }

    namespace Flags {
        constexpr const char* PLAYER = "player";
        constexpr const char* ELO_MIN = "elo-min";
        constexpr const char* ELO_MAX = "elo-max";
        constexpr const char* EVENT = "event";
        constexpr const char* SITE = "site";
        constexpr const char* ECO = "eco";
        constexpr const char* RESULT = "result";
        constexpr const char* DATE_MIN = "date-min";
        constexpr const char* DATE_MAX = "date-max";
        constexpr const char* COLOR = "color";
        constexpr const char* PLY_MIN = "ply-count-min";
        constexpr const char* PLY_MAX = "ply-count-max";
        constexpr const char* OPENING = "opening";
        constexpr const char* TIME_CONTROL = "time-control";
        constexpr const char* LIMIT = "limit";
        constexpr const char* OFFSET = "offset";
        constexpr const char* VERBOSE = "verbose";
    }

    namespace Help {
        constexpr const char* LOAD = 
            "Usage: load <filename>\n"
            "Load a PGN file into the database.\n"
            "Example: load pgn_files/Carlsen.pgn";
        
        constexpr const char* SEARCH = 
            "Usage: search [options]\n"
            "Search for games matching criteria.\n"
            "Options:\n"
            "  --player <name>     Filter by player name\n"
            "  --color <w|b|any>   Player color (default: any)\n"
            "  --elo-min <n>       Minimum Elo rating\n"
            "  --elo-max <n>       Maximum Elo rating\n"
            "  --event <name>      Filter by event\n"
            "  --site <name>       Filter by site\n"
            "  --eco <code>        Filter by ECO code\n"
            "  --result <r>        Filter by result (1-0, 0-1, 1/2-1/2, *)\n"
            "  --date-min <date>   Minimum date (YYYY.MM.DD)\n"
            "  --date-max <date>   Maximum date (YYYY.MM.DD)\n"
            "  --ply-count-min     Minimum ply-count\n"
            "  --ply-count-max     Maximum ply-count\n"
            "  --opening           Filter by game opening\n"
            "  --time-control      Filter by time-control\n"
            "  --limit <n>         Max results (default: 20)\n"
            "  --offset <n>        Skip first N results\n"
            "  --verbose           Show full PGN output\n"
            "Example: search --player Kasparov --elo-min 2500 --limit 10";
        
        constexpr const char* EXPORT = 
            "Usage: export <filename> [results|all]\n"
            "Export games to PGN file.\n"
            "  results - Export last search results (default)\n"
            "  all     - Export entire database\n"
            "Example: export my_games.pgn results";
        
        constexpr const char* STATS = 
            "Usage: stats [detailed|verbose|none] \n"
            "Show database statistics (total games, players, date range, etc.)\n"
            "Use 'verbose' or 'detailed' argument to see more statisctics";
        
        constexpr const char* CLEAR = 
            "Usage: clear\n"
            "Remove all games from the database.";
        
        constexpr const char* HELP = 
            "Usage: help [command]\n"
            "Show help for all commands or specific command.\n"
            "Example: help search";
        
        constexpr const char* EXIT = 
            "Usage: [quit|exit]\n"
            "Exit the program.";
    }

    namespace Cli {
        constexpr size_t ARGS_VEC_RESERVE = 8;
        constexpr size_t ARG_STR_RESERVE = 32;
        
        struct ParsedCommand {
            std::string name;
            std::unordered_map<std::string, std::string> flags;
            std::vector<std::string> args;
            std::optional<std::string> error_msg;
        };

        class Application{
        private:

            Parser::Parser parser_;
            Writer::Writer writer_;
            Database::Database db_;
            std::vector<const Pgn::Model::Game*> last_search_result_;
            bool verbose_ = false;

            std::unordered_map<std::string_view, const char*> help_map_;
            std::unordered_map<std::string_view, std::function<void(const ParsedCommand&)>> cmd_map_;
            std::unordered_map<std::string_view, std::function<void(const std::string&, Database::Query&)>> flag_map_;
            bool running_ = false;
            
            std::string trim_(std::string_view s) const;
            std::vector<std::string> split_args_(std::string_view line) const;
            ParsedCommand parse_command_line_(const std::vector<std::string>& args) const;
            
            std::optional<int> parse_int_(const std::string& val);
            std::optional<Database::ColorTarget> parse_color_(const std::string& val);
 
            void print_prompt_() const;
            
            void init_cmd_map_();
            void handle_command_(const ParsedCommand &cmd);
            
            void cmd_quit_();
            void cmd_stats_(const ParsedCommand& cmd);
            void cmd_clear_();
            void cmd_load_(const ParsedCommand& cmd);
            void cmd_search_(const ParsedCommand& cmd);
            void cmd_export_(const ParsedCommand& cmd);
            void cmd_help_(const ParsedCommand& cmd);

            void init_help_map_();
            void init_flag_map_();
        public:
            Application() {
                init_help_map_();
                init_cmd_map_();
                init_flag_map_();
            };
            ~Application() = default;

            void run();
        };
    }
}

#endif