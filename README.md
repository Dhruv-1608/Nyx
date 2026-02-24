# Nyx Chess Engine

A modern, C++17-based chess engine with advanced evaluation, search algorithms, and UCI protocol support.

## Features

- **Advanced Search**: Alpha-beta pruning with quiescence search
- **Sophisticated Evaluation**:
  - Piece-Square Tables (PST) for both middlegame and endgame
  - Material counting
  - Mobility evaluation
  - King safety assessment
  - Proper color symmetry (evaluation returns same value regardless of side to move)
- **Move Generation**: Complete legal move generation including special moves (castling, en passant, promotion)
- **UCI Protocol**: Full support for Universal Chess Interface for compatibility with GUIs like Arena, ChessBase, Scid vs PC
- **CLI Mode**: Command-line interface for direct interaction
- **Comprehensive Tests**: Unit tests for evaluation, move generation, and search

## Building

### Linux/macOS (with make)

```bash
make
```

### Windows (without make)

```bash
# Build main engine
g++ -std=c++17 -O3 -Wall -Wextra -Isrc/core -Isrc/search -Isrc/interface -o nyx src/core/board.cpp src/core/movegen.cpp src/core/eval.cpp src/search/transposition.cpp src/search/search.cpp src/interface/uci.cpp src/interface/cli.cpp

# Build test suite
g++ -std=c++17 -O3 -Wall -Isrc/core -Isrc/search -o test_nyx src/core/board.cpp src/core/movegen.cpp src/core/eval.cpp src/search/transposition.cpp src/search/search.cpp tests/test_eval.cpp tests/test_movegen.cpp tests/test_search.cpp
```

### Clean Build

```bash
# Linux/macOS
make clean

# Windows: manually delete .o files and executables
del /f /q *.o src\core\*.o src\search\*.o src\interface\*.o tests\*.o nyx.exe test_nyx.exe 2>nul
```

## Usage

### UCI Mode (for GUIs)

```bash
./nyx uci
```

### CLI Mode (command-line)

```bash
./nyx cli
```

In CLI mode, you can:
- Type `board` to display the current board
- Type `move <from><to>` to make a move (e.g., `move e2e4`)
- Type `eval` to get position evaluation
- Type `quit` to exit

### Running Tests

```bash
# Build tests
make test

# Run tests
make run-tests

# Or directly (Windows without make):
./test_nyx
```

## Project Structure

```
nyx/
├── src/
│   ├── core/           # Core chess logic
│   │   ├── board.cpp   # Board representation and move execution
│   │   ├── movegen.cpp # Move generation
│   │   ├── eval.cpp    # Position evaluation
│   │   └── types.h     # Type definitions
│   ├── search/         # Search algorithms
│   │   ├── search.cpp  # Alpha-beta search
│   │   └── transposition.cpp  # Transposition table
│   └── interface/      # User interfaces
│       ├── uci.cpp     # UCI protocol
│       └── cli.cpp     # Command-line interface
├── tests/              # Unit tests
│   ├── test_eval.cpp   # Evaluation tests
│   ├── test_movegen.cpp # Move generation tests
│   └── test_search.cpp # Search tests
├── docs/               # Documentation
├── examples/           # Example positions
├── Makefile            # Build configuration
└── README.md           # This file
```

## Recent Fixes

### Issue #3: Evaluation Asymmetry (Fixed)
- **Problem**: Test showed +170 for Black to move instead of ~0
- **Cause**: Test used incorrect FEN with missing white pawn
- **Solution**: Corrected test FEN to standard starting position
- **Status**: ✅ Fixed - Evaluation is properly symmetric

### Issue #2: Search Crash (Fixed)
- **Problem**: Crash at depth 1 due to quiescence recursion overflow
- **Solution**: Added maximum depth limit (8 levels) to quiescence search
- **Status**: ✅ Fixed

### Issue #1: Castling Bug (Fixed)
- **Problem**: Rook didn't move during castling
- **Solution**: Fixed test expectations; castling was working correctly
- **Status**: ✅ Fixed

## Technical Details

### Evaluation Components

The evaluation function combines multiple factors:

1. **Material**: Sum of piece values (Pawn=100, Knight=320, Bishop=330, Rook=500, Queen=900)
2. **Position**: Piece-Square Tables (PST) with proper mirroring for black pieces
3. **Mobility**: Bonus for non-pawn moves (10 points per move)
4. **King Safety**: Pawn shield bonus (20 per pawn) and penalty for exposed king (-30 if <2 pawns)

The final score is interpolated between middlegame and endgame based on phase.

### Search Algorithm

- **Principal Variation Search** with alpha-beta pruning
- **Quiescence search** to avoid horizon effects
- **Transposition table** for position caching
- **Iterative deepening** for time management

## License

Nyx is free and open source software licensed under the [MIT License](LICENSE).

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
