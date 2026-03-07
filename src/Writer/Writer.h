#ifndef WRITER_
#define WRITER_

#include <optional>
#include <ostream>
#include <string_view>
#include <vector>
#include "../General/Model.h"
#include "iostream"

namespace Pgn::Writer {

    constexpr size_t MAX_LEN = 80;

    class Writer{
    private:
        void write_tag_help_(std::ostream& out, std::string_view key, std::string_view value, bool mandatory) const;
        void write_moves_(std::ostream& stream, std::string_view moves) const;

        void write_tag_(std::ostream& stream, std::string_view key, std::string_view value) const;
        void write_tag_(std::ostream& out, std::string_view key, const std::string& value) const;
        void write_tag_(std::ostream& stream, std::string_view key, const std::optional<std::string>& value) const;        

        void write_tag_(std::ostream& stream, std::string_view key, std::optional<int> value) const;

    public:
        Writer() = default;
        ~Writer() = default;

        void write_game(const Model::Game& game, std::ostream& stream = std::cout);
        void write_games(const std::vector<const Pgn::Model::Game*> games, std::ostream& stream = std::cout);
        void write_games(const std::vector<const Pgn::Model::Game*> games, const std::string& filename);
        void write_game_compact(const Model::Game& game, std::ostream& stream = std::cout) const;
    };
}

#endif