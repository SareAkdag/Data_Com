#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Winsock kütüphanesini bağlama

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa; // Winsock kütüphanesi için gerekli veri yapısı
    SOCKET client_socket; // Bağlantı için soket tanımı
    struct sockaddr_in server_address; // Sunucunun adres bilgilerini saklamak için yapı
    char buffer[BUFFER_SIZE]; // Veri alışverişi için tampon bellek

    // Winsock'u başlatıyoruz
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // Winsock 2.2 versiyonunu yükle
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        return 1; // Hata durumunda programı sonlandır
    }

    printf("Winsock initialized.\n");

    // Soket oluşturma
    client_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP soketi oluştur
    if (client_socket == INVALID_SOCKET) { // Soket oluşturma başarısızsa
        printf("Could not create socket : %d\n", WSAGetLastError());
        return 1; // Programı sonlandır
    }
    printf("Socket created.\n");

    // Sunucunun adres bilgilerini ayarla
    server_address.sin_family = AF_INET; // IPv4 protokolü
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Yerel adres (localhost)
    server_address.sin_port = htons(PORT); // Sunucu portu

    // Sunucuya bağlan
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // Bağlantı isteği
        printf("Connection failed with error code : %d\n", WSAGetLastError());
        return 1; // Bağlantı başarısızsa programı sonlandır
    }
    printf("Connected to server.\n");

    // Sunucudan soruları almak ve cevap vermek için bir döngü
    while (1) {
        // Sunucudan soruyu al
        memset(buffer, 0, BUFFER_SIZE); // Tamponu sıfırla
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0); // Sunucudan veri al
        if (bytes_received <= 0) break; // Veri alınamazsa döngüyü sonlandır

        printf("%s", buffer); // Soruyu ekrana yazdır

        // Kullanıcıdan cevap al ve sunucuya gönder
        printf("Your answer: ");
        fgets(buffer, BUFFER_SIZE, stdin); // Kullanıcıdan girdi al
        send(client_socket, buffer, strlen(buffer), 0); // Cevabı sunucuya gönder
    }

    // Bağlantıyı kapat ve Winsock'u temizle
    closesocket(client_socket); // Soketi kapat
    WSACleanup(); // Winsock kaynaklarını temizle
    return 0;
}
