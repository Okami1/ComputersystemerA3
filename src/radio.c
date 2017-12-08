/*
 * radio.c
 *
 * Emulation of radio node using UDP (skeleton)
 */

// Implements
#include "../inc/radio.h"

// Uses
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#define BAUD_RATE 19200

int sock;    // UDP Socket used by this node

int radio_init(int addr) {
  struct sockaddr_in sa;   // Structure to set own address
	bzero((char *) &sa, sizeof(sa));
    // Check validity of address

	if(!(addr >= 1024 && addr<=0xFFFF))
	{
    perror("The address wasn't valid.");
		return ERR_INVAL;
	}
  // Create UDP socket
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
    perror("Failed to create UDP socket.");
		return ERR_FAILED;
	}

    // Prepare address structure
	sa.sin_family = AF_INET;
	sa.sin_port = htons(addr);
	//sa.sin_addr.s_addr = htonl(addr);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to port
	if(bind(sock, (struct sockaddr *) &sa, sizeof(sa)) == -1)
	{
    perror("Failed to bind socket to port.");
		return ERR_FAILED;
	}
    return ERR_OK;
}

int radio_send(int  dst, char* data, int len) {

    struct sockaddr_in sa;   // Structure to hold destination address
	bzero((char *) &sa, sizeof(sa));

    // Check that port and len are valid
	if(!(dst >= 1024 && dst<=0xFFFF) && len >= 0 && len <= FRAME_PAYLOAD_SIZE)
	{
    perror("The port and length wasn't valid.");
		return ERR_INVAL;
	}

    // Emulate transmission time
	usleep((INT_MAX*len)/BAUD_RATE);

    // Prepare address structure
	sa.sin_family = AF_INET;
	sa.sin_port = htons(dst);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

    // Send the message
  int sentLength = sendto(sock, data, len+1, 0 , (struct sockaddr *) &sa, sizeof(sa));
	if (sentLength ==-1)
	{
    perror("The message wasn't sent.");
		return ERR_FAILED;
	}

  // Check if fully sent
  if(sentLength != len+1)
  {
    perror("The message wasn't complete.");
    return ERR_FAILED;
  }


    return ERR_OK;
}

int radio_recv(int* src, char* data, int to_ms) {

    struct sockaddr_in sa; // Structure to receive source address
    socklen_t addrlen = sizeof(sa);
    struct pollfd ufds;
    ufds.fd = sock;
    ufds.events = POLLIN;
    // First poll/select with timeout (may be skipped at first)
    int err;

    if((err = poll(&ufds, 1, to_ms)) < 1)
    {
      if(err < 0)
      {
        perror("The poll failed.");
        return ERR_FAILED;
      }
      perror("Poll timeout.");
      return ERR_TIMEOUT;
    }

    // Then get the packet

    // Zero out the address structure
    memset((char *) &sa,0, sizeof(sa));

    // Receive data
    int recLength = recvfrom(sock, data, FRAME_PAYLOAD_SIZE, 0, (struct sockaddr *) &sa, &addrlen);
    if(recLength == -1)
    {
      perror("The data wasn't received.");
       return ERR_FAILED;
    }

    // Set source from address structure
    *src = ntohs(sa.sin_port);

    //printf("Received length: %d\n", recLength);

    return recLength;
}
