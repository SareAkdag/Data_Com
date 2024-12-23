#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <winsock2.h> // Windows soket programlama kütüphanesi.
#include <windows.h> // Windows API işlevlerini kullanmak için gerekli.

#pragma comment(lib, "ws2_32.lib") // Winsock kütüphanesini derleyiciye bağlama.

#define PORT 8080 // Sunucunun çalışacağı port numarası.
#define BUFFER_SIZE 1024 // Veri alımı/gönderimi için tampon boyutu.
#define MAX_CLIENTS 10 // Maksimum bağlantı kabul edecek istemci sayısı.

typedef struct {
    SOCKET socket; // İstemcinin soket bilgisi.
    struct sockaddr_in address; // İstemcinin adres bilgisi.
} Client;

// Soru ve cevap dizilerini tanımlama.
const char *questions[] = {
    "What is the capital of France?\n1) Berlin\n2) Madrid\n3) Paris\n4) Rome\n",
    "What is 5 + 3?\n1) 5\n2) 8\n3) 7\n4) 9\n",
    "Which planet is known as the Red Planet?\n1) Earth\n2) Mars\n3) Jupiter\n4) Saturn\n"
};
const int answers[] = {3, 2, 2}; // Soruların doğru cevapları.

// Her istemci için ayrı bir iş parçacığı çalıştıran fonksiyon.
DWORD WINAPI handle_client(void *arg) {
    Client *client = (Client *)arg; // İstemci bilgilerini al.
    char buffer[BUFFER_SIZE]; // Mesajlar için tampon oluştur.
    int question_index = 0; // Soruların sırasını tutacak değişken.

    while (question_index < 3) { // 3 soru için döngü oluştur.
        // Mevcut soruyu istemciye gönder.
        send(client->socket, questions[question_index], strlen(questions[question_index]), 0);

        // İstemciden cevabı al.
        memset(buffer, 0, BUFFER_SIZE); // Tamponu sıfırla.
        int bytes_received = recv(client->socket, buffer, BUFFER_SIZE, 0); // Cevabı al.
        if (bytes_received <= 0) break; // Bağlantı kesildiyse döngüden çık.

        int answer = atoi(buffer); // Gelen cevabı tamsayıya çevir.
        if (answer == answers[question_index]) { // Cevap doğru mu kontrol et.
            send(client->socket, "Correct!\n", 9, 0); // Doğru cevabı bildir.
        } else {
            send(client->socket, "Wrong answer.\n", 15, 0); // Yanlış cevabı bildir.
        }
        question_index++; // Sonraki soruya geç.
    }

    // Oyun bitti mesajını gönder.
    sprintf(buffer, "Game over! Thanks for playing!\n");
    send(client->socket, buffer, strlen(buffer), 0);

    closesocket(client->socket); // İstemcinin soketini kapat.
    free(client); // Belleği serbest bırak.
    return 0; // İş parçacığını sonlandır.
}

int main() {
    WSADATA wsa; // Winsock yapılandırma bilgisi.
    SOCKET server_socket, client_socket; // Sunucu ve istemci soketleri.
    struct sockaddr_in server_address, client_address; // Sunucu ve istemci adres bilgileri.

    printf("Initializing Winsock...\n");
    // Winsock başlat.
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        return 1;
    }

    printf("Winsock initialized.\n");

    // Sunucu soketi oluştur.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Could not create socket : %d\n", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    // Sunucu adresini ayarla.
    server_address.sin_family = AF_INET; // IPv4 adresleme kullan.
    server_address.sin_addr.s_addr = INADDR_ANY; // Tüm yerel IP adreslerini dinle.
    server_address.sin_port = htons(PORT); // Port numarasını ayarla.

    // Soketi adrese bağla.
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d\n", WSAGetLastError());
        return 1;
    }
    printf("Bind done.\n");

    // Gelen bağlantıları dinlemeye başla.
    listen(server_socket, MAX_CLIENTS);
    printf("Waiting for incoming connections...\n");

    int c = sizeof(struct sockaddr_in); // İstemci adresi boyutu.
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &c)) != INVALID_SOCKET) {
        printf("Connection accepted.\n");

        Client *client = malloc(sizeof(Client)); // Yeni istemci için bellek ayır.
        client->socket = client_socket; // İstemci soketini ata.
        client->address = client_address; // İstemci adresini ata.

        // İstemci için iş parçacığı oluştur.
        HANDLE thread;
        thread = CreateThread(NULL, 0, handle_client, (void *)client, 0, NULL);
        CloseHandle(thread); // İş parçacığı tanıtıcısını kapat.
    }

    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed with error code : %d\n", WSAGetLastError());
    }

    closesocket(server_socket); // Sunucu soketini kapat.
    WSACleanup(); // Winsock kaynaklarını serbest bırak.
    return 0;
}
