#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

const int PORT = 54000;
const string SERVER_IP = "127.0.0.1";

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Error initialization WinSock.\n";
        return -1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Error creating socket.\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in serverHint;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serverHint.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        cerr << "Error connecting to server.\n";
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    cout << "Connected to server. Press message to send:\n";

    string message;
    while (true) {
        getline(cin, message);

        if (message == "/quit") {
            cout << "Closing...\n";
            send(clientSocket, message.c_str(), message.size() + 1, 0);
            break;
        }

        int sendResult = send(clientSocket, message.c_str(), message.size() + 1, 0);
        if (sendResult == SOCKET_ERROR) {
            cerr << "Error send message.\n";
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
