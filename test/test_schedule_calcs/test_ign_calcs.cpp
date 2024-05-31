#include <Arduino.h>
#include <unity.h>
#include "test_calcs_common.h"
#include "schedule_calcs.h"
#include "crankMaths.h"
#include "decoders.h"
#include "../test_utils.h"

#define _countof(x) (sizeof(x) / sizeof (x[0]))

extern void SetRevolutionTime(uint32_t revTime);

constexpr uint16_t DWELL_TIME_MS = 4;

uint16_t dwellAngle;

void setEngineSpeed(uint16_t rpm, int16_t max_crank) {
    SetRevolutionTime(UDIV_ROUND_CLOSEST(60UL*1000000UL, rpm, uint32_t));
    CRANK_ANGLE_MAX_IGN = max_crank;
    CRANK_ANGLE_MAX_INJ = max_crank;
    dwellAngle = timeToAngleDegPerMicroSec(DWELL_TIME_MS*1000UL);
}

struct ign_test_parameters
{
    uint16_t channelAngle;  // deg
    int8_t advanceAngle;  // deg
    uint16_t crankAngle;    // deg
    uint32_t pending;       // Expected delay when channel status is PENDING
    uint32_t running;       // Expected delay when channel status is RUNNING
    int16_t expectedStartAngle;      // Expected start angle
    int16_t expectedEndAngle;      // Expected end angle
};

static void nullIgnCallback(void) { }

void test_calc_ign_timeout(const ign_test_parameters &test_params)
{
    char msg[150];
    IgnitionSchedule schedule(IGN4_COUNTER, IGN4_COMPARE, nullIgnCallback, nullIgnCallback);

    int startAngle;
    int endAngle;

    calculateIgnitionAngle(dwellAngle, test_params.channelAngle, test_params.advanceAngle, &endAngle, &startAngle);
    TEST_ASSERT_EQUAL_MESSAGE(test_params.expectedStartAngle, startAngle, "startAngle");
    TEST_ASSERT_EQUAL_MESSAGE(test_params.expectedEndAngle, endAngle, "endAngle");
    
    sprintf_P(msg, PSTR("PENDING advanceAngle: %" PRIi8 ", channelAngle: %" PRIu16 ", crankAngle: %" PRIu16 ", endAngle: %" PRIi16), test_params.advanceAngle, test_params.channelAngle, test_params.crankAngle, endAngle);
    schedule.Status = PENDING;
    TEST_ASSERT_INT32_WITHIN_MESSAGE(1, test_params.pending, calculateIgnitionTimeout(schedule, startAngle, test_params.channelAngle,  test_params.crankAngle), msg);
    
    sprintf_P(msg, PSTR("RUNNING advanceAngle: %" PRIi8 ", channelAngle: %" PRIu16 ", crankAngle: %" PRIu16 ", endAngle: %" PRIi16), test_params.advanceAngle, test_params.channelAngle, test_params.crankAngle, endAngle);
    schedule.Status = RUNNING;
    TEST_ASSERT_INT32_WITHIN_MESSAGE(1, test_params.running, calculateIgnitionTimeout(schedule, startAngle, test_params.channelAngle,  test_params.crankAngle), msg);
}

void test_calc_ign_timeout(const ign_test_parameters *pStart, const ign_test_parameters *pEnd)
{
    ign_test_parameters local;
    while (pStart!=pEnd)
    {
        memcpy_P(&local, pStart, sizeof(local));
        test_calc_ign_timeout(local);
        ++pStart;
    }
}

