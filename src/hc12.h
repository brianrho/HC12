/* Written by Brian Ejike (2019)
 * DIstributed under the MIT License */
 
#ifndef HC12_H_
#define HC12_H_

#include <stdint.h>
#include <stddef.h>

/* must be less than the size of the UART RX ring buffer */
#define HC12_MAX_PACKET_LEN         32

#define HC12_BROADCAST_ADDRESS      0xFF


/* only used internally */
typedef enum {
    HC12_STATE_READ_HEADER,
    HC12_STATE_READ_INFO,
    HC12_STATE_READ_PAYLOAD,
} hc12_state_e;


class Stream;


class HC12 {
    public:
        HC12(Stream * port);
        bool begin(uint8_t address);
        uint8_t send(const void * data, uint8_t len, uint8_t dest = 0x00);
        uint8_t broadcast(const void * data, uint8_t len);
        
        /* to recv all packets and let you sort it out */
        int16_t irecv(void * data, uint8_t len, uint8_t * src = NULL, uint8_t * dest = NULL);
        
        /* to recv only packets intended for you and broadcasts */
        int16_t recv(void * data, uint8_t len, uint8_t * src = NULL);
        
        uint8_t get_address(void);
        
        /* reset internal read FSM */
        void reset(void);
        
    private:
        Stream * port;
        uint8_t own_addr;
        
        /* used for the read FSM */
        hc12_state_e state;
        uint8_t to_read;
        uint8_t pkt_src;
        uint8_t pkt_dest;
};

#endif
