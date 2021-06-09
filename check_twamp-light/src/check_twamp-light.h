#ifndef _CHECK_TWAMP_LIGHT_H_
#define _CHECK_TWAMP_LIGHT_H_

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_PORT 862
#define BUFSIZE 4096
#define MIN_PACKET_LENGTH 17
#define TIMESTAMP_OFFSET_1900 2208988800
#define RECEIVE_TIMEOUT 2
#define PROGRAM_NAME "check_twamp-light"
#define VERSION "0.0.1"
#define MAX_HOSTNAME 128


struct ntp_ts_t {
  uint32_t seconds;
  uint32_t fraction;
};

void ntp_to_timeval(struct ntp_ts_t *ntp, struct timeval *tv);
void timeval_to_ntp(struct timeval *tv, struct ntp_ts_t *ntp);

#endif /* _CHECK_TWAMP_LIGHT_H_ */
