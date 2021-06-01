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

struct ntp_ts_t {
  uint32_t seconds;
  uint32_t fraction;
};

int main() {
  int sockfd;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket()");
    exit(1);
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
  printf("Sent.\n");

  int n, len;
  n = recvfrom(sockfd, (char *)buffer, BUFSIZE, MSG_WAITALL,
               (struct sockaddr *)&saddr, &len);
  printf("Received.\n");

  close(sockfd);
  return 0;
}
