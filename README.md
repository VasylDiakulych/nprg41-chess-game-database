# Chess Game Database
### Project for "Programming in C++" course at MFF CUNI

## Overview

Interactive CLI tool for parsing, storing, and querying chess games in PGN format

## Features

- **Load PGN files** - Parse and import chess games
- **Search games** by player, event, site, ECO code with prefix matching
- **Filtering** - Date ranges, ELO ratings, ply counts, game results
- **Export results** - Save search results or entire database to PGN files
- **Statistics** -  Analytics including player rankings and popular openings
- **Interactive CLI** - Command-line interface for navigating

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Quick Start 

```bash
./app
pgn-db> help
pgn-db> load pgn_files/Carlsen.pgn
pgn-db> search --player Carlsen --limit 5
pgn-db> stats detailed
pgn-db> quit
```

## Architecture

The project follows a modular design with class separation:

```

src/
|-- CLI/           # Command-line interface and user interaction
|-- Database/      # Game storage, indexing, and search
|-- General/       # Core data models and constants
|-- Parser/        # PGN file parsing
--- Writer/        # PGN output and formatting
```

### Core Components

#### Model (General/Model.h)

The `Game` class represents a complete chess game with metadata:

- **Mandatory PGN Tags**: Event, Site, Date, Round, White, Black, Result
- **Optional Fields**: ELO ratings, ECO codes, opening names, ply counts, time controls
- **Move Storage**: SAN (Standard Algebraic Notation) move strings

#### Parser (Parser/)

Implements a state machine for PGN parsing:

It handles transitions between tag sections and movetext, supports comments `{...}` and variations `(...)` with depth tracking

#### Database (Database/)

In-memory storage with indexes for fast queries.
It uses ordered maps for player, event, site, which allows prefix search and unordered map for ECO codes for fast exact lookup
  
**Search Algorithm**:
  1. Use indexes to find candidate game sets for each criterion
  2. Intersect index results to narrow candidates
  3. Apply predicates on candidates
  4. Support partial output with offset/limit

#### Writer (Writer/)

Writer implements PGN output formatting, with standard 80 character per line wrapping, mandatory  and optional non-empty tags.
Supports 2 formats: standard and compact
#### CLI (CLI/)

Interactive command-line interface handles user input and parses + executes commands.
## Usage Examples

### Loading Games

```
pgn-db> load pgn_files/Carlsen.pgn
Parsed 7484 games.
```

### Searching

```
pgn-db> search --player "Carlsen" --elo-min 2800 --limit 10
Carlsen,M (2840) vs Abdusattorov,Nodirbek (2732) | 2025.12.30 | World Blitz 2025 | 1/2-1/2 | ECO: B40
...
Found 10 games
```

**Available Search Flags:**
- `--player <name>`: Filter by player name (supports partial matching)
- `--color <w|b|any>`: Specify player color
- `--elo-min/max <n>`: ELO rating range
- `--date-min/max <YYYY.MM.DD>`: Date range
- `--event <name>`: Event filter (supports partial matching)
- `--site <name>`: Site filter (supports partial matching)
- `--eco <code>`: ECO opening code (exact match)
- `--result <1-0|0-1|1/2-1/2|*>`: Game result
- `--opening <name>`: Opening name filter
- `--ply-count-min/max <n>`: Ply count (half-moves) range
- `--limit <n>`: Maximum results (default: 20)
- `--offset <n>`: Skip first n results
- `--verbose`: Show full PGN output

### Exporting

```
pgn-db> export results.pgn results
Successfully exported last search result into the file results.pgn!

pgn-db> export all.pgn all
Successfully exported all games into the file all.pgn!
```

### Statistics

```
pgn-db> stats
===== Database Statistics =====  
  
Total games: 7484  
Unique players: 1287  
  
Results:  
 White wins: 39.15% (2930)  
 Black wins: 30.28% (2266)  
 Draws:      30.57% (2288)  
 Unknown:    0.00% (0)  
  
Date range: 2001.01.05 - 2025.12.30

pgn-db> stats detailed
===== Database Statistics =====  
  
Total games: 7484  
Unique players: 1287  
  
Results:  
 White wins: 39.15% (2930)  
 Black wins: 30.28% (2266)  
 Draws:      30.57% (2288)  
 Unknown:    0.00% (0)  
  
Date range: 2001.01.05 - 2025.12.30  
Top 10 Most Active Players:  
  1. carlsen,m                       7475 games  
  2. nakamura,hi                      406 games  
  3. firouzja,alireza                 266 games  
  4. aronian,l                        237 games  
  5. so,w                             235 games  
  6. caruana,f                        216 games  
  7. vachierlagrave,m                 208 games  
  8. nepomniachtchi,i                 173 games  
  9. anand,v                          143 games  
 10. tang,andrew                      142 games  
  
Top 10 Most Popular Openings:  
  1. A00      253 games (3.38%)  
  2. A45      156 games (2.09%)  
  3. D37      148 games (1.98%)  
  4. C65      141 games (1.89%)  
  5. B30      140 games (1.87%)  
  6. B06      137 games (1.83%)  
  7. C50      130 games (1.74%)  
  8. A07      128 games (1.71%)  
  9. B00      128 games (1.71%)  
 10. B40      118 games (1.58%)
```

### Other Commands

```
pgn-db> clear          # Remove all games from database
pgn-db> help           # Show available commands
pgn-db> help search    # Show help for specific command
pgn-db> quit           # Exit application
```
