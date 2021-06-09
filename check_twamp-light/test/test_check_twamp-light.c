/*
 * Copyright 2021, tickelton@gmail.com
 * SPDX-License-Identifier: MIT
 */

#include "check_twamp-light.h"
#include "unity.h"

#define OFFSET_1900_1970 2208988800

void setUp(void) {}

void tearDown(void) {}

void test_ntp_to_timeval_tv_should_be_0_at_01_01_1900(void) {
  struct ntp_ts_t ntp_ts;
  struct timeval tv;

  ntp_ts.seconds = OFFSET_1900_1970;
  ntp_ts.fraction = 0;

  ntp_to_timeval(&ntp_ts, &tv);

  TEST_ASSERT_EQUAL_UINT32(0, tv.tv_sec);
  TEST_ASSERT_EQUAL_UINT32(0, tv.tv_usec);
}

void test_ntp_to_timeval_usecs_should_be_500ms(void) {
  struct ntp_ts_t ntp_ts;
  struct timeval tv;

  ntp_ts.seconds = OFFSET_1900_1970;
  ntp_ts.fraction = (1LL << 32) / 2;

  ntp_to_timeval(&ntp_ts, &tv);

  TEST_ASSERT_EQUAL_UINT32(0, tv.tv_sec);
  TEST_ASSERT_EQUAL_UINT32(500000, tv.tv_usec);
}

void test_ntp_to_timeval_usecs_should_be_250ms(void) {
  struct ntp_ts_t ntp_ts;
  struct timeval tv;

  ntp_ts.seconds = OFFSET_1900_1970;
  ntp_ts.fraction = (1LL << 32) / 4;

  ntp_to_timeval(&ntp_ts, &tv);

  TEST_ASSERT_EQUAL_UINT32(0, tv.tv_sec);
  TEST_ASSERT_EQUAL_UINT32(250000, tv.tv_usec);
}

void test_timeval_to_ntp_tv_should_be_01_01_1970(void) {
  struct ntp_ts_t ntp_ts;
  struct timeval tv;

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  timeval_to_ntp(&tv, &ntp_ts);

  TEST_ASSERT_EQUAL_UINT32(OFFSET_1900_1970, ntp_ts.seconds);
  TEST_ASSERT_EQUAL_UINT32(4294, ntp_ts.fraction);
}

void test_timeval_to_ntp_tv_fraction_should_be_500ms(void) {
  struct ntp_ts_t ntp_ts;
  struct timeval tv;

  tv.tv_sec = 0;
  tv.tv_usec = 500000;
  timeval_to_ntp(&tv, &ntp_ts);

  TEST_ASSERT_EQUAL_UINT32(OFFSET_1900_1970, ntp_ts.seconds);
  TEST_ASSERT_EQUAL_UINT32(2147487942, ntp_ts.fraction);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ntp_to_timeval_tv_should_be_0_at_01_01_1900);
  RUN_TEST(test_ntp_to_timeval_usecs_should_be_500ms);
  RUN_TEST(test_ntp_to_timeval_usecs_should_be_250ms);
  RUN_TEST(test_timeval_to_ntp_tv_should_be_01_01_1970);
  RUN_TEST(test_timeval_to_ntp_tv_fraction_should_be_500ms);
  return UNITY_END();
}
