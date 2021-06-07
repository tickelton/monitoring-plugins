/*
 * Copyright 2021, tickelton@gmail.com
 * SPDX-License-Identifier: MIT
 */

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
#define DEST_ADDR "192.168.178.60"
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

void ntp_to_timeval(struct ntp_ts_t *ntp, struct timeval *tv) {
  tv->tv_sec = ntp->seconds - TIMESTAMP_OFFSET_1900;
  tv->tv_usec = (uint32_t)((double)ntp->fraction * 1.0e6 / (double)(1LL << 32));
}

void usage(char *cmd_name) {
  printf("Usage: %s [-h] [-V] [-p PORT] HOST\n", cmd_name);
  printf("  Arguments:\n");
  printf("    HOST    : Destination host\n");
  printf("\n  Options:\n");
  printf("    -h      : Show help message\n");
  printf("    -V      : Show version information\n");
  printf("    -p PORT : Set destination port\n");
}

void version() {
  printf("%s %s\n", PROGRAM_NAME, VERSION);
  printf("Copyright (C) 2021 tickelton@gmail.com\n");
  printf("Licence: MIT\n");
}

int main(int argc, char **argv) {
  int opt;
  int dest_port = DEFAULT_PORT;
  char check_host[MAX_HOSTNAME];
  memset(check_host, 0, MAX_HOSTNAME);
  while ((opt = getopt(argc, argv, "hp:V")) !=
         -1) {  // get option from the getopt() method
    switch (opt) {
      case 'h':
        usage(argv[0]);
        return 0;
        break;
      case 'p':
        dest_port = atoi(optarg);
        break;
      case 'V':
        version();
        return 0;
        break;
      case ':':
        usage(argv[0]);
        return 1;
        break;
      case '?':
        usage(argv[0]);
        return 1;
        break;
    }
  }
  if (optind != argc - 1) {
    usage(argv[0]);
    return 1;
  } else {
    strncpy(check_host, argv[optind], MAX_HOSTNAME);
    check_host[MAX_HOSTNAME - 1] = '\0';
  }

  if (dest_port < 0 || dest_port > 65536) {
    printf("Invalid port: %d\n", dest_port);
    return 1;
  }

  struct hostent *host;
  if ((host = gethostbyname(check_host)) == NULL) {
    perror("gethostbyname");
    return 1;
  }

  if (host->h_addrtype != AF_INET) {
    printf("Unexpected address type: %d\n", host->h_addrtype);
    return 1;
  }
  if (host->h_addr_list[0] == 0) {
    printf("Unable to get address for %s\n", check_host);
    return 1;
  }

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("snocket()");
    exit(1);
  }

  struct timeval tv_timeout = {.tv_sec = RECEIVE_TIMEOUT, .tv_usec = 0};
  if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout,
                       sizeof(tv_timeout))) {
    perror("setsockopt");
  }

  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));

  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(dest_port);
  saddr.sin_addr.s_addr = *(long *)(host->h_addr_list)[0];

  char buffer[BUFSIZE];
  memset(buffer, 0, BUFSIZE);

  uint16_t shortVal = htons(1);
  memcpy(buffer + 12, &shortVal, 2);
  uint8_t ttl = 255;
  memcpy(buffer + 16, &ttl, 1);

  // TODO: add padding

  // get send timestamp
  struct timeval tv;
  struct ntp_ts_t ntp_ts;
  gettimeofday(&tv, NULL);

  // convert unix timestamp to NTP timestamp
  ntp_ts.seconds = tv.tv_sec + TIMESTAMP_OFFSET_1900;
  ntp_ts.fraction =
      (uint32_t)((double)(tv.tv_usec + 1) * (double)(1LL << 32) * 1.0e-6);

  uint32_t longVal = htonl(ntp_ts.seconds);
  memcpy(buffer + 4, &longVal, 4);
  longVal = htonl(ntp_ts.fraction);
  memcpy(buffer + 8, &longVal, 4);

  sendto(sockfd, (const char *)buffer, MIN_PACKET_LENGTH, MSG_CONFIRM,
         (const struct sockaddr *)&saddr, sizeof(saddr));

  int n;
  socklen_t len;
  n = recvfrom(sockfd, (char *)buffer, BUFSIZE, MSG_WAITALL,
               (struct sockaddr *)&saddr, &len);

  double inbound_ms = 0;
  double outbound_ms = 0;
  if (n >= MIN_PACKET_LENGTH) {
    // get receive timestamp
    gettimeofday(&tv, NULL);
    close(sockfd);

    struct ntp_ts_t ntp_ts_sender;
    ntp_ts_sender.seconds = ntohl(*((int32_t *)(buffer) + 7));
    ntp_ts_sender.fraction = ntohl(*((int32_t *)(buffer) + 8));
    struct ntp_ts_t ntp_ts_reflector_receive;
    ntp_ts_reflector_receive.seconds = ntohl(*((int32_t *)(buffer) + 4));
    ntp_ts_reflector_receive.fraction = ntohl(*((int32_t *)(buffer) + 5));
    struct ntp_ts_t ntp_ts_reflector_send;
    ntp_ts_reflector_send.seconds = ntohl(*((int32_t *)(buffer) + 1));
    ntp_ts_reflector_send.fraction = ntohl(*((int32_t *)(buffer) + 2));

    struct timeval tv_sender;
    struct timeval tv_reflector_receive;
    struct timeval tv_reflector_send;
    ntp_to_timeval(&ntp_ts_sender, &tv_sender);
    ntp_to_timeval(&ntp_ts_reflector_receive, &tv_reflector_receive);
    ntp_to_timeval(&ntp_ts_reflector_send, &tv_reflector_send);

    outbound_ms = ((tv_reflector_receive.tv_sec - tv_sender.tv_sec) * 1000) +
                  ((tv_reflector_receive.tv_usec - tv_sender.tv_usec) / 1000.0);
    inbound_ms = ((tv.tv_sec - tv_reflector_send.tv_sec) * 1000) +
                 ((tv.tv_usec - tv_reflector_send.tv_usec) / 1000.0);
    if (outbound_ms < 0) {
      outbound_ms = 0;
    }
    if (inbound_ms < 0) {
      inbound_ms = 0;
    }
  }

  printf("%.2f\n%.2f\n%s\n\n", outbound_ms, inbound_ms, check_host);

  return 0;
}
