
#include "Database/Database.h"
#include "Parser/Parser.h"
#include "Writer/Writer.h"

int main(){
    Pgn::Database::Database db;
    Pgn::Parser::Parser parser;
    parser.parse_file("pgn_files/Akobian.pgn", db);

    std::cout << "--- DB DIAGNOSTICS ---\n";
    for (const auto& game : db.games()) {
        const auto& data = game.data();
        
        std::cout << "Game: " << data.white << " vs " << data.black << "\n";
        
        // Check WhiteElo
        if (data.white_elo.has_value()) {
            std::cout << "  [PARSER SUCCESS] WhiteElo: " << *data.white_elo << "\n";
        } else {
            std::cout << "  [PARSER FAIL/MISSING] WhiteElo has no value\n";
        }
        
        // Check ECO (since your diff showed it missing too)
        if (data.eco.has_value()) {
            std::cout << "  [PARSER SUCCESS] ECO: " << *data.eco << "\n";
        } else {
            std::cout << "  [PARSER FAIL/MISSING] ECO has no value\n";
        }
    }
    std::cout << "--- END DIAGNOSTICS ---\n\n";
        
}