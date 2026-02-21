
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace Pgn{

    namespace Model {

        class Game{
        private:
            //mandatory fiels
            std::string event_name;
            std::string site; // "City, Region COUNTRY" format, where COUNTRY is the three-letter International Olympic Committee code for the country
            std::string date; // "YYYY.MM.DD", ????.??.?? is used for unknown values
            std::string round;
            std::string white_player_name; // "Lastname, Firstname"
            std::string black_player_name;
            std::string result;

            std::string moves;

            //optional fieldss
            std::optional<int> white_elo;
            std::optional<int> black_elo;
            std::optional<std::string> ECO;
            std::optional<std::string> opening;
            std::optional<int> ply_count;
            std::optional<std::string> time_control;
            

        public:
            Game();
            ~Game() = default;
        };  

        enum class TimeCategory {
            Bullet, 
            Blitz,
            Rapid,
            Classical
        };
    }

}