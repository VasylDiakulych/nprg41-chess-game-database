#ifndef DATABASE_
#define DATABASE_

#include "Model.h"
#include <cstddef>
#include <vector>
namespace Pgn {
    namespace Database {
        
        using database_t = std::vector<Pgn::Model::Game>;

        class Database{
        private:
            database_t games_;
        public:
            void add_game(Pgn::Model::Game&& game) {
                games_.push_back(std::move(game));
            }

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

        class Queries{

        };

    }
}

#endif