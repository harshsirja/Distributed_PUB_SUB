#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "gen-cpp/pubsub_types.h"
#include "gen-cpp/BrokerService.h"

#include <iostream>
#include <thread>
#include <chrono>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace PubSub;

void pollMessages(const std::string &broker_address, const int port, const std::string &subscriber_id, const std::string &topic)
{
    std::shared_ptr<TTransport> socket(new TSocket(broker_address, port)); // Broker's port
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    BrokerServiceClient client(protocol);

    try
    {
        transport->open();
        while (true)
        {
            GetMessagesRequest request;
            request.topic = topic;
            request.subscriber_id = subscriber_id;

            GetMessagesResponse response;
            client.getMessages(response, request);

            if (response.success)
            {
                if (!response.messages.empty())
                {
                    std::cout << "Subscriber " << subscriber_id << " received new messages for topic " << topic << ":";
                    for (const auto &message : response.messages)
                    {
                        std::cout << " - " << message << std::endl;
                    }
                }
                else
                {
                    std::cout << "Subscriber " << subscriber_id << ": No new messages for topic " << topic << std::endl;
                }
            }
            else
            {
                std::cout << "Error: " << response.message << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::minutes(1)); // Poll every minute
        }
        transport->close();
    }
    catch (TException &tx)
    {
        std::cerr << "Error: " << tx.what() << std::endl;
    }
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << " <broker_address> <port> <topic> <subscriber_id>" << std::endl;
        return 1;
    }
    std::string broker_address = argv[1];
    int port = std::stoi(argv[2]);
    std::string topic = argv[3];
    std::string subscriber_id = argv[4];

    // Register the subscriber
    std::shared_ptr<TTransport> socket(new TSocket(broker_address, port)); // Broker's port
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    BrokerServiceClient client(protocol);

    try
    {
        transport->open();
        SubscriptionRequest request;
        request.subscriber_id = subscriber_id;
        request.topic = topic;

        SubscriptionResponse response;
        client.registerSubscriber(response, request);

        if (response.success)
        {
            std::cout << "Subscriber " << subscriber_id << " registered successfully for topic: " << topic << std::endl;

            // Start the polling function in a separate thread
            std::thread pollThread(pollMessages, broker_address, port, subscriber_id, topic);
            pollThread.join(); // Wait for the thread to complete (or run indefinitely)
        }
        else
        {
            std::cout << "Failed to register subscriber: " << response.message << std::endl;
        }

        transport->close();
    }
    catch (TException &tx)
    {
        std::cerr << "Registration error: " << tx.what() << std::endl;
    }

    return 0;
}
