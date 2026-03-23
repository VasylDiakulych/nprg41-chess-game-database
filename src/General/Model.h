
#ifndef MODEL_
#define MODEL_

#include <optional>
#include <string>

///@brief Core data models for PGN chess game representation
namespace Pgn :: Model{
    ///@brief Categorizes chess games by time control format
    ///@details Standard chess time control classifications based on game duration
    enum class TimeCategory {
        Bullet, 
        Blitz,
        Rapid,
        Classical
    };

    /// @brief Plain data structure holding all metadata for a chess game
    /// @details Used as a transfer object between parser and Game constructor.
    ///          Contains both 7 PGN mandatory fields and optional fields.
    struct GameData {
        // Mandatory PGN fields
        std::string event;      ///Event name (e.g., "FIDE World Championship 2023")
        std::string site;       ///Location in "City, Region COUNTRY" format
        std::string date;       ///Date in "YYYY.MM.DD" format (???? for unknown)
        std::string round;      ///Round number within the event
        std::string white;      ///White player name ("Lastname, Firstname")
        std::string black;      ///Black player name ("Lastname, Firstname")
        std::string result;     ///Game result: "1-0", "0-1", "1/2-1/2", or "*"

        // Optional fields
        std::optional<int> white_elo;               ///White player's ELO rating
        std::optional<int> black_elo;               ///Black player's ELO rating
        std::optional<std::string> eco;             ///ECO opening code (e.g., "C42")
        std::optional<std::string> opening;         ///Opening name
        std::optional<int> ply_count;               ///Number of half-moves played
        std::optional<std::string> time_control;    ///Time control (e.g., "300+3")

    };


    /// @brief Constants for move string storage
    /// @details Pre-calculates buffer sizes to minimize reallocations when 
    ///          building the move string. Based on statistical analysis of 
    ///          typical chess game lengths.
    struct GameConstants {
        static constexpr size_t CHARS_PER_MOVE = 15; // 10 chars in the beginning, 20 closer to the end, 15 on average
        static constexpr size_t TARGET_MOVE_COUNT = 80; // upper bound of an average length of the chess game
        static constexpr size_t TERMINATION_BUFFER = 16; // "1-0" or "0-1" or "*" or "1/2-1/2", but we use more for safety
        
        static constexpr size_t INITIAL_MOVE_RESERVE = 
            (CHARS_PER_MOVE * TARGET_MOVE_COUNT) + TERMINATION_BUFFER;
    };

    /// @brief Represents a complete chess game with all metadata and moves
    /// @details Immutable after construction except for move appending.
    ///          Uses move semantics for efficient construction from GameData.
    class Game{
    private:
    // Mandatory PGN fields
    std::string event_name_;      ///Tournament/event name
    std::string site_;              ///Location (IOC country code format)
    std::string date_;              ///Game date (YYYY.MM.DD)
    std::string round_;             ///Round identifier
    std::string white_player_name_; ///White player ("Lastname, Firstname")
    std::string black_player_name_; ///Black player ("Lastname, Firstname")
    std::string result_;            ///Final result code
    std::string moves_;             ///SAN move notation string
    
    // Optional metadata
    std::optional<int> white_elo_;              ///White ELO rating
    std::optional<int> black_elo_;              ///Black ELO rating
    std::optional<std::string> ECO_;            ///Encyclopedia of Chess Openings code
    std::optional<std::string> opening_;        ///Opening name
    std::optional<int> ply_count_;              ///Half-move count
    std::optional<std::string> time_control_;   ///Time control specification
        
    public:
        /// @brief Constructs empty game
        Game();

        /// @brief Default destructor
        ~Game() = default;

        /// @brief Constructs game from parsed data
        /// @param data Game metadata (moved from)
        /// @details Pre-allocates move buffer 
        Game(GameData&& data) :
            event_name_(std::move(data.event)),
            site_(std::move(data.site)),
            date_(std::move(data.date)),
            round_(std::move(data.round)),
            white_player_name_(std::move(data.white)),
            black_player_name_(std::move(data.black)),
            result_(std::move(data.result)),

            white_elo_(data.white_elo), // integer
            black_elo_(data.black_elo), // integer
            ECO_(std::move(data.eco)),
            opening_(std::move(data.opening)),
            ply_count_(data.ply_count), // integer
            time_control_(std::move(data.time_control))
        {
            moves_.reserve(GameConstants::INITIAL_MOVE_RESERVE); // reserving space for moves string, to improve performance
        };

        /// @brief Appends move token to the moves string
        /// @param token SAN move notation to append
        void add_moves_(const std::string& token);
        
        /// @brief Returns complete game metadata
        /// @return GameData struct with all fields populated
        [[nodiscard]] 
        GameData data() const {
            return GameData{
                event_name_,
                site_,
                date_,
                round_,
                white_player_name_,
                black_player_name_,
                result_,
                white_elo_,
                black_elo_,
                ECO_,
                opening_,
                ply_count_,
                time_control_
            };
        }

        /// @brief Accesses the move notation string
        /// @return const reference to moves in SAN format
        [[nodiscard]] 
        const std::string& moves() const noexcept { 
            return moves_; 
        }

    };  

}

#endif