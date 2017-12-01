#! /usr/bin/env sh
# Start a simple HTTP server from which bookwyrm fetches from in DEBUG build

top="$(git rev-parse --show-toplevel)"

# Start the server in the background
cd "$top/test"
python3 -m http.server 8000 2>&1 > http.output &
serverpid=$!

cd "$top/build"
if [ "$1" == "--debug" ]; then
    src/bookwyrm --debug -t "Some Title" .
else
    src/bookwyrm -t "Some Title" .
fi

kill $serverpid

