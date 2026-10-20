# Define the compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -I/usr/local/include
LDFLAGS = -lthrift -lhiredis -lredis++ -pthread

# Define the source files
SOURCES = broker.cpp publisher.cpp subscriber.cpp
GEN_CPP_DIR = gen-cpp
THRIFT_CPP_FILES = $(GEN_CPP_DIR)/BrokerService.cpp $(GEN_CPP_DIR)/BrokerService.h $(GEN_CPP_DIR)/pubsub_types.cpp $(GEN_CPP_DIR)/pubsub_types.h 
# THRIFT_CPP_FILES = $(wildcard $(GEN_CPP_DIR)/*.cpp)

# Define the output executables
BROKER = broker
PUBLISHER = publisher
SUBSCRIBER = subscriber

# Default target to build everything
all: $(BROKER) $(PUBLISHER) $(SUBSCRIBER)

# Rule to build the Broker
$(BROKER): Broker.cpp $(THRIFT_CPP_FILES)
	$(CXX) $(CXXFLAGS) -o $(BROKER) Broker.cpp $(THRIFT_CPP_FILES) $(LDFLAGS)

# Rule to build the Publisher
$(PUBLISHER): Publisher.cpp $(THRIFT_CPP_FILES)
	$(CXX) $(CXXFLAGS) -o $(PUBLISHER) Publisher.cpp $(THRIFT_CPP_FILES) $(LDFLAGS)

# Rule to build the Subscriber
$(SUBSCRIBER): Subscriber.cpp $(THRIFT_CPP_FILES)
	$(CXX) $(CXXFLAGS) -o $(SUBSCRIBER) Subscriber.cpp $(THRIFT_CPP_FILES) $(LDFLAGS)

# Clean up generated files
clean:
	rm -f $(BROKER) $(PUBLISHER) $(SUBSCRIBER)

# PHONY targets
.PHONY: all clean
