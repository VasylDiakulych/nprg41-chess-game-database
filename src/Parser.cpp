#include "Parser.h"
#include "Database.h"
#include "Exception.h"
#include "Model.h"
#include "Tokens.h"
#include <cctype>
#include <memory>
#include <fstream>

void Pgn::Parser::Parser::parse_int_(){
    //to be implemented
}

void Pgn::Parser::Parser::parse_tag_(){
    //to be implemented
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
            }

            if (state_ != State::MOVETEXT) {
                return; 
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
                parser.parse_tag_();
            }
        }

        else if(parser.state_ == State::MOVETEXT) {
            parser.parse_movetext_(line, db);
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