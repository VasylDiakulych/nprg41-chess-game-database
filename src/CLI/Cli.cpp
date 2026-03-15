#include "Cli.h"
#include <cctype>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include "../General/Tokens.h"

std::string Pgn::Cli::Application::trim_(std::string_view s) const {
    auto start = s.begin();
    auto end = s.end();
    
    while (start != end && std::isspace(static_cast<unsigned char>(*start))) {
        ++start;
    }
    
    while (end != start && std::isspace(static_cast<unsigned char>(*(end - 1)))) {
        --end;
    }
    
    return std::string(start, end);
}

std::vector<std::string> Pgn::Cli::Application::split_args_(std::string_view line) const {
    std::vector<std::string> args;
    args.reserve(ARGS_VEC_RESERVE);
    
    std::string current;
    current.reserve(ARG_STR_RESERVE);
    bool in_quotes = false;

    for (char c : line) {
        if (c == Tokens::VALUE_DELIM) {
            in_quotes = !in_quotes;
        } 
        else if (std::isspace(static_cast<unsigned char>(c)) && !in_quotes) {
            if (!current.empty()) {
                args.push_back(std::move(current));
                current.clear();
                current.reserve(ARG_STR_RESERVE);
            }
        } 
        else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        args.push_back(std::move(current));
    }
    
    return args;
}

Pgn::Cli::ParsedCommand Pgn::Cli::Application::parse_command_line_(const std::vector<std::string>& args) const {
    ParsedCommand cmd;
    
    if (args.empty()) {
        return cmd;
    }
    
    cmd.name = args[0];
    
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        
        if (arg.size() > 2 && arg[0] == Tokens::FLAG_PREFIX && arg[1] == Tokens::FLAG_PREFIX) {
            size_t eq_pos = arg.find(Tokens::EQUALS);
            if (eq_pos != std::string::npos) {
                std::string flag_name = arg.substr(2, eq_pos - 2);
                std::string flag_value = arg.substr(eq_pos + 1);
                cmd.flags[std::move(flag_name)] = std::move(flag_value);
            } 
            else {
                std::string flag_name = arg.substr(2);
                if (i + 1 < args.size() && !args[i + 1].empty() && args[i + 1][0] != Tokens::FLAG_PREFIX) {
                    cmd.flags[std::move(flag_name)] = args[++i];
                } else {
                    cmd.flags[std::move(flag_name)] = "";
                }
            }
        } else if (arg.size() > 1 && arg[0] == Tokens::FLAG_PREFIX) {
            std::string flag_name = arg.substr(1);
            if (i + 1 < args.size() && !args[i + 1].empty() && args[i + 1][0] != Tokens::FLAG_PREFIX) {
                cmd.flags[std::move(flag_name)] = args[++i];
            } else {
                cmd.flags[std::move(flag_name)] = "";
            }
        } else {
            cmd.args.push_back(arg);
        }
    }
    
    return cmd;
}

void Pgn::Cli::Application::print_prompt_() const {
    std::cout << "pgn-db> ";
}

void Pgn::Cli::Application::init_help_map_() {
    help_map_[Commands::LOAD] = Help::LOAD;
    help_map_[Commands::SEARCH] = Help::SEARCH;
    help_map_[Commands::EXPORT] = Help::EXPORT;
    help_map_[Commands::STATS] = Help::STATS;
    help_map_[Commands::CLEAR] = Help::CLEAR;
    help_map_[Commands::HELP] = Help::HELP;
    help_map_[Commands::EXIT_1] = Help::EXIT;
    help_map_[Commands::EXIT_2] = Help::EXIT;
}

