#include<iostream>
#include<string>
#include<WS2tcpip.h>
#pragma comment (lib,"ws2_32.lib")
using namespace std;
int main() {
	//Initialize winsock
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0) {
		cout << "Can't start winsock, Error"<<wsResult << endl;
		return 0;
	}
	//Create socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		cout<<"Can't Create socket. Error No"<<WSAGetLastError << endl;
		WSACleanup();
		return 0;
	}
	//Fill in the structure
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1024);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	//Connect to server
	int connResult = connect(s, (sockaddr*)&addr, sizeof(addr));
	if (connResult == SOCKET_ERROR) {
		cout << "Can't connect to server. Error No" << WSAGetLastError << endl;
		closesocket(s);
		WSACleanup();
		return 0;
	}

	//do-while loop to send and receive data
	char buffer[1024];
	string userInput;
	do {
		//Promt the user for some text
		cout<<"Message > ";
		getline(cin, userInput);

		//Make sure the user has typed something 
		if (userInput.size() > 0) {
			//send the text to server
			int sendResult = send(s,userInput.c_str(),userInput.size()+1,0);

			if (sendResult != SOCKET_ERROR) {
				//wait for response
				ZeroMemory(buffer,1024);
				int byteReceived = recv(s, buffer, 1024,0);
				if (byteReceived > 0) {
					//Echo response from server
					cout << "SERVER> " << string(buffer, 0, byteReceived) << endl;
				}
			}

		}
	} while (userInput.size() > 0);

	//Close everthing 
	closesocket(s);
	WSACleanup();
	return 0;
}