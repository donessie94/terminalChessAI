# terminalChessAI

A terminal-based chess game written in C++, featuring an AI opponent powered by minimax search with alpha-beta pruning and an NCurses-based ASCII board display.

<p align="center">
  <img width="730" alt="terminalChessAI in action" src="https://github.com/user-attachments/assets/6ffbdb46-3896-467d-b6ec-8583e1396045" />
</p>

---

## Features

- **Interactive ASCII GUI** using NCurses  
- **AI opponent** with configurable search depth (minimax + αβ pruning)  
- **Move history** panel and undo support  
- **Clean folder structure**: `ai/`, `board/`, `logic/`, `pieces/`, `utils/`  
- **Easy build** via CMake  

---

## Controls

- **Mouse**  
  - Click on a piece to select it  
  - Click on a destination square to move  

---

## Dependencies & How to Build/Run

```bash
# 1. Install prerequisites:

# Ubuntu / Debian:
sudo apt update
sudo apt install build-essential cmake libncurses5-dev libncursesw5-dev

# macOS (Homebrew):
brew install cmake ncurses

# Windows (MSYS2 MinGW-w64):
# 1. Install MSYS2: https://www.msys2.org
# 2. In “MSYS2 MinGW-64-bit” shell:
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-ncurses

# 2. Clone & enter project:
git clone https://github.com/donessie94/terminalChessAI.git
cd terminalChessAI

# 3. Create out-of-source build directory:
mkdir build && cd build

# 4. Generate build files with CMake:
cmake ..

# 5. Compile:
cmake --build .

# 6. Run:
./chess        # Linux/macOS or MSYS2 shell on Windows
chess.exe      # same on Windows if not in a POSIX shell

# 7. Clean up:
#    simply delete the entire build/ directory when done
