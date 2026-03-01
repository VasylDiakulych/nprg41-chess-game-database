#include "Database.h"
#include "../General/Model.h"
#include <optional>
#include <string>
#include <vector>

std::string Pgn::Database::Database::normalize_key_(std::string_view key){
    size_t start = key.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) {
        return ""; 
    }
    
    size_t end = key.find_last_not_of(" \t\r\n");
    std::string result(key.substr(start, end - start + 1));

    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    return result;
}

void Pgn::Database::Database::add_game(Pgn::Model::Game&& game){
    size_t current_index = games_.size();
    const auto& data = game.data();

    auto black = normalize_key_(data.black);
    auto white = normalize_key_(data.white);
    auto event = normalize_key_(data.event);
    auto site = normalize_key_(data.site);

    if (!white.empty()) player_index_[white].push_back(current_index);
    if (!black.empty()) player_index_[black].push_back(current_index);
    if (!event.empty()) event_index_[event].push_back(current_index);
    if (!site.empty()) site_index_[site].push_back(current_index);


    if (data.eco.has_value()) {
        std::string eco = normalize_key_(data.eco.value());
        if (!eco.empty()) {
            eco_index_[eco].push_back(current_index);
        }
    }

    games_.push_back(std::move(game));
}

std::vector<Pgn::Model::Game> Pgn::Database::Database::search(const Pgn::Database::Query& query) const {
    std::vector<const std::vector<size_t>*> indices;


    if (query.event) {
        auto it = Pgn::Database::Database::event_index_.find(normalize_key_(*query.event));
        if (it != event_index_.end()) indices.push_back(&it->second);
        else return {}; 
    }

    if (query.site) {
        auto it = site_index_.find(normalize_key_(*query.site));
        if (it != site_index_.end()) indices.push_back(&it->second);
        else return {};
    }

    if (query.eco) {
        auto it = eco_index_.find(normalize_key_(*query.eco));
        if (it != eco_index_.end()) indices.push_back(&it->second);
        else return {};
    }

    if (query.player_name) {
        auto it = player_index_.find(normalize_key_(*query.player_name));
        if (it != player_index_.end()) indices.push_back(&it->second);
        else return {}; 
    }
    
    // to be implemented
}