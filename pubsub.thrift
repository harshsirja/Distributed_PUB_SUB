namespace cpp PubSub

struct SubscriptionRequest {
    1: string topic;
    2: string subscriber_id;
}

struct SubscriptionResponse {
    1: bool success;
    2: string message;
}

struct PublishRequest {
    1: string topic;
    2: string message_data;
}

struct PublishResponse {
    1: bool success;
}

struct NotifyRequest {
    1: string topic;
    2: string message_data;
}

struct NotifyResponse {
    1: bool success;
}

struct SyncRequest {
    1: list<string> topics;
    2: map<string, string> message_logs; // topic -> latest message
}

struct SyncResponse {
    1: bool success;
}

struct GetMessagesRequest {
    1: string topic;
    2: string subscriber_id; // Identify which subscriber is requesting the messages
}

struct GetMessagesResponse {
    1: bool success;
    2: list<string> messages;
    3: string message; // Optional message for status or feedback
}

// Define an exception
exception SubscriptionException {
    1: string message;
}

// Service definition
service BrokerService {
    SubscriptionResponse registerSubscriber(1: SubscriptionRequest request);
    PublishResponse publishMessage(1: PublishRequest request);
    // NotifyResponse notifySubscribers(1: NotifyRequest request);
    // SyncResponse syncBrokers(1: SyncRequest request);
    GetMessagesResponse getMessages(1: GetMessagesRequest request); // New method added
}
