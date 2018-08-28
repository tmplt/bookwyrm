#!/usr/bin/env bash

# get git directory
ROOT=$(git rev-parse --show-toplevel)

# install the commit hooks
if [ -f $ROOT/.git/hooks/pre-commit-clang-format ];then
    echo "WARNING: File .git/hooks/pre-commit-clang-format already exists! Overwriting previous version..."
fi
cp $ROOT/etc/git-hooks/pre-commit-clang-format-hook $ROOT/.git/hooks/pre-commit-clang-format
chmod +x $ROOT/.git/hooks/pre-commit-clang-format

if [ -f $ROOT/.git/hooks/pre-commit ];then
    echo "ERROR: File .git/hooks/pre-commit already exists! Will not overwrite, add script manually if needed."
else
    cp $ROOT/etc/git-hooks/pre-commit-hook $ROOT/.git/hooks/pre-commit
    chmod +x $ROOT/.git/hooks/pre-commit
fi
