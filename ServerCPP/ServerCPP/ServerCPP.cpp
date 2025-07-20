#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <map>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFFER_SIZE 1024

std::map<SOCKET, std::string> clients;
std::mutex clientsMutex;

void BroadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (const auto& [client, username] : clients) {
        send(client, message.c_str(), message.length(), 0);
    }
}

void BroadcastUserList() {
    std::string userList = "/users";
    for (const auto& [client, username] : clients) {
        userList += " " + username;
    }
    BroadcastMessage(userList);
}

void HandleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;
    std::string username;

    // Receive username
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead <= 0) {
        closesocket(clientSocket);
        return;
    }

    buffer[bytesRead] = '\0';
    username = buffer;
    std::string clientIP = "Unknown";

    sockaddr_in clientAddr;
    int addrSize = sizeof(clientAddr);
    getpeername(clientSocket, (sockaddr*)&clientAddr, &addrSize);
    clientIP = inet_ntoa(clientAddr.sin_addr);

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients[clientSocket] = username + " (" + clientIP + ")";
        std::cout << username << " connected from " << clientIP << std::endl;
    }

    BroadcastUserList();
    BroadcastMessage(username + " joined the chat.");

    // Message handling loop
    while ((bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytesRead] = '\0';
        std::string message = buffer;
        if (message.starts_with("/typing")) {
            BroadcastMessage("/typing " + username);
        }
        else {
            BroadcastMessage(username + ": " + message);
        }
    }

    // Client disconnected
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        std::cout << username << " disconnected." << std::endl;
        clients.erase(clientSocket);
    }
    BroadcastUserList();
    BroadcastMessage(username + " left the chat.");
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Chat server started on port " << PORT << "..." << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting client." << std::endl;
            continue;
        }
        std::thread(HandleClient, clientSocket).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}