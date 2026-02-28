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
        if (c < '0' || c > '9') {
            return std::nullopt; 
        }

        int digit = c - '0';

        if (result > (INT_MAX - digit) / 10) {
            return std::nullopt;
        }

        result = result * 10 + digit;
    }
    return result;
}

void Pgn::Parser::Parser::parse_tag_(std::string_view line){
    auto key_start = line.find_first_not_of(Tokens::TAG_OPEN);
    auto key_end = line.find_first_of(Tokens::WHITESPACE, key_start);
    auto value_start = line.find_first_of(Tokens::VALUE_DELIM, key_end) + 1;
    auto value_end = line.find_last_of(Tokens::VALUE_DELIM);

    auto key = line.substr(key_start, key_end - key_start);
    auto value = line.substr(value_start, value_end - value_start);

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
    for(char c : line){
        switch (c) {
            case Tokens::COMMENT_OPEN:
                comment_depth_++;
                continue;
            case Tokens::COMMENT_CLOSE:
                comment_depth_--;
                continue;
            case Tokens::VARIATION_OPEN:
                variation_depth_++;
                continue;
            case Tokens::VARIATION_CLOSE:
                variation_depth_--;
                continue;
            default:
                break;
        }
    
        if (comment_depth_ > 0 || variation_depth_ > 0) {
            continue;
        }

        if(std::isspace(static_cast<unsigned char>(c))){
            if(!current_token_.empty()){
                evaluate_token_(current_token_, db);
                current_token_.clear();
            

                if (state_ != State::MOVETEXT) {
                    return; 
                }
            }
        }
        else {
            current_token_ += c;
        }
    }

    if (!current_token_.empty()) {
        evaluate_token_(current_token_, db);
        current_token_.clear();
    }
}

void Pgn::Parser::Parser::evaluate_token_(const std::string& token, Database::Database& db){
    if(token.empty()) return;

    bool termination = Tokens::is_termination(token);

    current_game_->add_moves_(token);

    if(termination){
        db.add_game(std::move(*current_game_));
        current_game_.reset();
        clear_data_();
        state_ = State::SEARCHING;
    }

}

void Pgn::Parser::Parser::parse(std::istream& stream, Database::Database& db){
    std::string line;
    Parser parser;
    parser.state_ = State::SEARCHING;

    while(std::getline(stream, line)){
        std::string_view line_view = trim(line);

        if(line_view.empty()){
            if(parser.state_ == State::TAG_PROCESSING) {
                parser.current_game_ = std::make_unique<Model::Game>(std::move(parser.current_data_));
                parser.state_ = State::MOVETEXT;
                parser.clear_data_();
            }
            continue;
        }

        if(parser.state_ == State::SEARCHING || parser.state_ == State::TAG_PROCESSING){
            if(line_view.starts_with(Tokens::TAG_OPEN)) {
                parser.state_ = State::TAG_PROCESSING;
                parser.parse_tag_(line_view);
            }
        }

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