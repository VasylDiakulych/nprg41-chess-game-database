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

    /// @brief Command name constants
    namespace Commands{
        constexpr const char* EXIT_1 = "exit";      ///< Exit command variant 1
        constexpr const char* EXIT_2 = "quit";      ///< Exit command variant 2
        constexpr const char* SEARCH = "search";    ///< Search command
        constexpr const char* HELP = "help";        ///< Help command
        constexpr const char* LOAD = "load";        ///< Load file command
        constexpr const char* EXPORT = "export";    ///< Export command
        constexpr const char* CLEAR = "clear";      ///< Clear database command
        constexpr const char* STATS = "stats";      ///< Statistics command
    }

    /// @brief Flag/option name constants for search command
    namespace Flags {
        constexpr const char* PLAYER = "player";            ///< Player name filter
        constexpr const char* ELO_MIN = "elo-min";          ///< Minimum Elo rating
        constexpr const char* ELO_MAX = "elo-max";          ///< Maximum Elo rating
        constexpr const char* EVENT = "event";              ///< Event name filter
        constexpr const char* SITE = "site";                ///< Site filter
        constexpr const char* ECO = "eco";                  ///< ECO code filter
        constexpr const char* RESULT = "result";            ///< Game result filter
        constexpr const char* DATE_MIN = "date-min";        ///< Minimum date filter
        constexpr const char* DATE_MAX = "date-max";        ///< Maximum date filter
        constexpr const char* COLOR = "color";              ///< Player color filter
        constexpr const char* PLY_MIN = "ply-count-min";    ///< Minimum ply count
        constexpr const char* PLY_MAX = "ply-count-max";    ///< Maximum ply count
        constexpr const char* OPENING = "opening";          ///< Opening name filter
        constexpr const char* TIME_CONTROL = "time-control"; ///< Time control filter
        constexpr const char* LIMIT = "limit";              ///< Result limit
        constexpr const char* OFFSET = "offset";            ///< Result offset
        constexpr const char* VERBOSE = "verbose";          ///< Verbose output flag
    }

    /// @brief Help text constants for each command
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

    /// @brief Command-line interface functionality
    namespace Cli {
        /// @brief Initial capacity for argument vectors
        constexpr size_t ARGS_VEC_RESERVE = 8;

        /// @brief Initial capacity for argument strings
        constexpr size_t ARG_STR_RESERVE = 32;

        /// @brief Parsed command structure
        struct ParsedCommand {
            std::string name;                                           ///< Command name
            std::unordered_map<std::string, std::string> flags;         ///< Flag key-value pairs
            std::vector<std::string> args;                              ///< Positional arguments
            std::optional<std::string> error_msg;                       ///< Error message if parsing failed
        };

        /// @brief Main CLI application class
        /// @details Handles command parsing, execution, and main loop
        class Application{
        private:
            Parser::Parser parser_;                                     ///< Parser instance
            Writer::Writer writer_;                                     ///< Writer instance
            Database::Database db_;                                     ///< Game database
            std::vector<const Pgn::Model::Game*> last_search_result_;   ///< Results from last search
            bool verbose_ = false;                                      ///< Verbose output flag

            std::unordered_map<std::string_view, const char*> help_map_;    ///< Maps command names to help text
            std::unordered_map<std::string_view, std::function<void(const ParsedCommand&)>> cmd_map_;  ///< Maps commands to handlers
            std::unordered_map<std::string_view, std::function<void(const std::string&, Database::Query&)>> flag_map_;  ///< Maps flags to query builders
            bool running_ = false;                                      ///< Application running state

            /// @brief Trims whitespace from a string
            std::string trim_(std::string_view s) const;

            /// @brief Splits command line into arguments
            std::vector<std::string> split_args_(std::string_view line) const;

            /// @brief Parses command line into structured command
            ParsedCommand parse_command_line_(const std::vector<std::string>& args) const;

            /// @brief Parses an integer from string
            std::optional<int> parse_int_(const std::string& val);

            /// @brief Parses color target from string
            std::optional<Database::ColorTarget> parse_color_(const std::string& val);

            /// @brief Prints the command prompt
            void print_prompt_() const;

            /// @brief Initializes command handler map
            void init_cmd_map_();

            /// @brief Handles a parsed command
            void handle_command_(const ParsedCommand &cmd);

            /// @brief Handler for quit/exit command
            void cmd_quit_();

            /// @brief Handler for stats command
            void cmd_stats_(const ParsedCommand& cmd);

            /// @brief Handler for clear command
            void cmd_clear_();

            /// @brief Handler for load command
            void cmd_load_(const ParsedCommand& cmd);

            /// @brief Handler for search command
            void cmd_search_(const ParsedCommand& cmd);

            /// @brief Handler for export command
            void cmd_export_(const ParsedCommand& cmd);

            /// @brief Handler for help command
            void cmd_help_(const ParsedCommand& cmd);

            /// @brief Initializes help text map
            void init_help_map_();

            /// @brief Initializes flag handler map
            void init_flag_map_();

        public:
            /// @brief Constructor - initializes maps
            Application() {
                init_help_map_();
                init_cmd_map_();
                init_flag_map_();
            };

            /// @brief Default destructor
            ~Application() = default;

            /// @brief Runs the CLI application main loop
            void run();
        };
    }
}

#endif
