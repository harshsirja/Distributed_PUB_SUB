#!/bin/bash

# Function to start a single broker in the background
start_broker() {
    local PORT=$1
    
    # Validate if the input is a valid number
    if ! [[ "$PORT" =~ ^[0-9]+$ ]]; then
        echo "Error: Port number must be a valid integer."
        exit 1
    fi
    
    echo "Starting the broker program on port $PORT..."
    ./broker "$PORT" &  # Run broker in the background
    # Capture the PID of the broker process
    BROKER_PID=$!
    
    # Store the PID to wait for all brokers to finish later
    BROKER_PIDS+=($BROKER_PID)
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to start broker on port $PORT!"
        exit 1
    fi
}

# Function to clean up broker processes
cleanup() {
    echo "Cleaning up broker processes..."
    for PID in "${BROKER_PIDS[@]}"; do
        if kill -0 $PID 2>/dev/null; then
            echo "Stopping broker with PID $PID..."
            kill $PID  # Terminate the broker process
        fi
    done
    echo "Cleanup complete. All brokers have been stopped."
}

# Trap signals to ensure cleanup is performed
trap cleanup EXIT SIGINT SIGTERM

read -p "How many brokers would you like to run? " NUM_BROKERS

# Validate if the input is a valid number
if ! [[ "$NUM_BROKERS" =~ ^[0-9]+$ ]]; then
    echo "Error: The number of brokers must be a valid integer."
    exit 1
fi

read -p "Enter the starting port number: " START_PORT

# Validate if the input is a valid port number
if ! [[ "$START_PORT" =~ ^[0-9]+$ ]]; then
    echo "Error: Starting port number must be a valid integer."
    exit 1
fi

# Initialize an array to hold broker PIDs
BROKER_PIDS=()

for ((i=0; i<NUM_BROKERS; i++))
do
    PORT=$((START_PORT + i))  # Increment the port number for each broker
    echo "---------------------------------"
    echo "Running broker #$((i+1)) on port $PORT"
    start_broker "$PORT"
done

# Wait for all background broker processes to finish
echo "All brokers have been started. Waiting for them to finish..."
wait "${BROKER_PIDS[@]}"  # Wait for all background processes (brokers) to finish

echo "All brokers have stopped."
exit 0
