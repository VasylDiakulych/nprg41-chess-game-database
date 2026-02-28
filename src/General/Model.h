
#ifndef MODEL_
#define MODEL_

#include <optional>
#include <string>

namespace Pgn :: Model{

    enum class TimeCategory {
        Bullet, 
        Blitz,
        Rapid,
        Classical
    };

    struct GameData {
        // mandatory fields
        std::string event;
        std::string site;
        std::string date;
        std::string round;
        std::string white;
        std::string black;
        std::string result;

        // optional fields
        std::optional<int> white_elo;
        std::optional<int> black_elo;
        std::optional<std::string> eco;
        std::optional<std::string> opening;
        std::optional<int> ply_count;
        std::optional<std::string> time_control;
    };

    struct GameConstants {
        static constexpr size_t CHARS_PER_MOVE = 15; // 10 chars in the beginning, 20 closer to the end, 15 on average
        static constexpr size_t TARGET_MOVE_COUNT = 80; // upper bound of an average length of the chess game
        static constexpr size_t TERMINATION_BUFFER = 16; // "1-0" or "0-1" or "*" or "1/2-1/2", but we use more for safety
        
        static constexpr size_t INITIAL_MOVE_RESERVE = 
            (CHARS_PER_MOVE * TARGET_MOVE_COUNT) + TERMINATION_BUFFER;
    };

    class Game{
    private:
        //mandatory fiels
        std::string event_name_;
        std::string site_; // "City, Region COUNTRY" format, where COUNTRY is the three-letter International Olympic Committee code for the country
        std::string date_; // "YYYY.MM.DD", ????.??.?? is used for unknown values
        std::string round_;
        std::string white_player_name_; // "Lastname, Firstname"
        std::string black_player_name_;
        std::string result_;

        std::string moves_;

        //optional fields
        std::optional<int> white_elo_;
        std::optional<int> black_elo_;
        std::optional<std::string> ECO_;
        std::optional<std::string> opening_;
        std::optional<int> ply_count_;
        std::optional<std::string> time_control_;
        
    public:

        Game();
        ~Game() = default;

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

        void add_moves_(const std::string& token);
        
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

        [[nodiscard]] 
        const std::string& moves() const noexcept { 
            return moves_; 
        }

    };  

}

#endif