void test_calc_ign_timeout_360()
{
    setEngineSpeed(4000, 360);
    
    TEST_ASSERT_EQUAL(15000, revolutionTime);    
    TEST_ASSERT_EQUAL(96, dwellAngle);

    // Expected test values were generated using floating point calculations (in Excel)
    static const ign_test_parameters test_data[] PROGMEM = {
         // ChannelAngle (deg), Advance, Crank, Expected Pending, Expected Running
        { 0, -40, 0, 12667, 12667, 304, 40 },
        { 0, -40, 45, 10792, 10792, 304, 40 },
        { 0, -40, 90, 8917, 8917, 304, 40 },
        { 0, -40, 135, 7042, 7042, 304, 40 },
        { 0, -40, 180, 5167, 5167, 304, 40 },
        { 0, -40, 215, 3708, 3708, 304, 40 },
        { 0, -40, 270, 1417, 1417, 304, 40 },
        { 0, -40, 315, 0, 14542, 304, 40 },
        { 0, -40, 360, 0, 12667, 304, 40 },
        { 0, 0, 0, 11000, 11000, 264, 360 },
        { 0, 0, 45, 9125, 9125, 264, 360 },
        { 0, 0, 90, 7250, 7250, 264, 360 },
        { 0, 0, 135, 5375, 5375, 264, 360 },
        { 0, 0, 180, 3500, 3500, 264, 360 },
        { 0, 0, 215, 2042, 2042, 264, 360 },
        { 0, 0, 270, 0, 14750, 264, 360 },
        { 0, 0, 315, 0, 12875, 264, 360 },
        { 0, 0, 360, 0, 11000, 264, 360 },
        { 0, 40, 0, 9333, 9333, 224, 320 },
        { 0, 40, 45, 7458, 7458, 224, 320 },
        { 0, 40, 90, 5583, 5583, 224, 320 },
        { 0, 40, 135, 3708, 3708, 224, 320 },
        { 0, 40, 180, 1833, 1833, 224, 320 },
        { 0, 40, 215, 375, 375, 224, 320 },
        { 0, 40, 270, 0, 13083, 224, 320 },
        { 0, 40, 315, 0, 11208, 224, 320 },
        { 0, 40, 360, 0, 9333, 224, 320 },
        { 72, -40, 0, 667, 667, 16, 112 },
        { 72, -40, 45, 0, 13792, 16, 112 },
        { 72, -40, 90, 11917, 11917, 16, 112 },
        { 72, -40, 135, 10042, 10042, 16, 112 },
        { 72, -40, 180, 8167, 8167, 16, 112 },
        { 72, -40, 215, 6708, 6708, 16, 112 },
        { 72, -40, 270, 4417, 4417, 16, 112 },
        { 72, -40, 315, 2542, 2542, 16, 112 },
        { 72, -40, 360, 667, 667, 16, 112 },
        { 72, 0, 0, 0, 14000, 336, 72 },
        { 72, 0, 45, 0, 12125, 336, 72 },
        { 72, 0, 90, 10250, 10250, 336, 72 },
        { 72, 0, 135, 8375, 8375, 336, 72 },
        { 72, 0, 180, 6500, 6500, 336, 72 },
        { 72, 0, 215, 5042, 5042, 336, 72 },
        { 72, 0, 270, 2750, 2750, 336, 72 },
        { 72, 0, 315, 875, 875, 336, 72 },
        { 72, 0, 360, 0, 14000, 336, 72 },
        { 72, 40, 0, 0, 12333, 296, 32 },
        { 72, 40, 45, 0, 10458, 296, 32 },
        { 72, 40, 90, 8583, 8583, 296, 32 },
        { 72, 40, 135, 6708, 6708, 296, 32 },
        { 72, 40, 180, 4833, 4833, 296, 32 },
        { 72, 40, 215, 3375, 3375, 296, 32 },
        { 72, 40, 270, 1083, 1083, 296, 32 },
        { 72, 40, 315, 0, 14208, 296, 32 },
        { 72, 40, 360, 0, 12333, 296, 32 },
        { 90, -40, 0, 1417, 1417, 34, 130 },
        { 90, -40, 45, 0, 14542, 34, 130 },
        { 90, -40, 90, 12667, 12667, 34, 130 },
        { 90, -40, 135, 10792, 10792, 34, 130 },
        { 90, -40, 180, 8917, 8917, 34, 130 },
        { 90, -40, 215, 7458, 7458, 34, 130 },
        { 90, -40, 270, 5167, 5167, 34, 130 },
        { 90, -40, 315, 3292, 3292, 34, 130 },
        { 90, -40, 360, 1417, 1417, 34, 130 },
        { 90, 0, 0, 0, 14750, 354, 90 },
        { 90, 0, 45, 0, 12875, 354, 90 },
        { 90, 0, 90, 11000, 11000, 354, 90 },
        { 90, 0, 135, 9125, 9125, 354, 90 },
        { 90, 0, 180, 7250, 7250, 354, 90 },
        { 90, 0, 215, 5792, 5792, 354, 90 },
        { 90, 0, 270, 3500, 3500, 354, 90 },
        { 90, 0, 315, 1625, 1625, 354, 90 },
        { 90, 0, 360, 0, 14750, 354, 90 },
        { 90, 40, 0, 0, 13083, 314, 50 },
        { 90, 40, 45, 0, 11208, 314, 50 },
        { 90, 40, 90, 9333, 9333, 314, 50 },
        { 90, 40, 135, 7458, 7458, 314, 50 },
        { 90, 40, 180, 5583, 5583, 314, 50 },
        { 90, 40, 215, 4125, 4125, 314, 50 },
        { 90, 40, 270, 1833, 1833, 314, 50 },
        { 90, 40, 315, 0, 14958, 314, 50 },
        { 90, 40, 360, 0, 13083, 314, 50 },
        { 144, -40, 0, 3667, 3667, 88, 184 },
        { 144, -40, 45, 1792, 1792, 88, 184 },
        { 144, -40, 90, 0, 14917, 88, 184 },
        { 144, -40, 135, 0, 13042, 88, 184 },
        { 144, -40, 180, 11167, 11167, 88, 184 },
        { 144, -40, 215, 9708, 9708, 88, 184 },
        { 144, -40, 270, 7417, 7417, 88, 184 },
        { 144, -40, 315, 5542, 5542, 88, 184 },
        { 144, -40, 360, 3667, 3667, 88, 184 },
        { 144, 0, 0, 2000, 2000, 48, 144 },
        { 144, 0, 45, 125, 125, 48, 144 },
        { 144, 0, 90, 0, 13250, 48, 144 },
        { 144, 0, 135, 0, 11375, 48, 144 },
        { 144, 0, 180, 9500, 9500, 48, 144 },
        { 144, 0, 215, 8042, 8042, 48, 144 },
        { 144, 0, 270, 5750, 5750, 48, 144 },
        { 144, 0, 315, 3875, 3875, 48, 144 },
        { 144, 0, 360, 2000, 2000, 48, 144 },
        { 144, 40, 0, 333, 333, 8, 104 },
        { 144, 40, 45, 0, 13458, 8, 104 },
        { 144, 40, 90, 0, 11583, 8, 104 },
        { 144, 40, 135, 0, 9708, 8, 104 },
        { 144, 40, 180, 7833, 7833, 8, 104 },
        { 144, 40, 215, 6375, 6375, 8, 104 },
        { 144, 40, 270, 4083, 4083, 8, 104 },
        { 144, 40, 315, 2208, 2208, 8, 104 },
        { 144, 40, 360, 333, 333, 8, 104 },
        { 180, -40, 0, 5167, 5167, 124, 220 },
        { 180, -40, 45, 3292, 3292, 124, 220 },
        { 180, -40, 90, 1417, 1417, 124, 220 },
        { 180, -40, 135, 0, 14542, 124, 220 },
        { 180, -40, 180, 12667, 12667, 124, 220 },
        { 180, -40, 215, 11208, 11208, 124, 220 },
        { 180, -40, 270, 8917, 8917, 124, 220 },
        { 180, -40, 315, 7042, 7042, 124, 220 },
        { 180, -40, 360, 5167, 5167, 124, 220 },
        { 180, 0, 0, 3500, 3500, 84, 180 },
        { 180, 0, 45, 1625, 1625, 84, 180 },
        { 180, 0, 90, 0, 14750, 84, 180 },
        { 180, 0, 135, 0, 12875, 84, 180 },
        { 180, 0, 180, 11000, 11000, 84, 180 },
        { 180, 0, 215, 9542, 9542, 84, 180 },
        { 180, 0, 270, 7250, 7250, 84, 180 },
        { 180, 0, 315, 5375, 5375, 84, 180 },
        { 180, 0, 360, 3500, 3500, 84, 180 },
        { 180, 40, 0, 1833, 1833, 44, 140 },
        { 180, 40, 45, 0, 14958, 44, 140 },
        { 180, 40, 90, 0, 13083, 44, 140 },
        { 180, 40, 135, 0, 11208, 44, 140 },
        { 180, 40, 180, 9333, 9333, 44, 140 },
        { 180, 40, 215, 7875, 7875, 44, 140 },
        { 180, 40, 270, 5583, 5583, 44, 140 },
        { 180, 40, 315, 3708, 3708, 44, 140 },
        { 180, 40, 360, 1833, 1833, 44, 140 },
        { 270, -40, 0, 8917, 8917, 214, 310 },
        { 270, -40, 45, 7042, 7042, 214, 310 },
        { 270, -40, 90, 5167, 5167, 214, 310 },
        { 270, -40, 135, 3292, 3292, 214, 310 },
        { 270, -40, 180, 1417, 1417, 214, 310 },
        { 270, -40, 215, 0, 14958, 214, 310 },
        { 270, -40, 270, 12667, 12667, 214, 310 },
        { 270, -40, 315, 10792, 10792, 214, 310 },
        { 270, -40, 360, 8917, 8917, 214, 310 },
        { 270, 0, 0, 7250, 7250, 174, 270 },
        { 270, 0, 45, 5375, 5375, 174, 270 },
        { 270, 0, 90, 3500, 3500, 174, 270 },
        { 270, 0, 135, 1625, 1625, 174, 270 },
        { 270, 0, 180, 0, 14750, 174, 270 },
        { 270, 0, 215, 0, 13292, 174, 270 },
        { 270, 0, 270, 11000, 11000, 174, 270 },
        { 270, 0, 315, 9125, 9125, 174, 270 },
        { 270, 0, 360, 7250, 7250, 174, 270 },
        { 270, 40, 0, 5583, 5583, 134, 230 },
        { 270, 40, 45, 3708, 3708, 134, 230 },
        { 270, 40, 90, 1833, 1833, 134, 230 },
        { 270, 40, 135, 0, 14958, 134, 230 },
        { 270, 40, 180, 0, 13083, 134, 230 },
        { 270, 40, 215, 0, 11625, 134, 230 },
        { 270, 40, 270, 9333, 9333, 134, 230 },
        { 270, 40, 315, 7458, 7458, 134, 230 },
        { 270, 40, 360, 5583, 5583, 134, 230 },
        { 360, -40, 0, 12667, 12667, 304, 40 },
        { 360, -40, 45, 10792, 10792, 304, 40 },
        { 360, -40, 90, 8917, 8917, 304, 40 },
        { 360, -40, 135, 7042, 7042, 304, 40 },
        { 360, -40, 180, 5167, 5167, 304, 40 },
        { 360, -40, 215, 3708, 3708, 304, 40 },
        { 360, -40, 270, 1417, 1417, 304, 40 },
        { 360, -40, 315, 0, 14542, 304, 40 },
        { 360, -40, 360, 12667, 12667, 304, 40 },
        { 360, 0, 0, 11000, 11000, 264, 360 },
        { 360, 0, 45, 9125, 9125, 264, 360 },
        { 360, 0, 90, 7250, 7250, 264, 360 },
        { 360, 0, 135, 5375, 5375, 264, 360 },
        { 360, 0, 180, 3500, 3500, 264, 360 },
        { 360, 0, 215, 2042, 2042, 264, 360 },
        { 360, 0, 270, 0, 14750, 264, 360 },
        { 360, 0, 315, 0, 12875, 264, 360 },
        { 360, 0, 360, 11000, 11000, 264, 360 },
        { 360, 40, 0, 9333, 9333, 224, 320 },
        { 360, 40, 45, 7458, 7458, 224, 320 },
        { 360, 40, 90, 5583, 5583, 224, 320 },
        { 360, 40, 135, 3708, 3708, 224, 320 },
        { 360, 40, 180, 1833, 1833, 224, 320 },
        { 360, 40, 215, 375, 375, 224, 320 },
        { 360, 40, 270, 0, 13083, 224, 320 },
        { 360, 40, 315, 0, 11208, 224, 320 },
        { 360, 40, 360, 9333, 9333, 224, 320 },
    };

    test_calc_ign_timeout(&test_data[0], &test_data[0]+_countof(test_data));
}



