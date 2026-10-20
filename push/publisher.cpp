#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

void publish(const std::vector<std::string> &topics, const std::string &message);

int main()
{
    std::cout << "Enter Number of Topics: ";
    int num_topics;
    std::cin >> num_topics;

    std::vector<std::string> topics;
    for (int i = 0; i < num_topics; i++)
    {
        std::string topic;
        std::cout << "Enter Topic " << i + 1 << ": ";
        std::cin >> topic;
        topics.push_back(topic);
    }
    std::string message;
    std::cout << "Enter Message: ";
    std::cin >> message;
    publish(topics, message);
    return 0;
}

void publish(const std::vector<std::string> &topics, const std::string &message)
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Creating socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converting IP address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address" << std::endl;
        close(sock);
        return;
    }

    // Connecting to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        close(sock);
        return;
    }

    // Formatting the publish message with multiple topics and delimiter before the message
    std::ostringstream oss;
    oss << "PUBLISH";
    for (const auto &topic : topics)
    {
        oss << " " << topic;
    }
    oss << " !" << message; // Add "!" before the message

    std::string publish_message = oss.str();

    // Sending the formatted message to the server
    send(sock, publish_message.c_str(), publish_message.size(), 0);

    // Closing the socket
    close(sock);
}
