#include "qclient.h"
#include <QtWidgets/QApplication>

#include <QApplication>
#include <QFileDialog>
#include <QDebug>
#include <iostream>
#include <fstream> 
/* MINI_POJECT
*
* Varsay�mlar
* Sistemin local networkte kullan�laca�� varsayd�m.
* Birbirinden farkl� cihazlarda kullan�lmas� i�in acknowledge mekanizmas�na ihtiya� duyulabilir.
* T�rk�e karakterde dosya isimleri olmad��� varsayd�m.
* Kullan�m
* G�ndermek istedi�imiz resmi g�nderdikten sonra a��lan pencerede resime bakt�ktan sonra o pencere kapat�l�r.
* Bir sonraki resim se�ilir. Ard�ndan farkl� Tab'lerde farkl� resimler g�zlenebilir.
*/

#include <WS2tcpip.h>
#include <windows.h>
#pragma comment (lib, "ws2_32.lib")
#define CHUNK_SIZE 65000
using namespace std;

//Choose image file from device
class QFileDialogTester : public QWidget
{
public:
    string openFile()
    {
        QString filename = QFileDialog::getOpenFileName(this, "Choose the image file to send", QDir::currentPath(), "JPEG files (*.jpg)");

        if (!filename.isNull())
        {
            return filename.toStdString();
        }
    }
};
// Reading selected image file, determine size and combine them for sending process 
byte* read_file(const char* filename)
{
    FILE* filePtr;
    filePtr = fopen(filename, "rb");
    fseek(filePtr, 0, SEEK_END);
    long length = ftell(filePtr);
    rewind(filePtr);
    byte* fileData = (byte*)malloc((sizeof(long) + length) * sizeof(byte));
    memcpy(fileData, &length, sizeof(long));
    fread(sizeof(long) + fileData, length, 1, filePtr);
    fclose(filePtr);
    return fileData;
}


int main(int argc, char** argv)
{
    SetConsoleOutputCP(65001);
    while (true) {
        QApplication app(argc, argv);
        QFileDialogTester test;

        string path = test.openFile();
        byte* file_data = read_file(path.c_str());
        long file_size = *((long*)file_data);

        cout << "FILENAME: " << path << endl;
        cout << "LENGTH: " << file_size << endl;

        int chunk_count = file_size % CHUNK_SIZE == 0 ? file_size / CHUNK_SIZE : file_size / CHUNK_SIZE + 1;
        int last_chunk_size = file_size % CHUNK_SIZE == 0 ? CHUNK_SIZE : file_size % CHUNK_SIZE;

        cout << "#Chunks: " << chunk_count << endl;

        WSADATA data;

        if (WSAStartup(MAKEWORD(2, 2), &data) == 0)
        {
            sockaddr_in server;
            server.sin_family = AF_INET;                        // AF_INET = IPv4 addresses
            server.sin_port = htons(54000);                     // Little to big endian conversion
            inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);  // Convert from string to byte array
            SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

            const char* payload = (const char*)file_data;
            // Sending size to server like hand-shaking
            int sendOk = sendto(out, payload, sizeof(long), 0, (sockaddr*)&server, sizeof(server));

            if (sendOk == SOCKET_ERROR) {
                cout << "Error while sending file size..." << endl;
                return 1;
            }
            cout << "File size is sent successfully." << endl;

            for (int i = 0; i < chunk_count; i++) {
                int sendOk;

                cout << "Sending chunk #" << i << "..." << endl;

                if (i == chunk_count - 1) {
                    sendOk = sendto(out, payload + sizeof(long) + (i * CHUNK_SIZE), last_chunk_size, 0, (sockaddr*)&server, sizeof(server));
                }
                else {
                    sendOk = sendto(out, payload + sizeof(long) + (i * CHUNK_SIZE), CHUNK_SIZE, 0, (sockaddr*)&server, sizeof(server));
                }

                if (sendOk == SOCKET_ERROR) {
                    cout << "Error while sending chunk " << i << endl;
                    return 1;
                }

                Sleep(200);
            }

            closesocket(out);
            WSACleanup();
        }

        free(file_data);
    }
    return 0;
}