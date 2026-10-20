#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

void subscribe(const std::vector<std::string> &topics);

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
    // Subscribing to multiple topics
    subscribe(topics);
    return 0;
}

void subscribe(const std::vector<std::string> &topics)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

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

    std::ostringstream subscribe_message;
    subscribe_message << "SUBSCRIBE";
    for (const auto &topic : topics)
    {
        subscribe_message << " " << topic;
    }
    std::string message = subscribe_message.str();
    send(sock, message.c_str(), message.size(), 0);

    while (true)
    {
        int bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0)
            break;

        buffer[bytes_read] = '\0';
        std::cout << "Received message: " << buffer << std::endl;
    }

    close(sock);
}
