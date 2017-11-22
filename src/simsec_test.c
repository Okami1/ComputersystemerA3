/*
 * simsec_test.c
 *
 * Test of simple simsec protocol 
 * Parameters:
 *   -r     Runs as receiver (default)
 *   -s     Runs as sender 
 *   -ra p  Set recevier address to p (port)
 *   -sa p  Set sender address to p (port)
 */


// Uses
#include "simsec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TIMEOUT_SEC        5
#define BUF_SIZE          60

int is_sender = 0;
int snd_addr = 2132;
int rcv_addr = 2135;

void read_args(int argc, char * argv[]) {
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) { is_sender = 0;  continue; }
        if (strcmp(argv[i], "-s") == 0) { is_sender = 1;  continue; }
        if (strcmp(argv[i], "-sa") == 0 && i < argc - 1) {
            snd_addr = atoi(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "-ra") == 0 && i < argc - 1) {
            rcv_addr = atoi(argv[++i]);
            continue;
        }
        printf("Unknown option %s\n", argv[i]);
    }
}

int sender() {
    int err, last;

    char msg[BUF_SIZE];

    printf("Acting as sender with address %d sending to %d\n", snd_addr, rcv_addr);

    if ( (err=simsec_init(snd_addr)) != ERR_OK) {
        printf("Protocol could not be initialized: %d\n", err);
        return 1;
    }

    printf("Protocol node initialized\n");

    while (1) {

        // Get next message from console
        printf("Enter message: ");
        fgets(msg, BUF_SIZE, stdin);
        last = strlen(msg) - 1;
        if (msg[last] == '\n') { msg[last] = '\0'; }  // Drop ending newline

        // Send it securely
        err = simsec_send(rcv_addr, msg, TIMEOUT_SEC * 1000);

        if (err != ERR_OK && err != ERR_TIMEOUT) {
            printf("simsec_send failed with %d\n", err);
            return 1;
        }

        if (err == ERR_TIMEOUT) {
            printf("... timed out\n");
            continue;
        }

        printf("Securely sent: %s\n", msg);
    }

    return 0;
}


int receiver() {
    int err;

    printf("Acting as receiver with address %d\n", rcv_addr);

    if ( (err=simsec_init(rcv_addr)) != ERR_OK) {
        printf("Protocol could not be initialized: %d\n", err);
        return 1;
    }

    printf("Protocol init initialized\n");

    simsec_receive_loop();

    printf("Receive loop ended!\n");
    return 1;

}


int main(int argc, char * argv[]) {

    read_args(argc, argv);

    return is_sender ? sender() :  receiver();

}
