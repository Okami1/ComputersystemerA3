/*
 * simsec.h
 *
 * Interface to a primitive secure communication protocol
 */

#ifndef _SIMSEC_H_
#define _SIMSEC_H_

#include "errors.h"

// Initialize protocol with local address
int simsec_init(int addr);

// Send msg-string to dst within to_ms 
int simsec_send(int  dst, char* msg, int to_ms);

// Run receive loop (does not normally terminate)
int simsec_receive_loop();

#endif // _SIMSEC_H_
