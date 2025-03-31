#!/bin/bash
# run_miniperf.sh
# This script runs 20 iterations.
# In each iteration:
#   - It launches the server with: ./miniperf -s -p 5555 (in background)
#   - Waits 1 second to ensure the server is running.
#   - Launches the client with: ./miniperf -c -p 5555 (in foreground)
#   - Waits for the server to finish before the next iteration.
# Make sure this script is executable (chmod +x run_miniperf.sh) and that
# ./miniperf is in your PATH or in the current directory.

ITERATIONS=20

for i in $(seq 1 $ITERATIONS); do
    echo "Iteration $i: Starting server and client..."

    # Start server in background and capture its process ID.
    ./miniperf -s -p 5555 &
    SERVER_PID=$!

    # Allow a brief pause to ensure the server has time to initialize.
    

    # Run the client (this runs in the foreground).
    ./miniperf -c -p 5555

    # Wait for the server process to finish before starting the next iteration.
    wait $SERVER_PID
 
    # Optional: add a brief pause before the next iteration.
done

echo "Completed $ITERATIONS iterations."
