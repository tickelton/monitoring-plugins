/*
 * Copyright 2021, tickelton@gmail.com
 * SPDX-License-Identifier: MIT
 */

#include "check_twamp-light.h"

void ntp_to_timeval(struct ntp_ts_t *ntp, struct timeval *tv) {
  tv->tv_sec = ntp->seconds - TIMESTAMP_OFFSET_1900;
  tv->tv_usec = (uint32_t)((double)ntp->fraction * 1.0e6 / (double)(1LL << 32));
}

void timeval_to_ntp(struct timeval *tv, struct ntp_ts_t *ntp) {
  ntp->seconds = tv->tv_sec + TIMESTAMP_OFFSET_1900;
  ntp->fraction =
      (uint32_t)((double)(tv->tv_usec + 1) * (double)(1LL << 32) * 1.0e-6);
}
