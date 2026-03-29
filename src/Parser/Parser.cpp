#include "Parser.h"
#include "../General/Exception.h"
#include "../General/Tokens.h"
#include <cctype>
#include <memory>
#include <fstream>
#include <optional>
#include <string_view>

std::optional<int> Pgn::Parser::Parser::parse_int_(std::string_view value){
    if(value.empty()) { return std::nullopt; }

    int result = 0;
    for (char c : value) {
        // Validate character is a digit
        if (c < '0' || c > '9') {
            return std::nullopt; 
        }

        int digit = c - '0';

        // Check for overflow before multiplying: result * 10 + digit > INT_MAX
        if (result > (INT_MAX - digit) / 10) {
            return std::nullopt;
        }

        result = result * 10 + digit;
    }
    return result;
}

void Pgn::Parser::Parser::parse_tag_(std::string_view line){
    // Parse PGN tag format: [Key "Value"]
     auto key_start = line.find_first_not_of(Tokens::TAG_OPEN);
    auto key_end = line.find_first_of(Tokens::WHITESPACE, key_start);
    auto value_start = line.find_first_of(Tokens::VALUE_DELIM, key_end) + 1;
    auto value_end = line.find_last_of(Tokens::VALUE_DELIM);

    auto key = line.substr(key_start, key_end - key_start);
    auto value = line.substr(value_start, value_end - value_start);

    // Map standard PGN tags to GameData fields
    if      (key == Tokens::EVENT)      current_data_.event = value;
    else if (key == Tokens::SITE)       current_data_.site = value;
    else if (key == Tokens::DATE)       current_data_.date = value;
    else if (key == Tokens::ROUND)      current_data_.round = value;
    else if (key == Tokens::WHITE)      current_data_.white = value;
    else if (key == Tokens::BLACK)      current_data_.black = value;
    else if (key == Tokens::RESULT)     current_data_.result = value;
    else if (key == Tokens::WHITE_ELO)  current_data_.white_elo = parse_int_(value);
    else if (key == Tokens::BLACK_ELO)  current_data_.black_elo = parse_int_(value);
    else if (key == Tokens::PLY_COUNT)  current_data_.ply_count = parse_int_(value);
    else if (key == Tokens::ECO)        current_data_.eco = value;
    else if (key == Tokens::OPENING)    current_data_.opening = value;
    else if (key == Tokens::TIME_CONTROL) current_data_.time_control = value;
}

void Pgn::Parser::Parser::parse_movetext_(std::string_view line, Database::Database& db){
    // Parsing movetext with nested comments and variations
    // Uses depth counters to track nesting level of {comments} and (variations)
    for(char c : line){
        switch (c) {
            case Tokens::COMMENT_OPEN:
                // Entering a comment block (can be nested in theory)
                comment_depth_++;
                continue;
            case Tokens::COMMENT_CLOSE:
                // Exiting a comment block
                comment_depth_--;
                continue;
            case Tokens::VARIATION_OPEN:
                // Entering a variation subline
                variation_depth_++;
                continue;
            case Tokens::VARIATION_CLOSE:
                // Exiting a variation subline
                variation_depth_--;
                continue;
            default:
                break;
        }
    
        // Skip all characters while inside comments or variations
        if (comment_depth_ > 0 || variation_depth_ > 0) {
            continue;
        }

        if(std::isspace(static_cast<unsigned char>(c))){
            // End of current token on whitespace
            if(!current_token_.empty()){
                evaluate_token_(current_token_, db);
                current_token_.clear();
            
                // Check if game ended (state changed by evaluate_token_)
                if (state_ != State::MOVETEXT) {
                    return; 
                }
            }
        }
        else {
            // Accumulate character into current token
            current_token_ += c;
        }
    }

    // Handle last token
    if (!current_token_.empty()) {
        evaluate_token_(current_token_, db);
        current_token_.clear();
    }
}

void Pgn::Parser::Parser::evaluate_token_(const std::string& token, Database::Database& db){
    if(token.empty()) return;

    // Check if this token ends the game
    bool termination = Tokens::is_termination(token);

    // Add token to current game's move list
    current_game_->add_moves_(token);

    if(termination){
        // Move game to database and reset for next game
        db.add_game(std::move(*current_game_));
        current_game_.reset();
        clear_data_();
        state_ = State::SEARCHING;  // Ready for next game's tags
    }
}

void Pgn::Parser::Parser::parse(std::istream& stream, Database::Database& db){
    std::string line;
    Parser parser;
    parser.state_ = State::SEARCHING;

    // Main parsing loop
    while(std::getline(stream, line)){
        std::string_view line_view = trim(line);

        // In PGN, empty line separates tag section from movetext section
        if(line_view.empty()){
            if(parser.state_ == State::TAG_PROCESSING) {
                // Tags finished, create Game object and prepare for moves
                parser.current_game_ = std::make_unique<Model::Game>(std::move(parser.current_data_));
                parser.state_ = State::MOVETEXT;
                parser.clear_data_();
            }
            // If current state is SEARCHING, skip line
            continue;
        }

        // TAG_PROCESSING or SEARCHING -> parse tags
        if(parser.state_ == State::SEARCHING || parser.state_ == State::TAG_PROCESSING){
            if(line_view.starts_with(Tokens::TAG_OPEN)) {
                parser.state_ = State::TAG_PROCESSING;
                parser.parse_tag_(line_view);
            }
        }
        // MOVETEXT -> parse moves
        else if(parser.state_ == State::MOVETEXT) {
            parser.parse_movetext_(line_view, db);
        }
    }
}

void Pgn::Parser::Parser::parse_file(const std::string& filename, Database::Database& db){
    std::ifstream stream;
    stream.open(filename);
    if (stream.good()) {
        parse(stream, db);
    }
    else {
        throw Pgn::Exception{1, FILE_EXCEPTION, filename};
    }
    stream.close();
}
