#ifndef PARSER_
#define PARSER_

#include "../General/Model.h"
#include "../Database/Database.h"
#include <istream>
#include <memory>
#include <string_view>

namespace Pgn {
    /// @brief Contains PGN parsing functionality
    /// @namespace Parser
    namespace Parser {
        
        /// @brief Parser state machine states
        /// @enum State
        enum class State{
            SEARCHING,  ///< Searching for the first tag in the file
            TAG_PROCESSING, ///< Processing tags (inside [...]) 
            MOVETEXT  ///< Processing moves after text 
        };

        /// @brief PGN file parser class
        /// @class Parser
        class Parser{
        private:
            State state_; ///< Current parser state
            Model::GameData current_data_; ///< Metadata of the game being parsed
            std::unique_ptr<Model::Game> current_game_ = nullptr; ///< Game object under construction

            // Internal state for movetext parser
            int comment_depth_ = 0; ///< Nesting level of curly brace comments 
            int variation_depth_ = 0; ///< Nesting level of parenthesis variations 
            std::string current_token_; ///< Current token being built

            /// @brief Clears all game data fields
            /// @details Resets current_data_ to default values for starting a new game
            void clear_data_(){
                current_data_.event.clear();
                current_data_.site.clear();
                current_data_.date.clear();
                current_data_.round.clear();
                current_data_.white.clear();
                current_data_.black.clear();
                current_data_.result.clear();
                
                current_data_.white_elo = std::nullopt;
                current_data_.black_elo = std::nullopt;
                current_data_.eco = std::nullopt;
                current_data_.opening = std::nullopt;
                current_data_.ply_count = std::nullopt;
                current_data_.time_control = std::nullopt;
            }

            /// @brief Parses an integer from a string view
            /// @param line String containing the integer to parse
            /// @return Parsed integer value, or std::nullopt if parsing fails
            std::optional<int> parse_int_(std::string_view line);
            
            /// @brief Parses a PGN tag line
            /// @param line Raw tag line in format [Key "Value"]
            /// @details Extracts tag name and value and stores in current_data_
            void parse_tag_(std::string_view line); 

            /// @brief Parses movetext section
            /// @param line Raw movetext line from input
            /// @param db Database to receive completed games
            /// @details Processes moves, comments, and variations
            void parse_movetext_(std::string_view line, Database::Database& db);
            
            /// @brief Evaluates a single token of movetext
            /// @param token The token to process (move, number, result, etc.)
            /// @param db Database to receive completed games
            /// @details Determines whether 
            void evaluate_token_(const std::string& token, Database::Database& db);

        public:
            
            /// @brief Parses a PGN stream
            /// @param stream Input stream containing PGN data
            /// @param db Database to populate with parsed games
            /// @details Static entry point for parsing from any input stream
            static void parse(std::istream& stream, Database::Database& db);
            
            /// @brief Parses a PGN file
            /// @param filename Path to the PGN file to parse
            /// @param db Database to populate with parsed games
            /// @details File wrapper for parse() 
            static void parse_file(const std::string& filename, Database::Database& db);
        };
        
        /// @brief Trims whitespace from both ends of a string
        /// @param s String view to trim
        /// @return Substring with leading/trailing whitespace removed
        /// @details Removes leading and trailing whitespace characters
        inline std::string_view trim(std::string_view s) {
            const char* whitespace = " \t\n\r\f\v";
            
            size_t start = s.find_first_not_of(whitespace);
            if (start == std::string_view::npos) {
                return "";
            }
            
            size_t end = s.find_last_not_of(whitespace);
            return s.substr(start, end - start + 1);
        }
    }
}

#endif