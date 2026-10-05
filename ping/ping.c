#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>

uint16_t icmp_checksum(void *data, size_t length);

int main() {

    // ICMP Package Structure
    struct icmp_echo{
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint16_t identifier;
        uint16_t sequence;
    };

    struct icmp_echo request;

    request.type = 8;
    request.code = 0;
    request.checksum = 0;
    request.identifier = htons(100);
    request.sequence = htons(1);

    request.checksum = htons(icmp_checksum(&request, sizeof(request)));

    // Winsock startup
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        perror("WSAStartup");
        return 1;
    }

    // Socket declaration
    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0) {
        perror("socket");
        return 1;
    }

    // Address structure
    struct sockaddr_in destination;
    memset(&destination, 0, sizeof(destination));

    destination.sin_family = AF_INET;
    destination.sin_addr.s_addr = inet_addr("127.0.0.1");

    // ICMP package sending
    int sent = sendto(
        sock,
        (const char *)&request,
        sizeof(request),
        0,
        (struct sockaddr *)&destination,
        sizeof(destination)
    );

    if (sent == SOCKET_ERROR) {
        printf("sendto failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Recieves the bytes from response
    char buffer[65535];
    int destination_len = sizeof(destination);

    int received = recvfrom(
        sock,
        buffer,
        sizeof(buffer),
        0,
        (struct sockaddr *)&destination,
        &destination_len
    );

    if (received == SOCKET_ERROR) {
        printf("recvfrom failed: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Bytes received: %d\n", received);

    // Get the ICMP Header from reply
    uint8_t ihl = buffer[0] & 0x0F;
    size_t ipv4_header_len = ihl * 4;
    uint8_t *icmp_header = (uint8_t *)buffer + ipv4_header_len;

    struct icmp_echo *reply = (struct icmp_echo *)icmp_header;
    printf("Type: %u\n", reply->type);
    printf("Code: %u\n", reply->code);
    printf("Checksum: %u\n", ntohs(reply->checksum));
    printf("Identifier: %u\n", ntohs(reply->identifier));
    printf("Sequence: %u\n", ntohs(reply->sequence));

    closesocket(sock);
    WSACleanup();
    return 0;
}

uint16_t icmp_checksum(void *data, size_t length) {

    uint32_t sum = 0;
    uint8_t *bytes = (uint8_t *)data;

    // Iterates over the bytes to sum the package fields
    for(size_t i = 0; i + 1 < length; i += 2) {
        sum += ((uint16_t)bytes[i] << 8) | bytes[i + 1];
    }

    // Add the solitary byte to the sum
    if(length % 2 != 0) {
        sum += (uint16_t)bytes[length - 1] << 8;
    }

    // End-Around Carry
    while(sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)~sum;
}