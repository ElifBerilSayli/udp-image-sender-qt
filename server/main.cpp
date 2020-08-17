#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <iostream>
#include <fstream> 
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable : 4996)
#define CHUNK_SIZE 65000
#pragma warning(disable : 4996)
using namespace std;

// Adding Tabs to window for each image
void draw(QByteArray arr, QMainWindow* window, QWidget* centralWidget, QTabWidget* tabs)
{
	QPixmap image;
	QLabel* label = new QLabel(tabs);
	image.loadFromData(arr);
	label->setPixmap(image);
	label->setFixedHeight(image.height());
	label->setFixedWidth(image.width());
	tabs->addTab(label, ("Image"));
}
int main(int argc, char** argv)
{
	// Some setup for window
	QApplication app(argc, argv);
	QMainWindow* window = new QMainWindow();
	QWidget* centralWidget = new QWidget(window);
	QTabWidget* tabs = new QTabWidget(centralWidget);
	window->setWindowTitle(QString::fromUtf8("Image Receiver"));
	window->resize(1500, 1500);
	// UDP connection basics via winsock
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		cout << "Can't start Winsock! " << endl;
		return NULL;
	}
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;			// Us any IP address available on the machine
	serverHint.sin_family = AF_INET;					// Address format is IPv4
	serverHint.sin_port = htons(54000);					// Convert from little to big endian
	if (::bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't bind socket! " << WSAGetLastError() << endl;
		return NULL;
	}
	sockaddr_in client;									// Use to hold the client information (port / ip address)
	int clientLength = sizeof(client);					// The size of the client information

	char* buf = (char*)malloc(CHUNK_SIZE);

	while (true) {

		// Clear Buffer
		ZeroMemory(&client, clientLength);
		ZeroMemory(buf, CHUNK_SIZE);

		cout << "Waiting for file size..." << endl;
		int bytesIn = recvfrom(in, buf, sizeof(long), 0, (sockaddr*)&client, &clientLength);
		if (bytesIn == SOCKET_ERROR)
		{
			cout << "Error while receiving file size..." << endl;
			return NULL;
		}

		long file_size = *((long*)buf);
		cout << "File size received succesfully: [" << file_size << "]" << endl;
		char* fileBuf = (char*)malloc(file_size);

		int chunk_count = file_size % CHUNK_SIZE == 0 ? file_size / CHUNK_SIZE : file_size / CHUNK_SIZE + 1;
		// Determine last datagram packet size
		int last_chunk_size = file_size % CHUNK_SIZE == 0 ? CHUNK_SIZE : file_size % CHUNK_SIZE;
		cout << "#Chunks: " << chunk_count << endl;
		for (int i = 0; i < chunk_count; i++) {
			// Clear Buffer
			ZeroMemory(&client, clientLength);
			ZeroMemory(buf, CHUNK_SIZE);
			int bytesIn;

			if (i == chunk_count - 1) {
				bytesIn = recvfrom(in, fileBuf + (i * CHUNK_SIZE), last_chunk_size, 0, (sockaddr*)&client, &clientLength);
			}
			else {
				bytesIn = recvfrom(in, fileBuf + (i * CHUNK_SIZE), CHUNK_SIZE, 0, (sockaddr*)&client, &clientLength);
			}

			if (bytesIn == SOCKET_ERROR)
			{
				cout << "Error while receiving chunk " << i << endl;
				return NULL;
			}
			cout << "Receiving chunk #" << i << "..." << endl;
		}
		cout << "File Received succesfully!" << endl;
		QByteArray ba(fileBuf, file_size);
		draw(ba, window, centralWidget, tabs);
		free(fileBuf);
		window->setCentralWidget(centralWidget);
		window->show();
		app.exec();
	}

	closesocket(in);
	WSACleanup();
	return 0;
}