void test_calc_ign_timeout_720()
{
    setEngineSpeed(4000, 720);

    // Expected test values were generated using floating point calculations (in Excel)
    static const ign_test_parameters test_data[] PROGMEM = {
         // ChannelAngle (deg), Advance, Crank, Expected Pending, Expected Running
        { 0, -40, 0, 27667, 27667, 664, 40 },
        { 0, -40, 45, 25792, 25792, 664, 40 },
        { 0, -40, 90, 23917, 23917, 664, 40 },
        { 0, -40, 135, 22042, 22042, 664, 40 },
        { 0, -40, 180, 20167, 20167, 664, 40 },
        { 0, -40, 215, 18708, 18708, 664, 40 },
        { 0, -40, 270, 16417, 16417, 664, 40 },
        { 0, -40, 315, 14542, 14542, 664, 40 },
        { 0, -40, 360, 12667, 12667, 664, 40 },
        { 0, 0, 0, 26000, 26000, 624, 720 },
        { 0, 0, 45, 24125, 24125, 624, 720 },
        { 0, 0, 90, 22250, 22250, 624, 720 },
        { 0, 0, 135, 20375, 20375, 624, 720 },
        { 0, 0, 180, 18500, 18500, 624, 720 },
        { 0, 0, 215, 17042, 17042, 624, 720 },
        { 0, 0, 270, 14750, 14750, 624, 720 },
        { 0, 0, 315, 12875, 12875, 624, 720 },
        { 0, 0, 360, 11000, 11000, 624, 720 },
        { 0, 40, 0, 24333, 24333, 584, 680 },
        { 0, 40, 45, 22458, 22458, 584, 680 },
        { 0, 40, 90, 20583, 20583, 584, 680 },
        { 0, 40, 135, 18708, 18708, 584, 680 },
        { 0, 40, 180, 16833, 16833, 584, 680 },
        { 0, 40, 215, 15375, 15375, 584, 680 },
        { 0, 40, 270, 13083, 13083, 584, 680 },
        { 0, 40, 315, 11208, 11208, 584, 680 },
        { 0, 40, 360, 9333, 9333, 584, 680 },
        { 72, -40, 0, 667, 667, 16, 112 },
        { 72, -40, 45, 0, 28792, 16, 112 },
        { 72, -40, 90, 26917, 26917, 16, 112 },
        { 72, -40, 135, 25042, 25042, 16, 112 },
        { 72, -40, 180, 23167, 23167, 16, 112 },
        { 72, -40, 215, 21708, 21708, 16, 112 },
        { 72, -40, 270, 19417, 19417, 16, 112 },
        { 72, -40, 315, 17542, 17542, 16, 112 },
        { 72, -40, 360, 15667, 15667, 16, 112 },
        { 72, 0, 0, 0, 29000, 696, 72 },
        { 72, 0, 45, 0, 27125, 696, 72 },
        { 72, 0, 90, 25250, 25250, 696, 72 },
        { 72, 0, 135, 23375, 23375, 696, 72 },
        { 72, 0, 180, 21500, 21500, 696, 72 },
        { 72, 0, 215, 20042, 20042, 696, 72 },
        { 72, 0, 270, 17750, 17750, 696, 72 },
        { 72, 0, 315, 15875, 15875, 696, 72 },
        { 72, 0, 360, 14000, 14000, 696, 72 },
        { 72, 40, 0, 0, 27333, 656, 32 },
        { 72, 40, 45, 0, 25458, 656, 32 },
        { 72, 40, 90, 23583, 23583, 656, 32 },
        { 72, 40, 135, 21708, 21708, 656, 32 },
        { 72, 40, 180, 19833, 19833, 656, 32 },
        { 72, 40, 215, 18375, 18375, 656, 32 },
        { 72, 40, 270, 16083, 16083, 656, 32 },
        { 72, 40, 315, 14208, 14208, 656, 32 },
        { 72, 40, 360, 12333, 12333, 656, 32 },
        { 90, -40, 0, 1417, 1417, 34, 130 },
        { 90, -40, 45, 0, 29542, 34, 130 },
        { 90, -40, 90, 27667, 27667, 34, 130 },
        { 90, -40, 135, 25792, 25792, 34, 130 },
        { 90, -40, 180, 23917, 23917, 34, 130 },
        { 90, -40, 215, 22458, 22458, 34, 130 },
        { 90, -40, 270, 20167, 20167, 34, 130 },
        { 90, -40, 315, 18292, 18292, 34, 130 },
        { 90, -40, 360, 16417, 16417, 34, 130 },
        { 90, 0, 0, 0, 29750, 714, 90 },
        { 90, 0, 45, 0, 27875, 714, 90 },
        { 90, 0, 90, 26000, 26000, 714, 90 },
        { 90, 0, 135, 24125, 24125, 714, 90 },
        { 90, 0, 180, 22250, 22250, 714, 90 },
        { 90, 0, 215, 20792, 20792, 714, 90 },
        { 90, 0, 270, 18500, 18500, 714, 90 },
        { 90, 0, 315, 16625, 16625, 714, 90 },
        { 90, 0, 360, 14750, 14750, 714, 90 },
        { 90, 40, 0, 0, 28083, 674, 50 },
        { 90, 40, 45, 0, 26208, 674, 50 },
        { 90, 40, 90, 24333, 24333, 674, 50 },
        { 90, 40, 135, 22458, 22458, 674, 50 },
        { 90, 40, 180, 20583, 20583, 674, 50 },
        { 90, 40, 215, 19125, 19125, 674, 50 },
        { 90, 40, 270, 16833, 16833, 674, 50 },
        { 90, 40, 315, 14958, 14958, 674, 50 },
        { 90, 40, 360, 13083, 13083, 674, 50 },
        { 144, -40, 0, 3667, 3667, 88, 184 },
        { 144, -40, 45, 1792, 1792, 88, 184 },
        { 144, -40, 90, 0, 29917, 88, 184 },
        { 144, -40, 135, 0, 28042, 88, 184 },
        { 144, -40, 180, 26167, 26167, 88, 184 },
        { 144, -40, 215, 24708, 24708, 88, 184 },
        { 144, -40, 270, 22417, 22417, 88, 184 },
        { 144, -40, 315, 20542, 20542, 88, 184 },
        { 144, -40, 360, 18667, 18667, 88, 184 },
        { 144, 0, 0, 2000, 2000, 48, 144 },
        { 144, 0, 45, 125, 125, 48, 144 },
        { 144, 0, 90, 0, 28250, 48, 144 },
        { 144, 0, 135, 0, 26375, 48, 144 },
        { 144, 0, 180, 24500, 24500, 48, 144 },
        { 144, 0, 215, 23042, 23042, 48, 144 },
        { 144, 0, 270, 20750, 20750, 48, 144 },
        { 144, 0, 315, 18875, 18875, 48, 144 },
        { 144, 0, 360, 17000, 17000, 48, 144 },
        { 144, 40, 0, 333, 333, 8, 104 },
        { 144, 40, 45, 0, 28458, 8, 104 },
        { 144, 40, 90, 0, 26583, 8, 104 },
        { 144, 40, 135, 0, 24708, 8, 104 },
        { 144, 40, 180, 22833, 22833, 8, 104 },
        { 144, 40, 215, 21375, 21375, 8, 104 },
        { 144, 40, 270, 19083, 19083, 8, 104 },
        { 144, 40, 315, 17208, 17208, 8, 104 },
        { 144, 40, 360, 15333, 15333, 8, 104 },
        { 180, -40, 0, 5167, 5167, 124, 220 },
        { 180, -40, 45, 3292, 3292, 124, 220 },
        { 180, -40, 90, 1417, 1417, 124, 220 },
        { 180, -40, 135, 0, 29542, 124, 220 },
        { 180, -40, 180, 27667, 27667, 124, 220 },
        { 180, -40, 215, 26208, 26208, 124, 220 },
        { 180, -40, 270, 23917, 23917, 124, 220 },
        { 180, -40, 315, 22042, 22042, 124, 220 },
        { 180, -40, 360, 20167, 20167, 124, 220 },
        { 180, 0, 0, 3500, 3500, 84, 180 },
        { 180, 0, 45, 1625, 1625, 84, 180 },
        { 180, 0, 90, 0, 29750, 84, 180 },
        { 180, 0, 135, 0, 27875, 84, 180 },
        { 180, 0, 180, 26000, 26000, 84, 180 },
        { 180, 0, 215, 24542, 24542, 84, 180 },
        { 180, 0, 270, 22250, 22250, 84, 180 },
        { 180, 0, 315, 20375, 20375, 84, 180 },
        { 180, 0, 360, 18500, 18500, 84, 180 },
        { 180, 40, 0, 1833, 1833, 44, 140 },
        { 180, 40, 45, 0, 29958, 44, 140 },
        { 180, 40, 90, 0, 28083, 44, 140 },
        { 180, 40, 135, 0, 26208, 44, 140 },
        { 180, 40, 180, 24333, 24333, 44, 140 },
        { 180, 40, 215, 22875, 22875, 44, 140 },
        { 180, 40, 270, 20583, 20583, 44, 140 },
        { 180, 40, 315, 18708, 18708, 44, 140 },
        { 180, 40, 360, 16833, 16833, 44, 140 },
        { 270, -40, 0, 8917, 8917, 214, 310 },
        { 270, -40, 45, 7042, 7042, 214, 310 },
        { 270, -40, 90, 5167, 5167, 214, 310 },
        { 270, -40, 135, 3292, 3292, 214, 310 },
        { 270, -40, 180, 1417, 1417, 214, 310 },
        { 270, -40, 215, 0, 29958, 214, 310 },
        { 270, -40, 270, 27667, 27667, 214, 310 },
        { 270, -40, 315, 25792, 25792, 214, 310 },
        { 270, -40, 360, 23917, 23917, 214, 310 },
        { 270, 0, 0, 7250, 7250, 174, 270 },
        { 270, 0, 45, 5375, 5375, 174, 270 },
        { 270, 0, 90, 3500, 3500, 174, 270 },
        { 270, 0, 135, 1625, 1625, 174, 270 },
        { 270, 0, 180, 0, 29750, 174, 270 },
        { 270, 0, 215, 0, 28292, 174, 270 },
        { 270, 0, 270, 26000, 26000, 174, 270 },
        { 270, 0, 315, 24125, 24125, 174, 270 },
        { 270, 0, 360, 22250, 22250, 174, 270 },
        { 270, 40, 0, 5583, 5583, 134, 230 },
        { 270, 40, 45, 3708, 3708, 134, 230 },
        { 270, 40, 90, 1833, 1833, 134, 230 },
        { 270, 40, 135, 0, 29958, 134, 230 },
        { 270, 40, 180, 0, 28083, 134, 230 },
        { 270, 40, 215, 0, 26625, 134, 230 },
        { 270, 40, 270, 24333, 24333, 134, 230 },
        { 270, 40, 315, 22458, 22458, 134, 230 },
        { 270, 40, 360, 20583, 20583, 134, 230 },
        { 360, -40, 0, 12667, 12667, 304, 400 },
        { 360, -40, 45, 10792, 10792, 304, 400 },
        { 360, -40, 90, 8917, 8917, 304, 400 },
        { 360, -40, 135, 7042, 7042, 304, 400 },
        { 360, -40, 180, 5167, 5167, 304, 400 },
        { 360, -40, 215, 3708, 3708, 304, 400 },
        { 360, -40, 270, 1417, 1417, 304, 400 },
        { 360, -40, 315, 0, 29542, 304, 400 },
        { 360, -40, 360, 27667, 27667, 304, 400 },
        { 360, 0, 0, 11000, 11000, 264, 360 },
        { 360, 0, 45, 9125, 9125, 264, 360 },
        { 360, 0, 90, 7250, 7250, 264, 360 },
        { 360, 0, 135, 5375, 5375, 264, 360 },
        { 360, 0, 180, 3500, 3500, 264, 360 },
        { 360, 0, 215, 2042, 2042, 264, 360 },
        { 360, 0, 270, 0, 29750, 264, 360 },
        { 360, 0, 315, 0, 27875, 264, 360 },
        { 360, 0, 360, 26000, 26000, 264, 360 },
        { 360, 40, 0, 9333, 9333, 224, 320 },
        { 360, 40, 45, 7458, 7458, 224, 320 },
        { 360, 40, 90, 5583, 5583, 224, 320 },
        { 360, 40, 135, 3708, 3708, 224, 320 },
        { 360, 40, 180, 1833, 1833, 224, 320 },
        { 360, 40, 215, 375, 375, 224, 320 },
        { 360, 40, 270, 0, 28083, 224, 320 },
        { 360, 40, 315, 0, 26208, 224, 320 },
        { 360, 40, 360, 24333, 24333, 224, 320 },
        { 432, -40, 0, 15667, 15667, 376, 472 },
        { 432, -40, 45, 13792, 13792, 376, 472 },
        { 432, -40, 90, 11917, 11917, 376, 472 },
        { 432, -40, 135, 10042, 10042, 376, 472 },
        { 432, -40, 180, 8167, 8167, 376, 472 },
        { 432, -40, 215, 6708, 6708, 376, 472 },
        { 432, -40, 270, 4417, 4417, 376, 472 },
        { 432, -40, 315, 2542, 2542, 376, 472 },
        { 432, -40, 360, 667, 667, 376, 472 },
        { 432, 0, 0, 14000, 14000, 336, 432 },
        { 432, 0, 45, 12125, 12125, 336, 432 },
        { 432, 0, 90, 10250, 10250, 336, 432 },
        { 432, 0, 135, 8375, 8375, 336, 432 },
        { 432, 0, 180, 6500, 6500, 336, 432 },
        { 432, 0, 215, 5042, 5042, 336, 432 },
        { 432, 0, 270, 2750, 2750, 336, 432 },
        { 432, 0, 315, 875, 875, 336, 432 },
        { 432, 0, 360, 0, 29000, 336, 432 },
        { 432, 40, 0, 12333, 12333, 296, 392 },
        { 432, 40, 45, 10458, 10458, 296, 392 },
        { 432, 40, 90, 8583, 8583, 296, 392 },
        { 432, 40, 135, 6708, 6708, 296, 392 },
        { 432, 40, 180, 4833, 4833, 296, 392 },
        { 432, 40, 215, 3375, 3375, 296, 392 },
        { 432, 40, 270, 1083, 1083, 296, 392 },
        { 432, 40, 315, 0, 29208, 296, 392 },
        { 432, 40, 360, 0, 27333, 296, 392 },
        { 576, -40, 0, 21667, 21667, 520, 616 },
        { 576, -40, 45, 19792, 19792, 520, 616 },
        { 576, -40, 90, 17917, 17917, 520, 616 },
        { 576, -40, 135, 16042, 16042, 520, 616 },
        { 576, -40, 180, 14167, 14167, 520, 616 },
        { 576, -40, 215, 12708, 12708, 520, 616 },
        { 576, -40, 270, 10417, 10417, 520, 616 },
        { 576, -40, 315, 8542, 8542, 520, 616 },
        { 576, -40, 360, 6667, 6667, 520, 616 },
        { 576, 0, 0, 20000, 20000, 480, 576 },
        { 576, 0, 45, 18125, 18125, 480, 576 },
        { 576, 0, 90, 16250, 16250, 480, 576 },
        { 576, 0, 135, 14375, 14375, 480, 576 },
        { 576, 0, 180, 12500, 12500, 480, 576 },
        { 576, 0, 215, 11042, 11042, 480, 576 },
        { 576, 0, 270, 8750, 8750, 480, 576 },
        { 576, 0, 315, 6875, 6875, 480, 576 },
        { 576, 0, 360, 5000, 5000, 480, 576 },
        { 576, 40, 0, 18333, 18333, 440, 536 },
        { 576, 40, 45, 16458, 16458, 440, 536 },
        { 576, 40, 90, 14583, 14583, 440, 536 },
        { 576, 40, 135, 12708, 12708, 440, 536 },
        { 576, 40, 180, 10833, 10833, 440, 536 },
        { 576, 40, 215, 9375, 9375, 440, 536 },
        { 576, 40, 270, 7083, 7083, 440, 536 },
        { 576, 40, 315, 5208, 5208, 440, 536 },
        { 576, 40, 360, 3333, 3333, 440, 536 },
        { 600, -40, 0, 22667, 22667, 544, 640 },
        { 600, -40, 45, 20792, 20792, 544, 640 },
        { 600, -40, 90, 18917, 18917, 544, 640 },
        { 600, -40, 135, 17042, 17042, 544, 640 },
        { 600, -40, 180, 15167, 15167, 544, 640 },
        { 600, -40, 215, 13708, 13708, 544, 640 },
        { 600, -40, 270, 11417, 11417, 544, 640 },
        { 600, -40, 315, 9542, 9542, 544, 640 },
        { 600, -40, 360, 7667, 7667, 544, 640 },
        { 600, 0, 0, 21000, 21000, 504, 600 },
        { 600, 0, 45, 19125, 19125, 504, 600 },
        { 600, 0, 90, 17250, 17250, 504, 600 },
        { 600, 0, 135, 15375, 15375, 504, 600 },
        { 600, 0, 180, 13500, 13500, 504, 600 },
        { 600, 0, 215, 12042, 12042, 504, 600 },
        { 600, 0, 270, 9750, 9750, 504, 600 },
        { 600, 0, 315, 7875, 7875, 504, 600 },
        { 600, 0, 360, 6000, 6000, 504, 600 },
        { 600, 40, 0, 19333, 19333, 464, 560 },
        { 600, 40, 45, 17458, 17458, 464, 560 },
        { 600, 40, 90, 15583, 15583, 464, 560 },
        { 600, 40, 135, 13708, 13708, 464, 560 },
        { 600, 40, 180, 11833, 11833, 464, 560 },
        { 600, 40, 215, 10375, 10375, 464, 560 },
        { 600, 40, 270, 8083, 8083, 464, 560 },
        { 600, 40, 315, 6208, 6208, 464, 560 },
        { 600, 40, 360, 4333, 4333, 464, 560 },
    };

    test_calc_ign_timeout(&test_data[0], &test_data[0]+_countof(test_data));
}

