#ifndef DATABASE_
#define DATABASE_

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <optional>
#include "../General/Model.h"

namespace Pgn::Database {

    enum class ColorTarget { Any, White, Black };

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

            std::unordered_map<std::string, std::vector<size_t>> player_index_;
            std::unordered_map<std::string, std::vector<size_t>> event_index_;
            std::unordered_map<std::string, std::vector<size_t>> site_index_;
            std::unordered_map<std::string, std::vector<size_t>> eco_index_;

            [[nodiscard]] static std::string normalize_key_(std::string_view key);
            static bool matched_(std::string_view str1, std::string_view str2);
            
            [[nodiscard]] std::optional<std::vector<const std::vector<size_t>*>> indexed_search_(const Query& query) const;
            [[nodiscard]] std::vector<size_t> intersect_indices_(std::vector<const std::vector<size_t>*>& indices_val) const;
            bool satisfies_predicates_(const Pgn::Model::Game& game, const Query& query, bool check_all, std::string_view norm_player) const;

        public:
            void add_game(Pgn::Model::Game&& game);
            [[nodiscard]] std::vector<const Pgn::Model::Game*> search(const Query& query) const;

    };

}

#endif