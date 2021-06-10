/*
 * Copyright 2021, tickelton@gmail.com
 * SPDX-License-Identifier: MIT
 */

#include "check_twamp-light.h"

static void usage(char *cmd_name) {
  printf("Usage: %s [-h] [-V] [-p PORT] HOST\n", cmd_name);
  printf("  Arguments:\n");
  printf("    HOST    : Destination host\n");
  printf("\n  Options:\n");
  printf("    -h      : Show help message\n");
  printf("    -V      : Show version information\n");
  printf("    -p PORT : Set destination port\n");
}

static void version() {
  printf("%s %s\n", PROGRAM_NAME, VERSION);
  printf("Copyright (C) 2021 tickelton@gmail.com\n");
  printf("Licence: MIT\n");
}

int main(int argc, char **argv) {
  int opt;
  int dest_port = DEFAULT_PORT;
  char check_host[MAX_HOSTNAME];
  memset(check_host, 0, MAX_HOSTNAME);
  while ((opt = getopt(argc, argv, "hp:V")) != -1) {
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
      case '?':
        usage(argv[0]);
        return 1;
        break;
    }
  }
  if (optind != argc - 1) {
    usage(argv[0]);
    return 1;
  }

  strncpy(check_host, argv[optind], MAX_HOSTNAME);
  check_host[MAX_HOSTNAME - 1] = '\0';

  if (dest_port < 0 || dest_port > 65536) {
    printf("Invalid port: %d\n", dest_port);
    return 1;
  }

  /* resolve target hostname */
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

  /* create socket */
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket()");
    exit(1);
  }

  /* set receive timeout */
  struct timeval tv_timeout = {.tv_sec = RECEIVE_TIMEOUT, .tv_usec = 0};
  if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout,
                       sizeof(tv_timeout))) {
    perror("setsockopt");
  }

  /* set destination host and port */
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(dest_port);
  saddr.sin_addr.s_addr = *(long *)(host->h_addr_list)[0];

  char buffer[BUFSIZE];
  memset(buffer, 0, BUFSIZE);

  /* set constant fields of TWAMP packet */
  uint16_t shortVal = htons(1);
  memcpy(buffer + 12, &shortVal, 2); /* error estimate */
  uint8_t ttl = 255;
  memcpy(buffer + 16, &ttl, 1); /* TTL */

  // TODO: add padding

  /* get send timestamp */
  struct timeval tv;
  struct ntp_ts_t ntp_ts;
  gettimeofday(&tv, NULL);

  /* convert and set timestamp */
  timeval_to_ntp(&tv, &ntp_ts);
  uint32_t longVal = htonl(ntp_ts.seconds);
  memcpy(buffer + 4, &longVal, 4);
  longVal = htonl(ntp_ts.fraction);
  memcpy(buffer + 8, &longVal, 4);

  /* send request packet */
  sendto(sockfd, (const char *)buffer, MIN_PACKET_LENGTH, MSG_CONFIRM,
         (const struct sockaddr *)&saddr, sizeof(saddr));

  /* receive response packet */
  int n;
  socklen_t len;
  n = recvfrom(sockfd, (char *)buffer, BUFSIZE, MSG_WAITALL,
               (struct sockaddr *)&saddr, &len);

  /* parse response packet */
  double inbound_ms = 0;
  double outbound_ms = 0;
  if (n >= MIN_PACKET_LENGTH) {
    /* get local receive timestamp */
    gettimeofday(&tv, NULL);
    close(sockfd);

    /* get timestamps from packet */
    struct ntp_ts_t ntp_ts_sender;
    ntp_ts_sender.seconds = ntohl(*((int32_t *)(buffer) + 7));
    ntp_ts_sender.fraction = ntohl(*((int32_t *)(buffer) + 8));
    struct ntp_ts_t ntp_ts_reflector_receive;
    ntp_ts_reflector_receive.seconds = ntohl(*((int32_t *)(buffer) + 4));
    ntp_ts_reflector_receive.fraction = ntohl(*((int32_t *)(buffer) + 5));
    struct ntp_ts_t ntp_ts_reflector_send;
    ntp_ts_reflector_send.seconds = ntohl(*((int32_t *)(buffer) + 1));
    ntp_ts_reflector_send.fraction = ntohl(*((int32_t *)(buffer) + 2));

    /* convert timestamps */
    struct timeval tv_sender;
    struct timeval tv_reflector_receive;
    struct timeval tv_reflector_send;
    ntp_to_timeval(&ntp_ts_sender, &tv_sender);
    ntp_to_timeval(&ntp_ts_reflector_receive, &tv_reflector_receive);
    ntp_to_timeval(&ntp_ts_reflector_send, &tv_reflector_send);

    /* calculate delays */
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
