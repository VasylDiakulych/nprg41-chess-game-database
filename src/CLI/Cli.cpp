#include "Cli.h"
#include <cctype>
#include <cstddef>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include "../General/Exception.h"
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
    
    // State for handling quotes
    // This allows spaces within quoted values (e.g., --player "Magnus Carlsen")
    bool in_quotes = false;

    for (char c : line) {
        if (c == Tokens::VALUE_DELIM) {
            // Toggle quote state
            in_quotes = !in_quotes;
        } 
        else if (std::isspace(static_cast<unsigned char>(c)) && !in_quotes) {
            // End of current argument 
            if (!current.empty()) {
                args.push_back(std::move(current));
                current.clear();
                current.reserve(ARG_STR_RESERVE);
            }
        } 
        else {
            // Regular character or space inside quotes - add to current token
            current += c;
        }
    }
    
    // Add last argument
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
    
    // First argument is always the command name
    cmd.name = args[0];
    
    // Parse remaining arguments as flags or positional arguments
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        
        // --flag or --flag=value
        if (arg.size() > 2 && arg[0] == Tokens::FLAG_PREFIX && arg[1] == Tokens::FLAG_PREFIX) {
            size_t eq_pos = arg.find(Tokens::EQUALS);
            if (eq_pos != std::string::npos) {
                // --flag=value
                std::string flag_name = arg.substr(2, eq_pos - 2);
                std::string flag_value = arg.substr(eq_pos + 1);

                // Detect duplicate flags
                if (cmd.flags.count(flag_name) > 0) {
                    cmd.error_msg = "Duplicate flag --" + flag_name;
                    return cmd;
                }
                
                cmd.flags[std::move(flag_name)] = std::move(flag_value);
            } 
            else {
                // --flag
                std::string flag_name = arg.substr(2);
                if (cmd.flags.count(flag_name) > 0) {
                    cmd.error_msg = "Duplicate flag --" + flag_name;
                    return cmd; 
                }  
                
                // Consume next argument as value if it exists and is not another flag
                if (i + 1 < args.size() && !args[i + 1].empty() && args[i + 1][0] != Tokens::FLAG_PREFIX) {
                    cmd.flags[std::move(flag_name)] = args[++i];
                } else {
                    cmd.flags[std::move(flag_name)] = "";
                }
            } 
        } 
        // -flag (single dash, treated same as long flags)
        else if (arg.size() > 1 && arg[0] == Tokens::FLAG_PREFIX) {
            std::string flag_name = arg.substr(1);
            if (cmd.flags.count(flag_name) > 0) {
                cmd.error_msg = "Duplicate flag --" + flag_name;
                return cmd; 
            }
            if (i + 1 < args.size() && !args[i + 1].empty() && args[i + 1][0] != Tokens::FLAG_PREFIX) {
                cmd.flags[std::move(flag_name)] = args[++i];
            } else {
                cmd.flags[std::move(flag_name)] = "";
            }
        } 
        // Not a flag, treat as positional argument
        else {
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
    cmd_map_[Commands::STATS] = [this](const auto& cmd) -> void { cmd_stats_(cmd); };
    cmd_map_[Commands::CLEAR] = [this](const auto&) -> void { cmd_clear_(); };
    cmd_map_[Commands::HELP] = [this](const auto& cmd) -> void { cmd_help_(cmd); };
    cmd_map_[Commands::EXIT_1] = [this](const auto&) -> void { cmd_quit_(); };
    cmd_map_[Commands::EXIT_2] = [this](const auto&) -> void { cmd_quit_(); };
}

