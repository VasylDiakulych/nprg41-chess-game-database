#ifndef PARSER_
#define PARSER_

#include "Model.h"
#include "Database.h"
#include <istream>
#include <memory>

namespace Pgn {
    namespace Parser {
        
        enum class State{
            SEARCHING,  // represents searching for the first tag in the file
            TAG_PROCESSING, // processes tags(inside [...])
            MOVETEXT  // processes moves after text
        };

        class Parser{
        private:
            State state_;
            Model::GameData current_data_;
            std::unique_ptr<Model::Game> current_game_ = nullptr;

            //internal state for movetext parser
            int comment_depth_ = 0; //pgn allows comments inside movetext inside {...}
            int variation_depth_ = 0; //pgn allows "what if" timelines that differ from what was player, denoted (...)
            std::string current_token_;

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

            void parse_int_();
            void parse_tag_(); 

            void parse_movetext_(std::string_view line, Database::Database& db);
            void evaluate_token_(const std::string& token, Database::Database& db);

        public:
            
            static void parse(std::istream& stream, Database::Database& db);
            static void parse_file(const std::string& filename, Database::Database& db);

        };
        
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