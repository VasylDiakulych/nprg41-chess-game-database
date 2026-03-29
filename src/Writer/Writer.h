#ifndef WRITER_
#define WRITER_

#include <optional>
#include <ostream>
#include <string_view>
#include <vector>
#include "../General/Model.h"
#include "../Database/Database.h"
#include "iostream"

namespace Pgn::Writer {

    /// @brief Maximum line length for PGN output
    constexpr size_t MAX_LEN = 80;

    /// @brief Contains PGN writing functionality
    /// @details Provides methods to write chess games in PGN format to streams or files
    class Writer{
    private:
        /// @brief Helper for writing tag pairs with validation
        void write_tag_help_(std::ostream& stream, std::string_view key, std::string_view value, bool mandatory) const;

        /// @brief Writes the moves section with proper formatting
        /// @details Formats moves with line wrapping at MAX_LEN characters
        void write_moves_(std::ostream& stream, std::string_view moves) const;

        /// @brief Writes a string view tag pair
        void write_tag_(std::ostream& stream, std::string_view key, std::string_view value) const;

        /// @brief Writes a string tag pair
        void write_tag_(std::ostream& stream, std::string_view key, const std::string& value) const;

        /// @brief Writes an optional string tag pair
        void write_tag_(std::ostream& stream, std::string_view key, const std::optional<std::string>& value) const;

        /// @brief Writes an optional integer tag pair
        void write_tag_(std::ostream& stream, std::string_view key, std::optional<int> value) const;

    public:
        /// @brief Default constructor
        Writer() = default;

        /// @brief Default destructor
        ~Writer() = default;

        /// @brief Writes a single game in PGN format
        /// @details Outputs complete PGN with all tags and moves
        void write_game(const Model::Game& game, std::ostream& stream = std::cout) const;

        /// @brief Writes multiple games to a stream
        /// @details Each game is separated by a blank line
        void write_games(const std::vector<const Pgn::Model::Game*> games, std::ostream& stream = std::cout) const;

        /// @brief Writes multiple games to a file
        /// @details Creates or overwrites the file
        void write_games(const std::vector<const Pgn::Model::Game*> games, const std::string& filename) const;

        /// @brief Writes all games from database to stream
        void write_games(const Database::Database& db, std::ostream& stream = std::cout) const;

        /// @brief Writes all games from database to file
        /// @details Wrapper for writing games to stream
        void write_games(const Database::Database& db, const std::string& filename) const;

        /// @brief Writes a game in compact format (single line)
        /// @details Used in search to not overwhelm konsole output
        void write_game_compact(const Model::Game& game, std::ostream& stream = std::cout) const;
    };
}

#endif
