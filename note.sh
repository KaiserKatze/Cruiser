#!/bin/sh

FILE_TODO="TODO.md"
COLOR_NONE='\033[0m'
COLOR_RED='\033[0;31m'
COLOR_YELLOW='\033[1;33m'

clear
echo "${COLOR_RED}Branches:${COLOR_NONE}"
git branch

echo "\n${COLOR_RED}Making...${COLOR_NONE}"
make > /dev/null

echo "\n${COLOR_RED}Tasks:${COLOR_NONE}"
find -type f ! -name "note.sh" ! -path "./.git*" ! -path "./build*" -exec grep -n "TODO" {} + | sed -e 's/\.\///' -e 's/\s\+\/\/\s*TODO\s*/\t/' -e 's/include\///'
echo
if [ -f $FILE_TODO ]; then
    cat $FILE_TODO
fi

git log --graph -8 --abbrev-commit --oneline

CHANGES=`git status | grep "modified:" | sed -e 's/#\s\+modified:\s\+/\t/'`
if [ ! -z $CHANGES ]; then
    echo "\nChanges not staged:"
    echo $CHANGES
fi
