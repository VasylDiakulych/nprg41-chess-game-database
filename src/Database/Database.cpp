#include "Database.h"
#include "../General/Model.h"
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cctype>

std::string Pgn::Database::Database::normalize_key_(std::string_view key) {
    std::string result;
    result.reserve(key.size());
    
    // Removes all whitespace and converts to lowercase for consistent lookup
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

    // Normalize player names and event/site for indexing
    auto black = normalize_key_(data.black);
    auto white = normalize_key_(data.white);
    auto event = normalize_key_(data.event);
    auto site = normalize_key_(data.site);

    // Using std::map for player/event/site to enable prefix search
    // Using std::unordered_map for ECO for exact match 
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

std::optional<std::vector<std::unordered_set<size_t>>> Pgn::Database::Database::indexed_search_(const Pgn::Database::Query& query) const {
    std::vector<std::unordered_set<size_t>> field_sets;

    // Use prefix search for text fields
    // E.g., searching "Kasp" will match "Kasparov"
    if (query.event.has_value()) {
        auto s = search_prefix_(event_index_, query.event);
        if (s.has_value()) {
            field_sets.emplace_back(std::move(s.value()));
        }
    } 

    if (query.site.has_value()) {
        auto s = search_prefix_(site_index_, query.site);
        if (s.has_value()) {
            field_sets.emplace_back(std::move(s.value()));
        }
    }
    
    if (query.player_name.has_value()) {
        auto s = search_prefix_(player_index_, query.player_name);
        if (s.has_value()) {
            field_sets.emplace_back(std::move(s.value()));
        }
    }

    // Use exact match for ECO codes 
    if (query.eco.has_value()) {
        auto s = search_exact_(eco_index_, query.eco);
        if (s.has_value()) {
            field_sets.emplace_back(std::move(s.value()));
        }
    }

    return field_sets;
}

std::vector<size_t> Pgn::Database::Database::intersect_indices_(std::vector<std::unordered_set<size_t>>& field_sets) const {
    if (field_sets.empty()) return {};


    // Smallest set first
    std::sort(field_sets.begin(), field_sets.end(), [](const auto& a, const auto& b) {
        return a.size() < b.size();
    });

    std::vector<size_t> candidates;
    candidates.reserve(field_sets[0].size());

    // Build intersection
    for (size_t idx : field_sets[0]) {
        bool in_all = true;
        for (size_t i = 1; i < field_sets.size(); ++i) {
            if (!field_sets[i].count(idx)) { in_all = false; break; }
        }
        if (in_all) candidates.push_back(idx);
    }

    return candidates;
}

bool Pgn::Database::Database::satisfies_predicates_(const Pgn::Model::Game& game, const Pgn::Database::Query& query, bool check_all, std::string_view norm_player) const {
    const auto& data = game.data();

    // Player name matching with color filtering
    if (query.player_name) {
        bool is_white = matched_(data.white, norm_player);
        bool is_black = matched_(data.black, norm_player);
    
        if (query.player_color == ColorTarget::White && !is_white) return false;
        if (query.player_color == ColorTarget::Black && !is_black) return false;
        
        if (check_all && query.player_color == ColorTarget::Any && !is_white && !is_black) return false;
    }

    // Direct string comparisons 
    if (query.event.has_value() && !matched_(data.event, query.event.value())) return false; 
    if (query.result.has_value() && data.result != query.result.value()) return false;
    
    // Optional field matching (only check if both game and query have values)
    if (query.opening.has_value() && data.opening.has_value() && !matched_(data.opening.value(), query.opening.value())) return false;
    if (query.time_control.has_value() && data.time_control.has_value() && !matched_(data.time_control.value(), query.time_control.value())) return false;

    // Date range comparison (string comparison works for YYYY.MM.DD format)
    if (query.date_min.has_value() && data.date < query.date_min.value()) return false;
    if (query.date_max.has_value() && data.date > query.date_max.value()) return false;

    // Ply count range
    if (query.ply_count_min.has_value() && (!data.ply_count.has_value() || data.ply_count.value() < query.ply_count_min.value())) return false;
    if (query.ply_count_max.has_value() && (!data.ply_count.has_value() || data.ply_count.value() > query.ply_count_max.value())) return false;

    // ELO rating range, game passes if either player's rating meets criteria
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
    
    // String matching, but str1 may contain spaces and str2 is normalized
    // Example: "Magnus Carlsen" (str1) should match "magnuscarlsen" (str2)
    // Returns true if str2 is a prefix of str1 
    for (size_t i = 0; i < str1.size(); ++i) {
        if (str1[i] == ' ') {
            continue;
        }

        if (j >= str2.size()) {
            return true;
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
    
    // Determine search strategy:
    // If no indexed fields matched: scan all games
    // If indexed fields matched: scan only intersection of index results
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

    // Scan candidates (or all games) and apply all predicates
    for (size_t i = 0; i < total_to_check; ++i) {
        size_t game_idx = check_all ? i : candidate_indices[i];
        const auto& game = games_[game_idx];
                
        if (!satisfies_predicates_(game, query, check_all, norm_player)) {
            continue;
        }
        
        // Skip results before offset 
        if (matches_found >= query.offset) {
            results.push_back(&game);
        }
        
        matches_found++;
        
        // Terminate once we have enough results
        if (results.size() >= query.limit) {
            break;
        }
    }

    return results;
}

void Pgn::Database::Database::clear() {
    std::vector<Pgn::Model::Game>().swap(games_);
    std::map<std::string, std::vector<size_t>>().swap(player_index_);
    std::map<std::string, std::vector<size_t>>().swap(event_index_);
    std::map<std::string, std::vector<size_t>>().swap(site_index_);
    std::unordered_map<std::string, std::vector<size_t>>().swap(eco_index_);
}

Pgn::Database::DBStats Pgn::Database::Database::gather_statistics_() const{
    Pgn::Database::DBStats stats;
    stats.total_games = games_.size();
    
    // Data structures for collecting statistics
    std::unordered_set<std::string> unique_players;
    std::unordered_map<std::string, size_t> player_games;  // For top players
    std::unordered_map<std::string, size_t> eco_counts;    // For top openings
    
    size_t total_ply = 0;
    size_t games_with_ply = 0;
    std::string oldest = "9999.99.99";  // Initialize to max possible date
    std::string newest = "0000.00.00";  // Initialize to min possible date
    bool has_date = false;

    // Single pass through all games to gather all statistics
    for (const auto& game : games_) {
        const auto& data = game.data();
        
        // Track unique players and their game counts
        auto white_norm = normalize_key_(data.white);
        auto black_norm = normalize_key_(data.black);
        unique_players.insert(white_norm);
        unique_players.insert(black_norm);
        player_games[white_norm]++;
        player_games[black_norm]++;
        
        if (data.result == "1-0") stats.white_wins++;
        else if (data.result == "0-1") stats.black_wins++;
        else if (data.result == "1/2-1/2") stats.draws++;
        else stats.unknown++;
        
        // Track date range
        if (!data.date.empty() && data.date != "????.??.??") {
            has_date = true;
            if (data.date < oldest) oldest = data.date;
            if (data.date > newest) newest = data.date;
        }
        
        // Calculate average game length
        if (data.ply_count.has_value()) {
            total_ply += data.ply_count.value();
            games_with_ply++;
        }
        
        // Count ECO code frequencies
        if (data.eco.has_value()) {
            eco_counts[data.eco.value()]++;
        }
    }
    
    stats.unique_players = unique_players.size();
    if (has_date) {
        stats.oldest_date = oldest;
        stats.newest_date = newest;
    }
    if (games_with_ply > 0) {
        stats.avg_ply = static_cast<double>(total_ply) / games_with_ply;
        stats.rated_games = games_with_ply;
    }
    
    // Sort players and openings by frequency for top 10 lists
    stats.players_by_games.assign(player_games.begin(), player_games.end());
    std::sort(stats.players_by_games.begin(), stats.players_by_games.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    stats.openings_by_freq.assign(eco_counts.begin(), eco_counts.end());
    std::sort(stats.openings_by_freq.begin(), stats.openings_by_freq.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    return stats;
}

void Pgn::Database::Database::print_stats(std::ostream& stream, bool detailed) const {
    auto stats = gather_statistics_();
    
    stream << "===== Database Statistics =====\n\n";
    stream << "Total games: " << stats.total_games << "\n";
    stream << "Unique players: " << stats.unique_players << "\n\n";
    
    // Results section with percentages
    stream << "Results:\n";
    double total = stats.total_games;
    stream << "  White wins: " << std::fixed << std::setprecision(2)
           << (stats.white_wins * 100.0 / total) << "% (" << stats.white_wins << ")\n";
    stream << "  Black wins: " << (stats.black_wins * 100.0 / total) 
           << "% (" << stats.black_wins << ")\n";
    stream << "  Draws:      " << (stats.draws * 100.0 / total) 
           << "% (" << stats.draws << ")\n";
    stream << "  Unknown:    " << (stats.unknown * 100.0 / total) 
           << "% (" << stats.unknown << ")\n\n";
    
    // Date range display
    stream << "Date range: " << stats.oldest_date << " - " << stats.newest_date << "\n";
    
    if (detailed) {
        // Additional statistics for detailed view
        if (stats.rated_games > 0) {
            stream << "Average game length: " << std::fixed << std::setprecision(2) 
                   << stats.avg_ply << " moves (from " << stats.rated_games << " rated games)\n\n";
        }
        
        // Top 10 most active players
        size_t n = std::min(size_t(10), stats.players_by_games.size());
        stream << "Top " << n << " Most Active Players:\n";
        for (size_t i = 0; i < n; ++i) {
            stream << "  " << std::setw(2) << (i + 1) << ". " 
                   << std::left << std::setw(30) << stats.players_by_games[i].first 
                   << std::right << std::setw(6) << stats.players_by_games[i].second 
                   << " games\n";
        }
        stream << "\n";
        
        // Top 10 most popular ECO openings
        n = std::min(size_t(10), stats.openings_by_freq.size());
        size_t total_eco = 0;
        for (const auto& [eco, count] : stats.openings_by_freq) total_eco += count;
        
        stream << "Top " << n << " Most Popular Openings:\n";
        for (size_t i = 0; i < n; ++i) {
            double pct = (total_eco == 0) ? 0.0 : (stats.openings_by_freq[i].second * 100.0 / total_eco);
            stream << "  " << std::setw(2) << (i + 1) << ". " 
                   << std::left << std::setw(6) << stats.openings_by_freq[i].first 
                   << std::right << std::setw(6) << stats.openings_by_freq[i].second 
                   << " games (" << std::fixed << std::setprecision(2) << pct << "%)\n";
        }
    }
    
    stream << "\n";
}
