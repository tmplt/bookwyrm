#!/usr/bin/env sh

# Create a temporary directory where the one tested plugin is copied to,
# and then forward this directory, and the supplied library path, to the
# binary doing the actual testing.

dir=$(mktemp --directory)
mkdir -p $dir && cp $2 $dir/

echo "Executing"
echo "\t\$ $1 $dir $3"
echo "Where $dir contains $2"
echo ""
exec $1 $dir $3