void Pgn::Cli::Application::init_flag_map_() {
    // String filter flags
    flag_map_[Flags::PLAYER] = [this](const std::string& val, auto& q) -> void { q.player_name = val; };
    flag_map_[Flags::EVENT] = [this](const std::string& val, auto& q) -> void { q.event = val; };
    flag_map_[Flags::SITE] = [this](const std::string& val, auto& q) -> void { q.site = val; };
    flag_map_[Flags::ECO] = [this](const std::string& val, auto& q) -> void { q.eco = val; };
    flag_map_[Flags::RESULT] = [this](const std::string& val, auto& q) -> void { q.result = val; };
    flag_map_[Flags::DATE_MIN] = [this](const std::string& val, auto& q) -> void { q.date_min = val; };
    flag_map_[Flags::DATE_MAX] = [this](const std::string& val, auto& q) -> void { q.date_max = val; };
    flag_map_[Flags::OPENING] = [this](const std::string& val, auto&  q) -> void { q.opening = val; };
    flag_map_[Flags::TIME_CONTROL] = [this](const std::string& val, auto& q) -> void { q.time_control = val; };

    // Boolean flag - true if empty or "true", false otherwise
    flag_map_[Flags::VERBOSE] = [this](const std::string& val, auto& q) -> void { verbose_ = val.empty() || val == "true"; };

    // Integer flags with validation
    flag_map_[Flags::LIMIT] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.limit = res.value();
        else std::cerr << "Invalid " << Flags::LIMIT << " value\n";
    };
    flag_map_[Flags::OFFSET] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.offset = res.value();
        else std::cerr << "Invalid " << Flags::OFFSET << " value\n";
    };

    // Color flag: "w", "b", "any"
    flag_map_[Flags::COLOR] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_color_(val);
        if(res.has_value()) q.player_color = res.value();
        else std::cerr << "Invalid " << Flags::COLOR << " value\n";
    };

    // Elo rating range filters
    flag_map_[Flags::ELO_MIN] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.elo_min = res.value();
        else std::cerr << "Invalid " << Flags::ELO_MIN << " value\n";
    };

    flag_map_[Flags::ELO_MAX] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.elo_max = res.value();
        else std::cerr << "Invalid " << Flags::ELO_MAX << " value\n";
    };

    // Ply count range filters
    flag_map_[Flags::PLY_MIN] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.ply_count_min = res.value();
        else std::cerr << "Invalid " << Flags::PLY_MIN << " value\n";
    };
    flag_map_[Flags::PLY_MAX] = [this](const std::string& val, auto& q) -> void { 
        auto res = parse_int_(val);
        if(res.has_value()) q.ply_count_max = res.value();
        else std::cerr << "Invalid " << Flags::PLY_MAX << " value\n";
    };
}

std::optional<int> Pgn::Cli::Application::parse_int_(const std::string& val){
    try {
        size_t pos;
        int parsed = std::stoi(val, &pos);
        // Ensure entire string was consumed
        if(pos == val.size()){
            return parsed;
        }
        return std::nullopt;
    }
    catch(...){
        // std::stoi throws on invalid input or overflow
        return std::nullopt;
    }
}
            
std::optional<Pgn::Database::ColorTarget> Pgn::Cli::Application::parse_color_(const std::string& val) {
    // Accept "w"/"white" or "b"/"black", "any" for no color preference
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
    // Clear last search results as they may reference cleared games
    last_search_result_.clear();
    std::cout << "Database successfully cleared\n";
}

void Pgn::Cli::Application::cmd_stats_(const ParsedCommand& cmd) {
    if(cmd.args.empty()) {
        db_.print_stats();
    }
    else{
        auto arg = cmd.args[0];
        if(arg == "verbose" || arg == "detailed"){
            db_.print_stats(std::cout, true);
        }
    }
}

