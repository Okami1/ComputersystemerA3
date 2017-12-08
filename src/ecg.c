/*
 * ecg.c
 *
 * Simple secure message passing protocol using radio device
 */

// Implements
#include "ecg.h"

// Uses
#include "radio.h"
#include "alarm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define RTT_MS  200

#define KEY1 0x9A
#define KEY2 0xB8

// tags
#define REQ 0
#define DATA 1
#define ACK  2

// PDU structures
typedef struct {char tag; } tag_t;

typedef struct {
    tag_t type;
    int   seal;
    int  count;
} req_pdu_t;

typedef struct {
    tag_t type;
    int   seal;
    char  str[0];
} data_pdu_t;

typedef struct {
    tag_t type;
    int   seal;
} ack_pdu_t;

typedef union {
    char raw[FRAME_PAYLOAD_SIZE];

    tag_t        pdu_type;
    req_pdu_t    req;
    data_pdu_t   data;
    ack_pdu_t    ack;
} pdu_frame_t;

#define DATA_PAYLOAD_SIZE (FRAME_PAYLOAD_SIZE - sizeof(data_pdu_t)-1)

int fingerprint(char * str, int key) {
    char sum = strlen(str);
    char * s;
    for (s = str; *s != 0; s++ ){ sum ^= *s; }
    return ((int) sum) ^ key;
}

int ecg_init(int addr)
{
    return radio_init(addr);
}

int ecg_send(int dst, char* packet, int len, int to_ms)
{
    pdu_frame_t buf;
    int err, done, src, count, time_left;
    while ( (err=radio_recv(&src, buf.raw, 0)) >= ERR_OK) {
        printf("Flushing pending frame of size %d\n", err);
    };
    if (err != ERR_TIMEOUT) { return ERR_FAILED; }

    alarm_t timer1;
    alarm_init(&timer1);
    alarm_set(&timer1, to_ms);
    char msg[len];
    done = 0;
    strcpy(msg, packet);
    alarm_set(&timer1, to_ms);
    while(!done && !alarm_expired(&timer1))
    {
        memset((char *) &buf,0, sizeof(buf));
        buf.pdu_type.tag = REQ;
        buf.req.seal = 3;
        if(len % DATA_PAYLOAD_SIZE == 0)
        {
          count = len/DATA_PAYLOAD_SIZE;
        }
        else
        {
           count = len/DATA_PAYLOAD_SIZE + 1;
        }
        buf.req.count = count;
        if((err=radio_send(dst, buf.raw, sizeof(req_pdu_t))) == -1)
        {
          perror("Request send failed.");
          return ERR_FAILED;
        }
        while(1)
        {
            memset((char *) &buf,0, sizeof(buf));
            time_left = alarm_rem(&timer1);
            if (time_left > RTT_MS) { time_left = RTT_MS; }

            err = radio_recv(&src, buf.raw, time_left);
            if (err>=ERR_OK) {
                // Somehting received -- check if expected acknowledgement
                if (err < sizeof(ack_pdu_t) || buf.pdu_type.tag != ACK) {
                    // Not an ACK packet -- ignore
                    printf("Non-ACK packcet with length %d received\n", err);
                    continue;
                }

                // Check sender
                if (src != dst) {
                    printf("Wrong sender: %d\n", src);
                    continue;
                };

                // Check fingerprint
                if (buf.ack.seal != 4) {
                    printf("Wrong fingerprint: 0x%08x\n", buf.ack.seal);
                    continue;
                };

                // ACK ok
                done = 1;
                break;
            }
            else if(err==ERR_TIMEOUT)
            {
              break;
            }
        }
    }



    while(count > 0 && !alarm_expired(&timer1))
    {
        memset((char *) &buf,0, sizeof(buf));
        strncpy(buf.data.str, msg, DATA_PAYLOAD_SIZE);
        buf.pdu_type.tag = DATA;
        char senMsg[strlen(buf.data.str)];
        memset(senMsg, 0, strlen(buf.data.str));
        strcpy(senMsg, buf.data.str);
        buf.data.seal = fingerprint(buf.data.str, KEY1);
        done = 0;


        if((err = radio_send(dst, buf.raw, sizeof(data_pdu_t)+strlen(buf.data.str))) == -1)
        {
            perror("Something went wrong.");
            return ERR_FAILED;
        }
        while(!done && !alarm_expired(&timer1))
        {
          memset((char *)&buf, 0, sizeof(buf));
          time_left = alarm_rem(&timer1);
          if (time_left > RTT_MS) { time_left = RTT_MS; }
          err = radio_recv(&src, buf.raw, time_left);
          if (err>=ERR_OK) {

              // Somehting received -- check if expected acknowledgement
              if (err < sizeof(ack_pdu_t) || buf.pdu_type.tag != ACK) {
                  // Not an ACK packet -- ignore
                  printf("Non-ACK packet with length %d received\n", err);
                  continue;
              }

              // Check sender
              if (src != dst) {
                  printf("Wrong sender: %d\n", src);
                  continue;
              };

              // Check fingerprint
              if (buf.ack.seal != fingerprint(senMsg, KEY2)) {
                  printf("Wrong fingerprint: 0x%08x\n", buf.ack.seal);
                  continue;
              };

          }
          else if(err==ERR_TIMEOUT)
          {
            break;
          }
          count--;
          strcpy(msg, msg+DATA_PAYLOAD_SIZE);
          done = 1;
        }
    }


    if(alarm_expired(&timer1))
    {
      return ERR_TIMEOUT;
    }
    return ERR_OK;
}

