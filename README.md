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
- **Easy build** via a simple Makefile  

---

## How to Run

- **Clone the repository:**  
  `git clone https://github.com/donessie94/terminalChessAI.git && cd terminalChessAI`

- **Compile the game:**  
  `make`

- **Launch the game:**  
  `./chess`

---

## Controls

- **Mouse**  
  - Click on a piece to select it  
  - Click on a destination square to move  

---

## Dependencies

- A C++11-compatible compiler (e.g. `g++`)  
- NCurses development headers and library  

```bash
# Ubuntu / Debian:
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev

# macOS (Homebrew):
brew install ncurses

# Windows (MSYS2/MinGW-w64)
	1.	Install MSYS2 from https://www.msys2.org
	2.	Open “MSYS2 MinGW-64-bit” shell and run:
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image
