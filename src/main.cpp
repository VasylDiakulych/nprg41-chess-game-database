
#include "Database.h"
#include "Parser.h"
#include <iostream>

int main(){
    Pgn::Database::Database db;
    Pgn::Parser::Parser parser;
    parser.parse_file("pgn_files/Akobian.pgn", db);
    std::cout << db.size();
    // auto& games = db.games();
    // for(auto game : games){
    //     std::cout << game.get_event_name() << std::endl;
    // }
}