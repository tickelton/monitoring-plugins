#include "unity.h"
#include "check_twamp-light.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_ntp_to_timeval_tv_should_be_0_at_01_01_1900(void)
{
    struct ntp_ts_t ntp_ts;
    struct timeval tv;

    ntp_ts.seconds = 2208988800;
    ntp_ts.fraction = 0;

    ntp_to_timeval(&ntp_ts, &tv);

    TEST_ASSERT_EQUAL_UINT32(tv.tv_sec, 0);
    TEST_ASSERT_EQUAL_UINT32(tv.tv_usec, 0);
}


int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ntp_to_timeval_tv_should_be_0_at_01_01_1900);
    return UNITY_END();
}
