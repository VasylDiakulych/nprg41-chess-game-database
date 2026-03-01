#ifndef DATABASE_
#define DATABASE_

#include "../General/Model.h"
#include <cstddef>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Pgn {
    namespace Database {
        
        using database_t = std::vector<Pgn::Model::Game>;

        enum class ColorTarget { White, Black, Any };

        struct Query{
            size_t limit = 50;
            size_t offset = 0;

            std::optional<std::string> player_name; 
            ColorTarget player_color = ColorTarget::Any;

            std::optional<std::string> event;
            std::optional<std::string> eco;
            std::optional<std::string> site;

            std::optional<std::string> result;
            std::optional<std::string> opening;
            std::optional<std::string> time_control;
            std::optional<std::string> round;

            std::optional<int> elo_min;
            std::optional<int> elo_max;
            
            std::optional<int> ply_count_min;
            std::optional<int> ply_count_max;

            std::optional<std::string> date_min;
            std::optional<std::string> date_max;
        };

        class Database{
        private:

            database_t games_;

            std::unordered_map<std::string, std::vector<size_t>> player_index_;
            std::unordered_map<std::string, std::vector<size_t>> event_index_;
            std::unordered_map<std::string, std::vector<size_t>> eco_index_;
            std::unordered_map<std::string, std::vector<size_t>> site_index_;

            static std::string normalize_key_(std::string_view key);

        public:
            void add_game(Pgn::Model::Game&& game);
            std::vector<Model::Game> search(const Query& query) const;

            size_t size() const noexcept {
                return games_.size();
            }

            const database_t& games() const noexcept {
                return games_;
            }

            void clear() {
                games_.clear();
            }
        };

    }
}

#endif