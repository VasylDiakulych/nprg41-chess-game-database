#include "Database.h"
#include "../General/Model.h"
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cctype>

std::string Pgn::Database::Database::normalize_key_(std::string_view key) {
    std::string result;
    result.reserve(key.size()); 
    for (char c : key) {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
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

std::optional<std::vector<const std::vector<size_t>*>> Pgn::Database::Database::indexed_search_(const Pgn::Database::Query& query) const {
    std::vector<const std::vector<size_t>*> indices;
    
    if (query.event) {
        auto it = event_index_.find(normalize_key_(query.event.value()));
        if (it != event_index_.end()) indices.push_back(&it->second);
        else return std::nullopt; 
    }

    if (query.site) {
        auto it = site_index_.find(normalize_key_(query.site.value()));
        if (it != site_index_.end()) indices.push_back(&it->second);
        else return std::nullopt;
    }

    if (query.eco) {
        auto it = eco_index_.find(normalize_key_(query.eco.value()));
        if (it != eco_index_.end()) indices.push_back(&it->second);
        else return std::nullopt;
    }

    if (query.player_name) {
        auto it = player_index_.find(normalize_key_(query.player_name.value()));
        if (it != player_index_.end()) indices.push_back(&it->second);
        else return std::nullopt; 
    }

    return indices;
}

std::vector<size_t> Pgn::Database::Database::intersect_indices_(std::vector<const std::vector<size_t>*>& indices_val) const {
    std::vector<size_t> candidate_indices;

    if(indices_val.size() == 1){
        candidate_indices = *indices_val[0];
    }
    else{
        std::sort(indices_val.begin(), indices_val.end(), [](const auto* a, const auto* b) {
            return a->size() < b->size();
        });

        candidate_indices = *indices_val[0];
        std::vector<size_t> temp;

        for (size_t i = 1; i < indices_val.size(); ++i) {
            temp.clear();
            temp.reserve(candidate_indices.size()); 
            
            std::set_intersection(
                candidate_indices.begin(), candidate_indices.end(),
                indices_val[i]->begin(), indices_val[i]->end(),
                std::back_inserter(temp)
            );
            
            candidate_indices = temp;
            
            if (candidate_indices.empty()) {
                break; 
            }
        }
    }
    
    return candidate_indices;
}

bool Pgn::Database::Database::satisfies_predicates_(const Pgn::Model::Game& game, const Pgn::Database::Query& query, bool check_all, std::string_view norm_player) const {
    const auto& data = game.data();

    if (query.player_name) {
        bool is_white = matched_(data.white, norm_player);
        bool is_black = matched_(data.black, norm_player);
    
        if (query.player_color == ColorTarget::White && !is_white) return false;
        if (query.player_color == ColorTarget::Black && !is_black) return false;
        if (check_all && query.player_color == ColorTarget::Any && !is_white && !is_black) return false;
    }

    if (query.result.has_value() && data.result != query.result.value()) return false;
    if (query.opening.has_value() && data.opening != query.opening.value()) return false;
    if (query.time_control.has_value() && data.time_control != query.time_control.value()) return false;

    if (query.date_min.has_value() && data.date < query.date_min.value()) return false;
    if (query.date_max.has_value() && data.date > query.date_max.value()) return false;

    if (query.ply_count_min.has_value() && (!data.ply_count.has_value() || data.ply_count.value() < query.ply_count_min.value())) return false;
    if (query.ply_count_max.has_value() && (!data.ply_count.has_value() || data.ply_count.value() > query.ply_count_max.value())) return false;

    if (query.elo_min.has_value()) {
        bool w_pass = data.white_elo.has_value() && data.white_elo.value() >= query.elo_min.value();
        bool b_pass = data.black_elo.has_value() && data.black_elo.value() >= query.elo_min.value();
        
        if (!w_pass && !b_pass) return false;
    }

    if (query.elo_max.has_value()) {
        bool w_pass = data.white_elo.has_value() && data.white_elo.value() <= query.elo_max.value();
        bool b_pass = data.black_elo.has_value() && data.black_elo.value() <= query.elo_max.value();
        
        if (!w_pass && !b_pass) return false;
    }
    
    return true;
}

bool Pgn::Database::Database::matched_(std::string_view str1, std::string_view str2) {
    size_t j = 0;
    
    for (size_t i = 0; i < str1.size(); ++i) {
        if (str1[i] == ' ') {
            continue;
        }

        if (j >= str2.size()) {
            return false;
        }

        if (std::tolower(static_cast<unsigned char>(str1[i])) != str2[j]) {
            return false;
        }
        ++j;
    }

    return j == str2.size();
}

std::vector<const Pgn::Model::Game*> Pgn::Database::Database::search(const Pgn::Database::Query& query) const {
    auto indices = indexed_search_(query);

    if(!indices.has_value()){
        return {};
    }
    
    auto indices_val = indices.value();

    std::vector<size_t> candidate_indices;
    bool check_all = indices_val.empty();

    if(!check_all){
        candidate_indices = intersect_indices_(indices_val);
        if (candidate_indices.empty()) return {};
    }

    std::vector<const Pgn::Model::Game*> results;
    results.reserve(query.limit);

    std::string norm_player = query.player_name.has_value() ? normalize_key_(query.player_name.value()) : "";

    size_t total_to_check = check_all ? games_.size() : candidate_indices.size();
    size_t matches_found = 0;

    for (size_t i = 0; i < total_to_check; ++i) {
        size_t game_idx = check_all ? i : candidate_indices[i];
        
        const auto& game = games_[game_idx];
        
        if (!satisfies_predicates_(game, query, check_all, norm_player)) {
            continue;
        }

        if (matches_found >= query.offset) {
            results.push_back(&game);
        }
        
        matches_found++;
        if (results.size() >= query.limit) {
            break;
        }
    }

    return results;
}