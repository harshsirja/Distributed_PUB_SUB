#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

std::unordered_map<std::string, std::vector<int>> topic_subscribers;
std::mutex lock;

void handle_client(int client_socket);
void publish_message(const std::vector<std::string> &topics, const std::string &message);
int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Broker server started on port " << PORT << std::endl;

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0)
    {
        std::thread client_thread(handle_client, new_socket);
        client_thread.detach();
    }

    return 0;
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE);

    if (strncmp(buffer, "SUBSCRIBE", 9) == 0)
    {
        // Handle multiple topic subscriptions
        std::istringstream iss(buffer + 10);
        std::string topic;

        std::lock_guard<std::mutex> guard(lock);

        // Parse each topic and add the client socket to the list of subscribers for that topic
        while (iss >> topic)
        {
            topic_subscribers[topic].push_back(client_socket);
            std::cout << "Subscriber added to topic: " << topic << std::endl;
        }
    }
    else if (strncmp(buffer, "PUBLISH", 7) == 0)
    {
        std::istringstream iss(buffer + 8);
        std::vector<std::string> topics;
        std::string word;

        while (iss >> word)
        {
            if (word[0] == '!')
            {
                std::string message = word.substr(1) + " ";
                std::string remainder;
                std::getline(iss, remainder); // Get the rest of the message line
                message += remainder;
                publish_message(topics, message);
                break;
            }
            else
            {
                topics.push_back(word);
            }
        }

        close(client_socket);
    }
}

void publish_message(const std::vector<std::string> &topics, const std::string &message)
{
    std::cout << "Publishing message to topics [";
    for (const auto &topic : topics)
    {
        std::cout << topic << " ";
    }
    std::cout << "]: " << message << std::endl;

    std::lock_guard<std::mutex> guard(lock);

    std::unordered_set<int> notified_subscribers;

    for (const auto &topic : topics)
    {
        if (topic_subscribers.count(topic))
        {
            for (const auto &subscriber_socket : topic_subscribers[topic])
            {
                if (!notified_subscribers.count(subscriber_socket))
                {
                    send(subscriber_socket, message.c_str(), message.size(), 0);
                    notified_subscribers.insert(subscriber_socket);
                }
            }
        }
    }
}
