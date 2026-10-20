#!/bin/bash

# Global Variables
BROKER_ADDRESS="127.0.0.1"  # Default broker address
TOPIC="xyz"                 # Default topic
MESSAGE="Hello, World!"     # Default message

# Function to generate ports array dynamically
generate_ports() {
    local START_PORT=$1
    local NUM_PORTS=$2
    local INCREMENT=$3
    
    # Generate the ports array dynamically
    PORTS=($(seq "$START_PORT" "$INCREMENT" $((START_PORT + INCREMENT * (NUM_PORTS - 1)))))
}

run_publisher() {
    local PORT=$1  # Port passed as an argument
    
    # Measure time for this publisher
    START_TIME=$(date +%s%3N)  # Current time in milliseconds
    ./publisher "$BROKER_ADDRESS" "$PORT" "$TOPIC" "$MESSAGE"
    END_TIME=$(date +%s%3N)    # Current time in milliseconds
    
    if [ $? -ne 0 ]; then
        echo "Error: Publisher failed to start!"
        exit 1
    fi
    
    # Return the elapsed time for this publisher in milliseconds
    echo $((END_TIME - START_TIME))
}

# Function to measure total time for a given number of publishers
measure_publishers_time() {
    local NUM_PUBLISHERS=$1
    local TOTAL_TIME=0
    
    # Start publishers in parallel using ports from the array, cycling with modulo
    for ((i=0; i<NUM_PUBLISHERS; i++))
    do
        PORT_INDEX=$((i % ${#PORTS[@]}))  # Use modulo to cycle through ports
        run_publisher "${PORTS[PORT_INDEX]}" &  # Run publisher in background
    done
    
    # Wait for all background processes to finish
    wait
}

# User input for port generation
read -p "Enter the starting port number: " START_PORT
read -p "Enter the number of brokers to use for port generation: " NUM_BROKERS
read -p "Enter the port increment value: " INCREMENT

# Validate input
if ! [[ "$START_PORT" =~ ^[0-9]+$ ]] || ! [[ "$NUM_BROKERS" =~ ^[0-9]+$ ]] || ! [[ "$INCREMENT" =~ ^[0-9]+$ ]]; then
    echo "Error: All inputs must be valid integers."
    exit 1
fi

# Generate the ports array dynamically
generate_ports "$START_PORT" "$NUM_BROKERS" "$INCREMENT"
echo "Ports generated: ${PORTS[*]}"

# Create a data file for plotting
DATA_FILE="publishers_time_data.dat"
> $DATA_FILE  # Clear the file if it exists

# Measure the time for running publishers from 10 to 10000 in increments
for NUM_PUBLISHERS in $(seq 10 500 2500)
do
    echo "Measuring time for $NUM_PUBLISHERS publishers..."
    
    # Measure the total time for the current number of publishers
    TOTAL_START_TIME=$(date +%s%3N)
    measure_publishers_time $NUM_PUBLISHERS
    TOTAL_END_TIME=$(date +%s%3N)
    
    # Calculate total elapsed time
    TOTAL_ELAPSED_TIME=$((TOTAL_END_TIME - TOTAL_START_TIME))
    
    # Output the result to the data file (key-value pairs)
    echo "$NUM_PUBLISHERS $TOTAL_ELAPSED_TIME" >> $DATA_FILE
done

# Plot the graph using gnuplot
gnuplot << EOF
set terminal png size 800,600
set output 'publishers_time_plot.png'
set title 'Time Taken vs Number of requsts'
set xlabel 'Number of requsts'
set ylabel 'Time Taken (ms)'
plot '$DATA_FILE' using 1:2 with linespoints title 'Publisher Run Time'
EOF

echo "Graph generated: publishers_time_plot.png"
exit 0
