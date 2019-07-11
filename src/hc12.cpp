/* Written by Brian Ejike (2019)
 * DIstributed under the MIT License */
 
#include "hc12.h"
#include <Arduino.h>

#define HC12_PACKET_HEADER      0xAF

HC12::HC12(Stream * port) : 
    port(port), own_addr(0), state(HC12_STATE_READ_HEADER)
{

}

bool HC12::begin(uint8_t address) {
    own_addr = address;
}


uint8_t HC12::send(const void * data, uint8_t len, uint8_t dest) {
    /* no zero-length writes allowed */
    if (len == 0)
        return 0;
    
    port->write(HC12_PACKET_HEADER);
    
    /* calculate how much we'll actually write */
    uint8_t to_write, chksum = 0;
    to_write = (len > HC12_MAX_PACKET_LEN) ? HC12_MAX_PACKET_LEN : len;
    
    /* send src, dest, len */
    port->write(own_addr);
    port->write(dest);
    port->write(to_write);
    
    chksum = (HC12_PACKET_HEADER + own_addr + dest + to_write) & 0xff;
    
    /* calc chksum */
    const uint8_t * ptr = (uint8_t *)data;
    for (uint8_t i = 0; i < to_write; i++) {
        chksum += ptr[i];
    }
    
    /* send data and 2's complete chksum */
    port->write(ptr, to_write);
    port->write(~chksum + 1);

    return to_write;
}

uint8_t HC12::broadcast(const void * data, uint8_t len) {
    return send(data, len, HC12_BROADCAST_ADDRESS);
}

int16_t HC12::irecv(void * data, uint8_t len, uint8_t * src, uint8_t * dest) {
    if (len == 0)
        return 0;
    
    while (true) {  
        yield();
              
        switch (state) {
            case HC12_STATE_READ_HEADER: {
                /* wait till we get the header */
                if (port->available() == 0)
                    return -1;
                
                uint8_t header = port->read();
                if (header != HC12_PACKET_HEADER) {
                    continue;
                }
                
                state = HC12_STATE_READ_INFO;
                break;
            }
            case HC12_STATE_READ_INFO: {
                /* wait till we have src, dest, len */
                if (port->available() < 3)
                    return -1;
                
                pkt_src = port->read();
                pkt_dest = port->read();
                to_read = port->read();
                
                /* make sure its within bounds */
                if (to_read > HC12_MAX_PACKET_LEN) {
                    state = HC12_STATE_READ_HEADER;
                    continue;
                }
                
                state = HC12_STATE_READ_PAYLOAD;
                break;
            }
            case HC12_STATE_READ_PAYLOAD: {
                /* wait till we get the whole payload + chksum */
                if (port->available() < to_read + 1)
                    return -1;
                
                /* if supplied buffer is too small */
                if (len < to_read)
                    return 0;
                
                /* not coming back here again */
                state = HC12_STATE_READ_HEADER;
                
                uint8_t * ptr = (uint8_t *)data;
                port->readBytes(ptr, to_read);
                
                /* calc chksum */
                uint8_t chksum = (HC12_PACKET_HEADER + pkt_src + pkt_dest + to_read) & 0xff;
                for (uint8_t i = 0; i < to_read; i++) {
                    chksum += ptr[i];
                }
                
                /* read chksum */
                chksum += port->read();
                
                /* something went wrong, discard packet */
                if (chksum != 0)
                    continue;
                    
                /* save the addresses and return */
                if (src != NULL)    *src = pkt_src;
                if (dest != NULL)   *dest = pkt_dest;
                
                return to_read;
            }
            default:
                break;
        }
    }
}

int16_t HC12::recv(void * data, uint8_t len, uint8_t * src) {
    uint8_t dest;
    int16_t rlen = irecv(data, len, src, &dest);
    
    /* if we read nothing */
    if (rlen <= 0)
        return rlen;
    
    /* only if its for us or a broadcast */
    if (dest == own_addr || dest == HC12_BROADCAST_ADDRESS)
        return rlen;
}

uint8_t HC12::get_address(void) {
    return own_addr;
}

void HC12::reset(void) {
    state = HC12_STATE_READ_HEADER;
}

