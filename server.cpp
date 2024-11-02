#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <unordered_set>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

const int PORT = 54000;
unordered_set<SOCKET> clients;
mutex clientsMutex;

void broadcastMessage(const std::string& message, SOCKET senderSocket) {
	lock_guard<mutex> lock(clientsMutex);
	for (SOCKET clientSocket : clients) {
		if (clientSocket != senderSocket) {
			send(clientSocket, message.c_str(), message.size(), 0);
		}
	}
}

void handleClient(SOCKET clientSocket) {
	char buffer[4096];
	while (true) {
		memset(buffer, 0, sizeof(buffer));
		int bytesRecieved = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

		if (bytesRecieved <= 0) {
			cout << "Client disconnected: " << clientSocket << "\n";
			break;
		}

		string message(buffer, bytesRecieved);

		if (message == "/quit") {
			cout << "Client disconnected: " << clientSocket << "\n";
			break;
		}

		cout << "Message from client " << clientSocket << ": " << message << "\n";
		broadcastMessage(message, clientSocket);
	}

	{
		lock_guard<mutex> lock(clientsMutex);
		clients.erase(clientSocket);
	}
	closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro initialization WinSock.\n";
        return -1;
    }

    SOCKET listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket == INVALID_SOCKET) {
        std::cerr << "Error of creating socket.\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in serverHint;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(PORT);
    serverHint.sin_addr.s_addr = INADDR_ANY;

    if (bind(listeningSocket, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        std::cerr << "Error banding socket.\n";
        closesocket(listeningSocket);
        WSACleanup();
        return -1;
    }

    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening.\n";
        closesocket(listeningSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Server running on port " << PORT << ". Waiting connection...\n";

    while (true) {
        sockaddr_in clientHint;
        int clientSize = sizeof(clientHint);
        SOCKET clientSocket = accept(listeningSocket, (sockaddr*)&clientHint, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error user connection....\n";
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.insert(clientSocket);
        }

        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    closesocket(listeningSocket);
    WSACleanup();
    return 0;
}