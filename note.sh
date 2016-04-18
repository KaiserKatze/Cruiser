#!/bin/sh

clear
echo "Branches:"
git branch

echo "\nMaking..."
make > /dev/null
echo

git log --graph -8 --abbrev-commit --oneline
echo

git status
