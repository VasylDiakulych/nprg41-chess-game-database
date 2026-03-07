#include <iostream>
#include <cassert>
#include <optional>
#include "Database/Database.h"
#include "General/Model.h"
#include "Writer/Writer.h"

Pgn::Model::Game create_test_game(
    std::optional<int> w_elo, 
    std::optional<int> b_elo, 
    const std::string& w_name, 
    const std::string& b_name) 
{

    auto data = Pgn::Model::GameData(
        "Paris Cup",            // event_name_
        "Paris",            // site_
        "????.??.??",   // date_
        "2",            // round_
        w_name,         // white_player_name_
        b_name,         // black_player_name_
        "1-0",             // result_
        w_elo,
        b_elo
    );

        Pgn::Model::Game game(std::move(data));
    
    return game;
}

int main() {
    Pgn::Database::Database db;
    Pgn::Writer::Writer writer;

    db.add_game(create_test_game(2499, 2400, "PlayerA", "PlayerB")); 
    db.add_game(create_test_game(2500, 2400, "PlayerC", "PlayerD")); 
    db.add_game(create_test_game(2501, 2400, "PlayerE", "PlayerF")); 
    db.add_game(create_test_game(std::nullopt, std::nullopt, "Unknown", "Unknown")); 

    Pgn::Database::Query q_boundary;
    q_boundary.elo_min = 2500;
    q_boundary.limit = 10;
    q_boundary.offset = 0;
    auto res_boundary = db.search(q_boundary);
    
    assert(res_boundary.size() == 2 && "Boundary inclusion failed");
    assert(res_boundary[0]->data().white == "PlayerC" && "Lower bound rejection failed");
    assert(res_boundary[1]->data().white == "PlayerE" && "Upper bound inclusion failed");

    Pgn::Database::Query q_null;
    q_null.elo_max = 2000;
    q_null.limit = 10;
    q_null.offset = 0;
    auto res_null = db.search(q_null);

    assert(res_null.empty() && "Nullopt resolution failed: unknown Elo evaluated as <= 2000");

    std::cout << "Engine verification passed.\n";
    return 0;
}