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

    request.checksum = icmp_checksum(&request, sizeof(request));

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