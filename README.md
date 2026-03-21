# Chess Game Database
### Project for "Programming in C++" course at MFF CUNI

## Overview
Interactive CLI tool for parsing, storing, and querying chess games in PGN format.
## Features
- Load PGN files
- Search games by player, event, site, ECO code
- Prefix matching for flexible searches
- Export results
- Statistics
## Building
mkdir build && cd build
cmake ..
make
## Usage
./app
pgn-db> help
pgn-db> load pgn_files/Carlsen.pgn
pgn-db> search --player Carlsen --limit 5
pgn-db> stats detailed
pgn-db> quit