void test_rotary_channel_calcs(void)
{
    setEngineSpeed(4000, 360);

    static const int test_data[][5] PROGMEM = {
        // End Angle (deg), Dwell Angle, rotary split degrees, expected start angle, expected end angle
        { -40, 5, 0, -40, 315 },
        { -40, 95, 0, -40, 225 },
        { -40, 185, 0, -40, 135 },
        { -40, 275, 0, -40, 45 },
        { -40, 355, 0, -40, -35 },
        { -40, 5, 40, 0, 355 },
        { -40, 95, 40, 0, 265 },
        { -40, 185, 40, 0, 175 },
        { -40, 275, 40, 0, 85 },
        { -40, 355, 40, 0, 5 },
        { 0, 5, 0, 0, 355 },
        { 0, 95, 0, 0, 265 },
        { 0, 185, 0, 0, 175 },
        { 0, 275, 0, 0, 85 },
        { 0, 355, 0, 0, 5 },
        { 0, 5, 40, 40, 35 },
        { 0, 95, 40, 40, 305 },
        { 0, 185, 40, 40, 215 },
        { 0, 275, 40, 40, 125 },
        { 0, 355, 40, 40, 45 },
        { 40, 5, 0, 40, 35 },
        { 40, 95, 0, 40, 305 },
        { 40, 185, 0, 40, 215 },
        { 40, 275, 0, 40, 125 },
        { 40, 355, 0, 40, 45 },
        { 40, 5, 40, 80, 75 },
        { 40, 95, 40, 80, 345 },
        { 40, 185, 40, 80, 255 },
        { 40, 275, 40, 80, 165 },
        { 40, 355, 40, 80, 85 },
    };

    const int (*pStart)[5] = &test_data[0];
    const int (*pEnd)[5] = &test_data[0]+_countof(test_data);

    int endAngle, startAngle;
    int local[5];
    while (pStart!=pEnd)
    {
        memcpy_P(local, pStart, sizeof(local));
        ignition2EndAngle = local[0];
        calculateIgnitionTrailingRotary(local[1], local[2], local[0], &endAngle, &startAngle);
        TEST_ASSERT_EQUAL_MESSAGE(local[3], endAngle, "endAngle");
        TEST_ASSERT_EQUAL_MESSAGE(local[4], startAngle, "startAngle");
        ++pStart;
    } 

}

void test_calc_ign_timeout(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_calc_ign_timeout_360);
    RUN_TEST(test_calc_ign_timeout_720);
    RUN_TEST(test_rotary_channel_calcs);
  }
}