void Pgn::Cli::Application::init_cmd_map_() {
    cmd_map_[Commands::LOAD] = [this](const auto& cmd) -> void { cmd_load_(cmd); };
    cmd_map_[Commands::SEARCH] = [this](const auto& cmd) -> void { cmd_search_(cmd); };
    cmd_map_[Commands::EXPORT] = [this](const auto& cmd) -> void { cmd_export_(cmd); };
    cmd_map_[Commands::STATS] = [this](const auto&) -> void { cmd_stats_(); };
    cmd_map_[Commands::CLEAR] = [this](const auto&) -> void { cmd_clear_(); };
    cmd_map_[Commands::HELP] = [this](const auto& cmd) -> void { cmd_help_(cmd); };
    cmd_map_[Commands::EXIT_1] = [this](const auto&) -> void { cmd_quit_(); };
    cmd_map_[Commands::EXIT_2] = [this](const auto&) -> void { cmd_quit_(); };
}

void Pgn::Cli::Application::init_flag_map_() {
    flag_map_[Flags::PLAYER] = [this](const std::string& val, auto& q) -> void { q.player_name = val; };
    flag_map_[Flags::EVENT] = [this](const std::string& val, auto& q) -> void { q.event = val; };
    flag_map_[Flags::SITE] = [this](const std::string& val, auto& q) -> void { q.site = val; };
    flag_map_[Flags::ECO] = [this](const std::string& val, auto& q) -> void { q.eco = val; };
    flag_map_[Flags::RESULT] = [this](const std::string& val, auto& q) -> void { q.result = val; };
    flag_map_[Flags::DATE_MIN] = [this](const std::string& val, auto& q) -> void { q.date_min = val; };
    flag_map_[Flags::DATE_MAX] = [this](const std::string& val, auto& q) -> void { q.date_max = val; };
    flag_map_[Flags::OPENING] = [this](const std::string& val, auto&  q) -> void { q.opening = val; };
    flag_map_[Flags::TIME_CONTROL] = [this](const std::string& val, auto& q) -> void { q.time_control = val; };

    flag_map_[Flags::VERBOSE] = [this](const std::string& val, auto& q) -> void { verbose_ = val.empty() || val == "true"; };

    flag_map_[Flags::LIMIT] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.limit = res.value();
        else std::cout << "Invalid " << Flags::LIMIT << " value\n";
    };
    flag_map_[Flags::OFFSET] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.offset = res.value();
        else std::cout << "Invalid " << Flags::OFFSET << " value\n";
    };

    flag_map_[Flags::COLOR] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_color_(val);
        if(res.has_value()) q.player_color = res.value();
        else std::cout << "Invalid " << Flags::COLOR << " value\n";
    };

    flag_map_[Flags::ELO_MIN] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.elo_min = res.value();
        else std::cout << "Invalid " << Flags::ELO_MIN << " value\n";
    };
    flag_map_[Flags::ELO_MAX] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.elo_max = res.value();
        else std::cout << "Invalid " << Flags::ELO_MAX << " value\n";
    };

    flag_map_[Flags::PLY_MIN] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.ply_count_min = res.value();
        else std::cout << "Invalid " << Flags::PLY_MIN << " value\n";
    };
    flag_map_[Flags::PLY_MAX] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.ply_count_max = res.value();
        else std::cout << "Invalid " << Flags::PLY_MAX << " value\n";
    };

}

std::optional<int> Pgn::Cli::Application::parse_int_(const std::string& val){
    try {
        size_t pos;
        int parsed = std::stoi(val, &pos);
        if(pos == val.size()){
            return parsed;
        }
        return std::nullopt;
    }
    catch(...){
        return std::nullopt;
    }
}
            
std::optional<Pgn::Database::ColorTarget> Pgn::Cli::Application::parse_color_(const std::string& val) {
    if (val == "w") return Pgn::Database::ColorTarget::White;
    if (val == "b") return Pgn::Database::ColorTarget::Black;
    if (val == "any") return Pgn::Database::ColorTarget::Any;
    return std::nullopt;
}

void Pgn::Cli::Application::cmd_quit_() {
    running_ = false;
    std::cout << "Goodbye!\n";
}

void Pgn::Cli::Application::cmd_clear_() {
    db_.clear();
    last_search_result_.clear();
    std::cout << "Database successfully cleared\n";
}

void Pgn::Cli::Application::cmd_stats_() {
    db_.print_stats();
}

