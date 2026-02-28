#include "Writer.h"
#include "iostream"
#include "../General/Tokens.h"

void Pgn::Writer::Writer::write_tag_help_(std::ostream& out, std::string_view key, std::string_view value, bool mandatory) const {
    out << '[' << key << " \"";
    
    if (mandatory && value.empty()) {
        out << '?';
    } else {
        for (char c : value) {
            if (c == '"' || c == '\\') out << '\\';
            out << c;
        }
    }
    out << "\"]\n";
}

void Pgn::Writer::Writer::write_moves_(std::ostream& stream, std::string_view moves) const {
    size_t start = 0;

    while (start < moves.length()) {
        if (moves.length() - start <= MAX_LEN) {
            stream << moves.substr(start) << '\n';
            break;
        }

        size_t break_pos = moves.find_last_of(' ', start + MAX_LEN);

        if (break_pos == std::string_view::npos || break_pos <= start) {
            break_pos = start + MAX_LEN;
        }

        stream << moves.substr(start, break_pos - start) << '\n';
        start = break_pos + 1;
    }
}

void Pgn::Writer::Writer::write_tag_(std::ostream& stream, std::string_view key, const std::string& value) const{
    write_tag_help_(stream, key, value, true);
}

void Pgn::Writer::Writer::write_tag_(std::ostream& stream, std::string_view key, std::string_view value) const{
   write_tag_help_(stream, key, value, true);
}

void Pgn::Writer::Writer::write_tag_(std::ostream& stream, std::string_view key, const std::optional<std::string>& value) const{
    if (value) {
        write_tag_help_(stream, key, *value, false);
    }
}

void Pgn::Writer::Writer::write_tag_(std::ostream& stream, std::string_view key, std::optional<int> value) const{
    if(!value.has_value()) { return; }
    stream << '[' << key << " \"" << value.value() << "\"]\n";
}

void Pgn::Writer::Writer::write_game(const Model::Game& game, std::ostream& stream) {
    const auto& data = game.data();

    write_tag_(stream, Tokens::EVENT, data.event);
    write_tag_(stream, Tokens::SITE,  data.site);
    write_tag_(stream, Tokens::DATE,  data.date);
    write_tag_(stream, Tokens::ROUND, data.round);
    write_tag_(stream, Tokens::WHITE, data.white);
    write_tag_(stream, Tokens::BLACK, data.black);
    write_tag_(stream, Tokens::RESULT, data.result);

    write_tag_(stream, Tokens::WHITE_ELO, data.white_elo);
    write_tag_(stream, Tokens::BLACK_ELO, data.black_elo);
    write_tag_(stream, Tokens::ECO, data.eco);
    write_tag_(stream, Tokens::PLY_COUNT, data.ply_count);
    write_tag_(stream, Tokens::OPENING, data.opening);
    write_tag_(stream, Tokens::TIME_CONTROL, data.time_control);

    stream << '\n';

    write_moves_(stream, game.moves());

    stream << std::endl;
}
