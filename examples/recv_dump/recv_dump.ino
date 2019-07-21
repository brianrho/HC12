#if defined(ARDUINO_ARCH_ESP32)
#include <HardwareSerial.h>
#else
#include <SoftwareSerial.h>
#endif

#include <hc12.h>

#if defined(ARDUINO_ARCH_ESP32)
HardwareSerial rfport(1);
#else
SoftwareSerial rfport(4, 5);
#endif

HC12 hc12(&rfport);
uint8_t own_addr = 10;

void setup(void) {
    Serial.begin(9600);
    
    #if defined(ARDUINO_ARCH_ESP32)
    rfport.begin(9600, SERIAL_8N1, 16, 17);
    #else
    rfport.begin(9600);
    #endif
    
    hc12.begin(own_addr);
    
    Serial.println("Starting up.");
}

char buffer[HC12_MAX_PACKET_LEN];

void loop(void) {    
    /* read and print anything we've recved */
    uint8_t src; 
    int16_t rlen = hc12.recv(buffer, HC12_MAX_PACKET_LEN, &src);
    
    if (rlen > 0) {
        Serial.print("From "); Serial.print(src); Serial.print(": ");
        
        for (int i = 0; i < rlen; i++) {
        
            /* try to print printable chars */
            if (isprint(buffer[i])) {
                Serial.write(buffer[i]);
            }
            else {
                Serial.print(buffer[i], HEX);
            }
            
            Serial.print(" ");
        }
        
        Serial.println();
    }
}
