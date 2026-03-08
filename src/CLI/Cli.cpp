#include "Cli.h"
#include <cctype>
#include <iostream>
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
                if (i + 1 < args.size() && args[i + 1][0] != Tokens::FLAG_PREFIX) {
                    cmd.flags[std::move(flag_name)] = args[++i];
                } else {
                    cmd.flags[std::move(flag_name)] = "";
                }
            }
        } else if (arg.size() > 1 && arg[0] == Tokens::FLAG_PREFIX) {
            std::string flag_name = arg.substr(1);
            if (i + 1 < args.size() && args[i + 1][0] != Tokens::FLAG_PREFIX) {
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

void Pgn::Cli::Application::cmd_quit_() {
    running_ = false;
    std::cout << "Goodbye!\n";
}

void Pgn::Cli::Application::cmd_clear_() {

}

void Pgn::Cli::Application::cmd_stats_() {

}

void Pgn::Cli::Application::cmd_help_(const ParsedCommand& cmd) {
    if (cmd.args.empty()) {
        std::cout << "Possible Commands:\n";
        for (const auto& [name, help] : help_map_) {
            std::cout << name << "\n" << help << "\n\n";
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

};

void Pgn::Cli::Application::cmd_search_(const ParsedCommand& cmd) {

}

void Pgn::Cli::Application::cmd_export_(const ParsedCommand& cmd) {

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