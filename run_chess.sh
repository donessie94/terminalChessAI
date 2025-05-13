#!/bin/zsh
# AppleScript that opens a new Terminal window at a specific size
# and runs your 'chess' executable.

osascript <<EOF
tell application "Terminal"
    do script "cd \"$(pwd)\" && ./chess"
    set bounds of front window to {100, 100, 1600, 900}
end tell
EOF