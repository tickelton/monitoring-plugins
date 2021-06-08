/*
 * Copyright 2021, tickelton@gmail.com
 * SPDX-License-Identifier: MIT
 */

#include "check_twamp-light.h"

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
