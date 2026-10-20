#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/server/TThreadedServer.h>
#include "gen-cpp/pubsub_types.h"
#include "gen-cpp/BrokerService.h"
#include <sw/redis++/redis++.h>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <mutex>
#include <sstream>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace PubSub;
using namespace sw::redis;

class BrokerServiceHandler : public BrokerServiceIf
{
private:
    std::mutex mutex_;
    Redis redis;

public:
    BrokerServiceHandler() : redis("tcp://127.0.0.1:6379") {}

    void registerSubscriber(SubscriptionResponse &_return, const SubscriptionRequest &request) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        registerSubscriberInRedis(request.topic, request.subscriber_id);
        _return.success = true;
        _return.message = "Subscriber registered successfully.";
    }

    void publishMessage(PublishResponse &_return, const PublishRequest &request) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        storeMessageInRedis(request.topic, request.message_data);
        std::cout << "Message published and stored in Redis for topic: " << request.topic << std::endl;
        _return.success = true;
    }

    void getMessages(GetMessagesResponse &_return, const GetMessagesRequest &request) override
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Get the messages for the requested topic
        auto messages = getMessagesFromRedis(request.topic);

        // Get the last read index for the subscriber
        size_t last_read_index = getLastReadIndexFromRedis(request.topic, request.subscriber_id);

        // Get only the unread messages
        std::vector<std::string> unread_messages;
        for (size_t i = last_read_index; i < messages.size(); ++i)
        {
            unread_messages.push_back(messages[i]);
        }

        // Respond with unread messages
        if (!unread_messages.empty())
        {
            _return.success = true;
            _return.messages = unread_messages;

            // Update the last read index for the subscriber
            updateLastReadIndexInRedis(request.topic, request.subscriber_id, messages.size());
        }
        else
        {
            _return.success = false;
            _return.message = "No new messages.";
        }
    }

private:
    void registerSubscriberInRedis(const std::string &topic, const std::string &subscriber_id)
    {
        redis.sadd("topic_subscribers:" + topic, subscriber_id);
        std::cout << "Subscriber " << subscriber_id << " registered for topic: " << topic << " in Redis." << std::endl;
    }

    void storeMessageInRedis(const std::string &topic, const std::string &message)
    {
        redis.rpush("topic_messages:" + topic, message);
        std::cout << "Message stored for topic: " << topic << " in Redis." << std::endl;
    }

    std::set<std::string> getSubscribersFromRedis(const std::string &topic)
    {
        std::set<std::string> subscribers;
        redis.smembers("topic_subscribers:" + topic, std::inserter(subscribers, subscribers.end())); // Use std::inserter
        return subscribers;
    }

    std::vector<std::string> getMessagesFromRedis(const std::string &topic)
    {
        std::vector<std::string> messages;
        redis.lrange("topic_messages:" + topic, 0, -1, std::back_inserter(messages)); // Pass the output iterator
        return messages;
    }

    size_t getLastReadIndexFromRedis(const std::string &topic, const std::string &subscriber_id)
    {
        std::string key = "last_read_index:" + topic;
        auto index = redis.hget(key, subscriber_id);
        return index ? std::stoul(*index) : 0; // Default to 0 if not found
    }

    void updateLastReadIndexInRedis(const std::string &topic, const std::string &subscriber_id, size_t index)
    {
        std::string key = "last_read_index:" + topic;
        redis.hset(key, subscriber_id, std::to_string(index));
    }
};

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int port = std::stoi(argv[1]);
    std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    // Create a shared pointer for the handler
    std::shared_ptr<BrokerServiceHandler> handler(new BrokerServiceHandler());

    // Pass the shared pointer to the processor
    std::shared_ptr<TProcessor> processor(new BrokerServiceProcessor(handler));

    TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
    // std::cout << "Broker server started on port " << port << std::endl;
    server.serve();
    return 0;
}
