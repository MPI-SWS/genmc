#!/bin/bash

# Fix for CI/CD
export TERM="${TERM:-rxvt-256color}"

# Color codes -- require ncurses package
BLACK=$(tput setaf 0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
LIME_YELLOW=$(tput setaf 190)
POWDER_BLUE=$(tput setaf 153)
BLUE=$(tput setaf 4)
MAGENTA=$(tput setaf 5)
CYAN=$(tput setaf 6)
WHITE=$(tput setaf 7)
ORANGE=$(tput setaf 9)
BRIGHT=$(tput bold)
NC=$(tput sgr0)
BLINK=$(tput blink)
REVERSE=$(tput smso)
UNDERLINE=$(tput smul)

# Various printing utilities
printline() {
    for _ in {0..67}; do echo -n '-'; done; echo ''
}
