#! /usr/bin/env sh
# Start a simple HTTP server from which bookwyrm fetches from in DEBUG build

PORT=8000

# Kill anything occupying port port the port
kill $(lsof -i tcp:$PORT | awk 'FNR==2{ print $2 }')

top="$(git rev-parse --show-toplevel)"

# Start the server in the background
cd "$top/test"
python3 -m http.server $PORT 2>&1 > http.output &
serverpid=$!

cd "$top/build"
if [ "$1" == "--debug" ]; then
    src/bookwyrm --debug -t "Some Title" .
else
    src/bookwyrm -t "Some Title" .
fi

kill $serverpid

