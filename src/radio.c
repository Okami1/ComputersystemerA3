/*
 * radio.c
 *
 * Emulation of radio node using UDP (skeleton)
 */

// Implements
#include "radio.h"

// Uses
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int sock;    // UDP Socket used by this node

int radio_init(int addr) {
  struct sockaddr_in sa;   // Structure to set own address
	bzero((char *) &sa, sizeof(sa));
    // Check validity of address

	if(!(addr >= 1024 && addr<=32767))
	{
		return ERR_INVAL;
	}
  // Create UDP socket
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		return ERR_FAILED;
	}
  // struct timeval timeout;
  // timeout.tv_sec = 10;
  // timeout.tv_usec = 0;
  // if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
  // {
  //     return ERR_FAILED;
  // }
    // Prepare address structure
	sa.sin_family = AF_INET;
	sa.sin_port = htons(addr);
	//sa.sin_addr.s_addr = htonl(addr);
	sa.sin_addr.s_addr = INADDR_ANY;

	printf("%d\n", addr);
	printf("%d\n", sa.sin_port);
	printf("%d\n", sa.sin_addr.s_addr);
    // Bind socket to port
	if(bind(sock, (struct sockaddr *) &sa, sizeof(sa)) == -1)
	{
		return ERR_FAILED;
	}
    return ERR_OK;
}

int radio_send(int  dst, char* data, int len) {

    struct sockaddr_in sa;   // Structure to hold destination address
	bzero((char *) &sa, sizeof(sa));
    // Check that port and len are valid
	if(!(dst >= 1024 && dst<=32767) && len >= 0 && len <= 72)
	{
		return ERR_INVAL;
	}
    // Emulate transmission time
	sleep((8*len)/19200);
    // Prepare address structure
	sa.sin_family = AF_INET;
	sa.sin_port = htons(dst);
	//sa.sin_addr.s_addr = htonl(dst);
	sa.sin_addr.s_addr = INADDR_ANY;
    // Send the message
	if (sendto(sock, data, sizeof(data)/sizeof(data[0]) , 0 , (struct sockaddr *) &sa, sizeof(sa))==-1)
	{
		return ERR_FAILED;
	}
    // Check if fully sent

    return ERR_OK;
}

int radio_recv(int* src, char* data, int to_ms) {

    struct sockaddr_in sa; // Structure to receive source address
    socklen_t addrlen = sizeof(sa);
    struct pollfd ufds;
    int len = sizeof(data)/sizeof(data[0]);            // Size of received packet (or error code)

    // First poll/select with timeout (may be skipped at first)
    int err;
    if((err = poll(&ufds, 1, to_ms)) < 1)
    {
      if(err < 0)
      {
        return ERR_FAILED;
      }
      return ERR_TIMEOUT;
    }
    // Then get the packet

    if(recvfrom(sock, data, len, 0, (struct sockaddr *) &sa, &addrlen) == -1)
    {
       return ERR_FAILED;
    }
    // Zero out the address structure
	   //bzero((char *) &sa, sizeof(sa));

    // Receive data

    // Set source from address structure
    // *src = ntohs(sa.sin_port);

    return len;
}
