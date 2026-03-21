#ifndef DATABASE_
#define DATABASE_

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <optional>
#include "../General/Model.h"

namespace Pgn::Database {

    enum class ColorTarget { Any, White, Black };

    struct DBStats {
        size_t total_games = 0;
        size_t unique_players = 0;

        size_t white_wins = 0;
        size_t black_wins = 0;
        size_t draws = 0;
        size_t unknown = 0;
    
        std::string oldest_date = "unknown";
        std::string newest_date = "unknown";

        size_t avg_ply = 0;
        size_t rated_games = 0;

        std::vector<std::pair<std::string, size_t>> players_by_games;
        std::vector<std::pair<std::string, size_t>> openings_by_freq;
    };

    struct Query {
        std::optional<std::string> player_name;
        ColorTarget player_color = ColorTarget::Any;
        std::optional<std::string> event;
        std::optional<std::string> site;
        std::optional<std::string> eco;
        
        std::optional<std::string> result;       
        std::optional<std::string> opening;    
        std::optional<std::string> time_control;

        std::optional<std::string> date_min;     
        std::optional<std::string> date_max;    
        
        std::optional<int> ply_count_min;
        std::optional<int> ply_count_max;
        std::optional<int> elo_min;
        std::optional<int> elo_max;
        
        size_t limit = 20;
        size_t offset = 0;
    };

    class Database {
        private:
            std::vector<Pgn::Model::Game> games_;

            std::map<std::string, std::vector<size_t>> player_index_;
            std::map<std::string, std::vector<size_t>> event_index_;
            std::map<std::string, std::vector<size_t>> site_index_;
            std::unordered_map<std::string, std::vector<size_t>> eco_index_;

            [[nodiscard]] static std::string normalize_key_(std::string_view key);
            static bool matched_(std::string_view str1, std::string_view str2);

            template<typename IndexType>
            std::optional<std::unordered_set<size_t>> search_prefix_(const IndexType& index, const std::optional<std::string>& query_val) const {
                if (!query_val) return std::nullopt;
                
                std::unordered_set<size_t> result;
                auto normalized = normalize_key_(query_val.value());
                auto start = index.lower_bound(normalized);
                
                while (start != index.end()) {
                    if (start->first.compare(0, normalized.size(), normalized) != 0) 
                        break;
                    result.insert(start->second.begin(), start->second.end());
                    ++start;
                }
                
                if (result.empty()) return std::nullopt;
                return result;
            }

            template<typename IndexType>
            std::optional<std::unordered_set<size_t>> search_exact_(const IndexType& index, const std::optional<std::string>& query_val) const {
                if (!query_val.has_value()) return std::nullopt;
                
                auto it = index.find(normalize_key_(query_val.value()));
                if (it == index.end()) return std::nullopt;
                
                return std::unordered_set<size_t>(it->second.begin(), it->second.end());
            }
            
            [[nodiscard]] std::optional<std::vector<std::unordered_set<size_t>>> indexed_search_(const Query& query) const;
            [[nodiscard]] std::vector<size_t> intersect_indices_(std::vector<std::unordered_set<size_t>>& indices_val) const;
            bool satisfies_predicates_(const Pgn::Model::Game& game, const Query& query, bool check_all, std::string_view norm_player) const;

            DBStats gather_statistics_() const;

            // Basic stats
            void print_basic_stats_(std::ostream& stream) const;
            void print_result_distribution_(std::ostream& stream) const;
            void print_date_range_(std::ostream& stream) const;

            // Detailed stats
            void print_top_players_(std::ostream& stream, size_t n = 10) const;
            void print_top_openings_(std::ostream& stream, size_t n = 10) const;
            
        public:
            
            void clear();
            size_t size() { return games_.size(); };
            bool empty() { return games_.empty(); };

            const std::vector<Pgn::Model::Game>& get_games() const { return games_;}

            void add_game(Pgn::Model::Game&& game);
            [[nodiscard]] std::vector<const Pgn::Model::Game*> search(const Query& query) const;
            void print_stats(std::ostream& stream = std::cout, bool detailed = false) const;
    };

}

#endif