void Pgn::Cli::Application::cmd_help_(const ParsedCommand& cmd) {
    if (cmd.args.empty()) {
        std::cout << "Possible Commands:\n";
        for (const auto& [name, help] : help_map_) {
            std::cout << "Command: " << name << "\n" << help << "\n\n";
        }
    } else {
        auto it = help_map_.find(cmd.args[0]);
        if (it != help_map_.end()) {
            std::cout << it->second << "\n";
        } else {
            std::cout << "Unknown command: " << cmd.args[0] << "\n"
                      << "Type 'help' for available commands.\n";
        }
    }
}

void Pgn::Cli::Application::cmd_load_(const ParsedCommand& cmd){
    if(cmd.args.empty()){
        std::cout << "File name is missing!\n";
        return;
    }

    auto filename = cmd.args[0];
    auto prev_size = db_.size();
    last_search_result_.clear();
    parser_.parse_file(filename, db_);
    auto new_size = db_.size();

    std::cout << "Successfully parsed " << new_size - prev_size << " games!\n";
};

void Pgn::Cli::Application::cmd_search_(const ParsedCommand& cmd) {
    Database::Query query;
    
    for (const auto& [name, value] : cmd.flags) {
        if (auto it = flag_map_.find(name); it != flag_map_.end()) {
            it->second(value, query);
        }
    }
    
    last_search_result_ = db_.search(query);
    
    if(!verbose_) {
        for (const auto* game : last_search_result_) {
            writer_.write_game_compact(*game, std::cout);
        }
    }
    else{
        for (const auto* game : last_search_result_) {
            writer_.write_game(*game, std::cout);
        }
    }
    
    std::cout << "Found " << last_search_result_.size() << " games\n";
}

void Pgn::Cli::Application::cmd_export_(const ParsedCommand& cmd) {
    if(cmd.args.empty()){
        std::cout<< "Usage: export <filename> [results|all]\n"
            "Export games to PGN file.\n"
            "  results - Export last search results (default)\n"
            "  all     - Export entire database\n"
            "Example: export my_games.pgn results\n";
        return;
    }

    auto filename = cmd.args[0];

    if(cmd.args.size() == 2){
        auto option = cmd.args[1];
        if(option == "results"){
            if(last_search_result_.empty()) {
                std::cout << "Cannot export games, you did not search for anything or search result is empty!\n";
                return;
            }
            writer_.write_games(last_search_result_, filename);
            std::cout << "Successfully exported last search result into the file " << filename << "!\n";
        }
        else if(option == "all"){ 
            writer_.write_games(db_, filename);
            std::cout<< "Successfully exported all games into the file " << filename << "!\n";
        }
        else {
            std::cout <<  "Unknown option <" << option << ">, please use [results|all].\n";
        }
        return;
    }

    if(cmd.args.size() > 2){
        std::cout <<  "Wrong number of arguments, please use [results|all].\n";
        return;
    }

    writer_.write_games(db_, filename);
    std::cout<< "Successfully exported all games into the file " << filename << "!\n";
}

void Pgn::Cli::Application::handle_command_(const ParsedCommand& cmd) {
    if (cmd.name.empty()) {
        return;
    }
    
    auto it = cmd_map_.find(cmd.name);
    if (it != cmd_map_.end()) {
        it->second(cmd);
    } else {
        std::cout << "Unknown command: " << cmd.name << "\n"
                    << "Type 'help' for available commands.\n";
    }
}

void Pgn::Cli::Application::run(){
    running_ = true;
    
    std::cout << "PGN Database CLI\n";
    std::cout << "Type 'help' for commands, 'quit' to exit.\n\n";
    
    std::string line;
    
    while (running_) {
        print_prompt_();
        
        if (!std::getline(std::cin, line)) {
            break;
        }
        
        auto trimmed = trim_(line);
        if (trimmed.empty()) {
            continue;
        }
        
        auto args = split_args_(trimmed);
        auto cmd = parse_command_line_(args);
        
        handle_command_(cmd);
    }
}