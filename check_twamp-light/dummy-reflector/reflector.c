/*
 * Copyright 2021, tickelton@gmail.com
 * SPDX-License-Identifier: MIT
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 4096
#define TIMESTAMP_OFFSET_1900 2208988800

struct ntp_ts_t {
  uint32_t seconds;
  uint32_t fraction;
};

static void usage(char *cmd_name) {
  printf("Usage: %s [-h] [-p PORT] [-r US] [-s US]\n", cmd_name);
  printf("\n  Options:\n");
  printf("    -h      : Show help message\n");
  printf("    -p PORT : Set destination port\n");
  printf("    -r US   : Set receive delay to US microseconds\n");
  printf("    -s US   : Set send delay to US microseconds\n");
}

int main(int argc, char **argv) {
  int opt;
  int listen_port = 862;
  int recv_delay = 20000;
  int send_delay = 10000;
  while ((opt = getopt(argc, argv, "hp:r:s:")) != -1) {
    switch (opt) {
      case 'h':
        usage(argv[0]);
        return 0;
        break;
      case 'p':
        listen_port = atoi(optarg);
        break;
      case 'r':
        recv_delay = atoi(optarg);
        break;
      case 's':
        send_delay = atoi(optarg);
        break;
      case ':':
      case '?':
        usage(argv[0]);
        return 1;
        break;
    }
  }

  /* create socket */
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(listen_port);

  /* bind socket */
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind()");
    exit(EXIT_FAILURE);
  }

  unsigned int addrlen = sizeof(cliaddr);
  int n;
  char recv_buffer[BUFSIZE];
  char send_buffer[BUFSIZE];
  uint32_t seq_nr = 0;
  uint16_t error_estimate = 1;
  uint8_t sender_ttl = 255;
  while (1) {
    n = recvfrom(sockfd, (char *)recv_buffer, BUFSIZE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &addrlen);

    if (n < 14) {
      printf("short receiv\n");
      continue;
    }

    /* sleep to simulate network latency */
    usleep(recv_delay);

    /* get receive timestamp */
    struct timeval tv_recv;
    gettimeofday(&tv_recv, NULL);
    struct ntp_ts_t ntp_recv;
    ntp_recv.seconds = tv_recv.tv_sec + TIMESTAMP_OFFSET_1900;
    ntp_recv.fraction = (uint32_t)((double)(tv_recv.tv_usec + 1) *
                                   (double)(1LL << 32) * 1.0e-6);

    memset(send_buffer, 0, sizeof(send_buffer));

    /* Set response sequence number */
    uint32_t longVal = htonl(seq_nr);
    memcpy(send_buffer, &longVal, 4);

    /* Set error estimate */
    memcpy(send_buffer + 12, &error_estimate, 2);

    /* set receive timestamp */
    longVal = htonl(ntp_recv.seconds);
    memcpy(send_buffer + 16, &longVal, 4);
    longVal = htonl(ntp_recv.fraction);
    memcpy(send_buffer + 20, &longVal, 4);

    /* Copy request packet to the appropriate location in the
       response packet. */
    memcpy(send_buffer + 24, recv_buffer, 14);
    memcpy(send_buffer + 40, &sender_ttl, 1);

    /* get send timestamp */
    struct timeval tv_send;
    gettimeofday(&tv_send, NULL);
    struct ntp_ts_t ntp_send;
    ntp_send.seconds = tv_send.tv_sec + TIMESTAMP_OFFSET_1900;
    ntp_send.fraction = (uint32_t)((double)(tv_send.tv_usec + 1) *
                                   (double)(1LL << 32) * 1.0e-6);

    /* set send timestamp */
    longVal = htonl(ntp_send.seconds);
    memcpy(send_buffer + 4, &longVal, 4);
    longVal = htonl(ntp_send.fraction);
    memcpy(send_buffer + 8, &longVal, 4);

    /* sleep to simulate network latency */
    usleep(send_delay);

    /* send response */
    sendto(sockfd, (const char *)send_buffer, 41, MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, addrlen);

    ++seq_nr;
  }

  return 0;
}