void Pgn::Cli::Application::cmd_help_(const ParsedCommand& cmd) {
    if (cmd.args.empty()) {
        // Show help for all commands
        std::cout << "Possible Commands:\n";
        for (const auto& [name, help] : help_map_) {
            std::cout << "Command: " << name << "\n" << help << "\n\n";
        }
    } else {
        // Show help for specific command
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
    
    // Clear previous search results as database content is changing
    last_search_result_.clear();

    try{
        // Measure parsing performance for debugging
        #ifdef DEBUG
        auto start = std::chrono::steady_clock::now();
        #endif
        
        parser_.parse_file(filename, db_);

        #ifdef DEBUG
        auto end = std::chrono::steady_clock::now();
        auto total_duration = duration_cast<std::chrono::microseconds>(end - start).count();
        #endif

        auto new_size = db_.size();
        auto total_added = new_size - prev_size;

        #ifdef DEBUG
        if (total_added > 0) {
            // Calculate and display performance metrics
            double avgTime = static_cast<double>(total_duration) / total_added;
            std::cout << "Parsed " << total_added << " games." << std::endl;
            std::cout << "Total time: " << total_duration / 1000.0 << " ms" << std::endl;
            std::cout << "Average time per game: " << avgTime << " us" << std::endl;
        }
        #endif

        std::cout << "Successfully parsed " << total_added << " games!\n";
    } catch(const Pgn::Exception& e){
         std::cerr << std::format(Pgn::LOAD_FILE_ERROR, e.what()) << "\n";
    }
}

void Pgn::Cli::Application::cmd_search_(const ParsedCommand& cmd) {
    Database::Query query;
    
    // Apply all provided flags to build the query
    // Each flag has a lambda that sets the appropriate query field
    for (const auto& [name, value] : cmd.flags) {
        if (auto it = flag_map_.find(name); it != flag_map_.end()) {
            it->second(value, query);
        }
    }
    
    // Execute search and store results for potential export
    last_search_result_ = db_.search(query);
    
    // Output results based on verbosity preference
    if(!verbose_) {
        // Compact: one line per game
        for (const auto* game : last_search_result_) {
            writer_.write_game_compact(*game, std::cout);
        }
    }
    else{
        // Full PGN 
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

    try{
        // Handle explicit option: results or all
        if(cmd.args.size() == 2){
            auto option = cmd.args[1];
            if(option == "results"){
                // Export only games from last search
                if(last_search_result_.empty()) {
                    std::cout << "Cannot export games, you did not search for anything or search result is empty!\n";
                    return;
                }
                writer_.write_games(last_search_result_, filename);
                std::cout << "Successfully exported last search result into the file " << filename << "!\n";
            }
            else if(option == "all"){ 
                // Export entire database
                writer_.write_games(db_, filename);
                std::cout<< "Successfully exported all games into the file " << filename << "!\n";
            }
            else {
                std::cout <<  "Unknown option <" << option << ">, please use [results|all].\n";
            }
            return;
        }

        // Too many arguments
        if(cmd.args.size() > 2){
            std::cout <<  "Wrong number of arguments, please use [results|all].\n";
            return;
        }

        // Default behavior
        if(!last_search_result_.empty()) {
            // Export last results if available
            writer_.write_games(last_search_result_, filename);
            std::cout << "Successfully exported last search result into the file " << filename << "!\n";
        }
        else {
            // Otherwise export all
            writer_.write_games(db_, filename);
            std::cout<< "Successfully exported all games into the file " << filename << "!\n";
        }
    } catch (const Pgn::Exception& e){
        std::cerr << std::format(Pgn::EXPORT_FILE_ERROR, e.what()) << "\n";
    }
}

void Pgn::Cli::Application::handle_command_(const ParsedCommand& cmd) {
    if (cmd.name.empty()) {
        return;
    }

    // Check for parsing errors first
    if (cmd.error_msg.has_value()) {
        std::cerr << "Error: " << cmd.error_msg.value() << '\n';
        return;  
    }
    
    // Call command handler from map
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
    
    // Main loop
    while (running_) {
        print_prompt_();
        
        // Read user input
        if (!std::getline(std::cin, line)) {
            break;
        }

        // Skip empty lines
        auto trimmed = trim_(line);
        if (trimmed.empty()) {
            continue;
        }
        
        // Tokenize input into arguments
        auto args = split_args_(trimmed);
        
        // Parse arguments into structured command
        auto cmd = parse_command_line_(args);
        
        // Execute command
        handle_command_(cmd);
    }
}
