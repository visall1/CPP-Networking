#include<iostream>
#include<WS2tcpip.h>
#pragma comment (lib,"ws2_32.lib")
using namespace std;

int main() 
{
	//Initialize winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsOK = WSAStartup(ver, &wsData);
	if (wsOK != 0) {
		cout << "Can't initilize windock! Quitting" << endl;
		return 0;
	}
	//Create the socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		cout << "Can't create a socket! Quitting" << endl;
		WSACleanup();
		return 0;
	}
	//Bind the IP address and port to the socket
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(1024);
	bind(s, (sockaddr*)&addr, sizeof(addr));

	//Tell to winsock the socket is for listening
	listen(s, SOMAXCONN);
	//Accept the connection 
	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET ns = accept(s,(sockaddr*)&client,&clientSize);
	if (ns == INVALID_SOCKET) {
		cout << "Can't connect with client! Quitting" << endl;
		return 0;
	}

	char host[NI_MAXHOST]; //Client remote name
	char service[NI_MAXSERV]; //Server remote port
	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);
	if (getnameinfo((sockaddr*) & client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV,0)==0)
		cout<< host<<"connected on port"<<service<<endl;
	else {
		inet_ntop(AF_INET,&client.sin_addr,host,NI_MAXHOST);
		cout << host << " connected on port" << ntohs(client.sin_port) << endl;
	}
	//Client listening socket 
	closesocket(s);
	//while loop; accept and echo message back to client 
	char buffer[1024];
	while (true) {
		ZeroMemory(buffer, 1024);
		//wait for the client to send data
		int byteReceived = recv(ns,buffer,1024,0);
		if (byteReceived == SOCKET_ERROR) {
			cout << "Error in receiving. Quitting" << endl;
			break;
		}
		if (byteReceived == 0) {
			cout << "Client disconnected." << endl;
			break;
		}
		cout << string(buffer, 0, byteReceived) << endl;
		//Echo message back to client
		send(ns, buffer, byteReceived+1,0);

	}
	closesocket(ns);
	WSACleanup();
}