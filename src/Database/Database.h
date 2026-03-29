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

    /// @brief Specifies which player color to target in queries
    enum class ColorTarget { 
        Any,    ///< Match either white or black player
        White,  ///< Match only white player
        Black   ///< Match only black player
    };

    /// @brief Statistics about the database contents
    /// @details Aggregated statistics calculated from all games in the database
    struct DBStats {
        size_t total_games = 0;         ///< Total number of games in database
        size_t unique_players = 0;      ///< Count of unique player names

        size_t white_wins = 0;          ///< Number of games won by white
        size_t black_wins = 0;          ///< Number of games won by black
        size_t draws = 0;               ///< Number of drawn games
        size_t unknown = 0;             ///< Number of games with unknown/unfinished result

        std::string oldest_date = "unknown";  ///< Earliest game date in database
        std::string newest_date = "unknown";  ///< Latest game date in database

        size_t avg_ply = 0;             ///< Average number of half-moves per game
        size_t rated_games = 0;         ///< Number of games with Elo ratings

        std::vector<std::pair<std::string, size_t>> players_by_games;   ///< Top players by game count
        std::vector<std::pair<std::string, size_t>> openings_by_freq;   ///< Top openings by frequency
    };

    /// @brief Query parameters for searching games
    struct Query {
        std::optional<std::string> player_name;     ///< Player name to search for
        ColorTarget player_color = ColorTarget::Any; ///< Which color the player played
        std::optional<std::string> event;           ///< Event name filter
        std::optional<std::string> site;            ///< Site/location filter
        std::optional<std::string> eco;             ///< ECO opening code filter

        std::optional<std::string> result;          ///< Game result filter ("1-0", "0-1", "1/2-1/2", "*")
        std::optional<std::string> opening;         ///< Opening name filter
        std::optional<std::string> time_control;    ///< Time control specification filter

        std::optional<std::string> date_min;        ///< Minimum date (YYYY.MM.DD format)
        std::optional<std::string> date_max;        ///< Maximum date (YYYY.MM.DD format)

        std::optional<int> ply_count_min;           ///< Minimum ply count
        std::optional<int> ply_count_max;           ///< Maximum ply count
        std::optional<int> elo_min;                 ///< Minimum Elo rating
        std::optional<int> elo_max;                 ///< Maximum Elo rating

        size_t limit = 20;                          ///< Maximum results to return
        size_t offset = 0;                          ///< Number of results to skip
    };

    /// @brief In-memory database for chess games
    /// @details Stores games with indexed access for efficient searching
    class Database {
        private:
            std::vector<Pgn::Model::Game> games_;   ///< Storage for all games

            // Indexes for efficient searching
            std::map<std::string, std::vector<size_t>> player_index_;   ///< Maps normalized player names to game indices
            std::map<std::string, std::vector<size_t>> event_index_;    ///< Maps normalized event names to game indices
            std::map<std::string, std::vector<size_t>> site_index_;     ///< Maps normalized site names to game indices
            std::unordered_map<std::string, std::vector<size_t>> eco_index_;  ///< Maps ECO codes to game indices

            /// @brief Normalizes a key string for indexing
            [[nodiscard]] static std::string normalize_key_(std::string_view key);

            /// @brief Compares two strings for matching
            static bool matched_(std::string_view str1, std::string_view str2);

            /// @brief Searches an ordered index by prefix
            /// @return Set of matching game indices, or nullopt if not found
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

            /// @brief Searches an index for exact match
            /// @return Set of matching game indices, or nullopt if not found
            template<typename IndexType>
            std::optional<std::unordered_set<size_t>> search_exact_(const IndexType& index, const std::optional<std::string>& query_val) const {
                if (!query_val.has_value()) return std::nullopt;

                auto it = index.find(normalize_key_(query_val.value()));
                if (it == index.end()) return std::nullopt;

                return std::unordered_set<size_t>(it->second.begin(), it->second.end());
            }

            /// @brief Performs indexed search based on query parameters
            [[nodiscard]] std::optional<std::vector<std::unordered_set<size_t>>> indexed_search_(const Query& query) const;

            /// @brief Intersects multiple index sets
            [[nodiscard]] std::vector<size_t> intersect_indices_(std::vector<std::unordered_set<size_t>>& indices_val) const;

            /// @brief Checks if a game satisfies all query predicates
            /// @return True if game matches all criteria
            bool satisfies_predicates_(const Pgn::Model::Game& game, const Query& query, bool check_all, std::string_view norm_player) const;

            /// @brief Calculates database statistics
            DBStats gather_statistics_() const;

            /// @brief Prints basic database statistics
            void print_basic_stats_(std::ostream& stream) const;

            /// @brief Prints result distribution statistics
            void print_result_distribution_(std::ostream& stream) const;

            /// @brief Prints date range information
            void print_date_range_(std::ostream& stream) const;

            /// @brief Prints top players by game count
            void print_top_players_(std::ostream& stream, size_t n = 10) const;

            /// @brief Prints top openings by frequency
            void print_top_openings_(std::ostream& stream, size_t n = 10) const;

        public:
            /// @brief Removes all games from the database
            void clear();

            /// @brief Returns the number of games in the database
            size_t size() { return games_.size(); };

            /// @brief Checks if database is empty
            bool empty() { return games_.empty(); };

            /// @brief Accesses all games (read-only)
            const std::vector<Pgn::Model::Game>& get_games() const { return games_;}

            /// @brief Adds a game to the database
            void add_game(Pgn::Model::Game&& game);

            /// @brief Searches for games matching query criteria
            [[nodiscard]] std::vector<const Pgn::Model::Game*> search(const Query& query) const;

            /// @brief Prints database statistics
            void print_stats(std::ostream& stream = std::cout, bool detailed = false) const;
    };

}

#endif
