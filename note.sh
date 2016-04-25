#!/bin/sh

clear
echo "Branches:"
git branch

echo "\nMaking..."
make > /dev/null

echo "\nTasks:"
find -type f ! -name "note.sh" -exec grep -n "TODO" {} + | sed -e 's/\.\///' -e 's/\s\+\/\/\s*TODO\s*/\t/' -e 's/include\///'

echo
git log --graph -8 --abbrev-commit --oneline

echo "\nChanges not staged:"
git status | grep "modified:" | sed -e 's/#\s\+modified:\s\+/\t/'
