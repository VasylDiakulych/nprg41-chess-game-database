#include "Model.h"
#include "Tokens.h"

void Pgn::Model::Game::add_moves_(const std::string& token){
    if(!moves_.empty()){
        moves_ += Tokens::WHITESPACE;
    }
    moves_ += token;
}