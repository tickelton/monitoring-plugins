#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DEST_PORT 862
#define DEST_ADDR "192.168.178.60"
#define BUFSIZE 4096
#define MIN_PACKET_LENGTH 17
#define TIMESTAMP_OFFSET_1900 2208988800
#define RECEIVE_TIMEOUT 2

struct ntp_ts_t {
  uint32_t seconds;
  uint32_t fraction;
};

void ntp_to_timeval(struct ntp_ts_t *ntp, struct timeval *tv) {
  tv->tv_sec = ntp->seconds - TIMESTAMP_OFFSET_1900;
  tv->tv_usec = (uint32_t)((double)ntp->fraction * 1.0e6 / (double)(1LL << 32));
}

int main() {
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
  saddr.sin_port = htons(DEST_PORT);
  saddr.sin_addr.s_addr = inet_addr(DEST_ADDR);

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

  int inbound_ms = 0;
  int outbound_ms = 0;
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
                  ((tv_reflector_receive.tv_usec - tv_sender.tv_usec) / 1000);
    inbound_ms = ((tv.tv_sec - tv_reflector_send.tv_sec) * 1000) +
                 ((tv.tv_usec - tv_reflector_send.tv_usec) / 1000);
    if (outbound_ms < 0) {
      outbound_ms = 0;
    }
    if (inbound_ms < 0) {
      inbound_ms = 0;
    }
  }

  printf("%d\n%d\n\n\n", outbound_ms, inbound_ms);

  return 0;
}