int ecg_recv(int* src, char* packet, int len, int to_ms)
{
    pdu_frame_t buf;
    int err;
    char msg[80];
    int count, time_left, lastPrint, haveDoneThis;
    haveDoneThis = 0;
    alarm_t timer1;
    alarm_init(&timer1);
    memset((char *)packet, 0, sizeof(packet));
    while (1)
    {
        memset((char *)&buf, 0, sizeof(buf));
        err=radio_recv(src, buf.raw, -1);
        if (err>=ERR_OK) {
            // Somehting received -- check if DATA PDU
            if (err < sizeof(req_pdu_t) || buf.pdu_type.tag != REQ)
            {
                printf("Non-REQ packet with length %d received\n", err);
                continue;
            }
            if (buf.req.seal != 3)
            {
              printf("Wrong fingerprint: 0x%08x\n", buf.req.seal);
              continue;
            }
            count = buf.req.count;
            memset((char *)&buf, 0, sizeof(buf));
            buf.ack.seal = 4;
            buf.pdu_type.tag = ACK;

            // Send acknowledgement to sender
            if ( (err=radio_send(*src, buf.raw, sizeof(ack_pdu_t))) != ERR_OK) {
                printf("radio_send failed with %d\n", err);
                return ERR_FAILED;
            }
            alarm_set(&timer1, to_ms);
                  while(count > 0 && !alarm_expired(&timer1))
                  {
                      memset((char *)&buf, 0, sizeof(buf));

                      err=radio_recv(src, buf.raw, alarm_rem(&timer1));
                      if(err >= ERR_OK)
                      {
                        if (err < sizeof(data_pdu_t) || buf.pdu_type.tag != DATA) {
                            // Not a DATA packet -- ignore
                            printf("Non-DATA packet with length %d received\n", err);
                            continue;
                        }
                        // Check message consistency and fingerprint
                        int expLen = sizeof(data_pdu_t) + strlen(buf.data.str)+1;
                        //printf("len = %d, explen = %d\n", err, expLen);
                        if ( expLen != err) {
                            printf("Length mismatch: %d\n", err);
                            continue;
                        }
                        if (buf.data.seal != fingerprint(buf.data.str, KEY1)) {
                            printf("Wrong fingerprint: 0x%08x\n", buf.data.seal);
                            continue;
                        };
                        char msg[strlen(buf.data.str)];
                        memset(msg, 0, strlen(buf.data.str));
                        strcpy(msg, buf.data.str);
                        if(fingerprint(msg,KEY1) == lastPrint)
                        {
                            count++;

                        }
                        else
                        {
                          lastPrint = fingerprint(msg,KEY1);
                          strcat(packet, msg);
                        
                        }
                        memset((char *)&buf, 0, sizeof(buf));
                        buf.ack.seal = fingerprint(msg, KEY2);
                        buf.pdu_type.tag = ACK;
                        // Send acknowledgement to sender

                            if ( (err=radio_send(*src, buf.raw, sizeof(ack_pdu_t))) != ERR_OK)
                             {
                                printf("radio_send failed with %d\n", err);
                                return ERR_FAILED;
                            }
                            printf("Received message from %d: %s\n", *src, msg);
                            memset(&msg, 0, sizeof(msg));
                            count--;

                      }
                      else if(err == ERR_TIMEOUT)
                      {
                        continue;
                      }

                  }
                  break;
            }

          printf("Unexpected error %d\n", err);
          exit(1);
      }

    /* DONE STATE */
    if(alarm_expired(&timer1))
    {
      return ERR_TIMEOUT;
    }
    return strlen(packet);
}
