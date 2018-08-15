#!/usr/bin/env sh

# Create a temporary directory where the one tested plugin is copied to,
# and then forward this directory, and the supplied library path, to the
# binary doing the actual testing.

dir=$(mktemp --directory)
mkdir -p $dir/$2 && cp $2 "$_"

echo "Executing $1 $dir/$2 $3"
exec $1 $dir/$2 $3
