/* Test bidirectional communication between 2 nodes */

#if defined(ARDUINO_ARCH_ESP32)
#include <HardwareSerial.h>
#else
#include <SoftwareSerial.h>
#endif

#include <hc12.h>

#define PINGER      1
#define PONGER      2

/* set one device as PINGER and the other as PONGER */
#define ROLE        PINGER

#if defined(ARDUINO_ARCH_ESP32)
HardwareSerial rfport(1);
#else
SoftwareSerial rfport(4, 5);
#endif

HC12 hc12(&rfport);
uint8_t ping_addr = 1;
uint8_t pong_addr = 2;

void setup(void) {
    Serial.begin(9600);
    
    #if defined(ARDUINO_ARCH_ESP32)
    rfport.begin(9600, SERIAL_8N1, 16, 17);
    #else
    rfport.begin(9600);
    #endif
    
    #if (ROLE == PINGER)
    hc12.begin(ping_addr);
    #else
    hc12.begin(pong_addr);
    #endif

    Serial.println("Starting up.");
}

char ping_payload[] = "Knock knock.";
char pong_payload[] = "Who's there?";

char buffer[HC12_MAX_PACKET_LEN];

#define PING_INTERVAL       1000
uint32_t last_ping = 0;

void loop(void) {
    #if (ROLE == PINGER)
    
    if (millis() - last_ping >= PING_INTERVAL) {
        /* send ping */
        hc12.send(ping_payload, strlen(ping_payload), pong_addr);
        last_ping = millis();
    }
    
    /* read and print anything we've recved */
    uint8_t src;
    int16_t rlen = hc12.recv(buffer, HC12_MAX_PACKET_LEN, &src);
    if (rlen > 0) {
        Serial.print("From "); Serial.print(src); Serial.print(": ");
        Serial.write((uint8_t *)buffer, rlen);
        Serial.println();
    }
    
    #else
    
    /* read and print anything we've recved */
    uint8_t src;
    int16_t rlen = hc12.recv(buffer, HC12_MAX_PACKET_LEN, &src);
    if (rlen > 0) {
        Serial.print("From "); Serial.print(src); Serial.print(": ");
        Serial.write((uint8_t *)buffer, rlen);
        Serial.println();
        
        /* send pong response */
        hc12.send(pong_payload, strlen(pong_payload), src);
    }
    
    #endif
}
