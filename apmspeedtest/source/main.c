#include <string.h>
#include <stdio.h>

#include <switch.h>

#define inc asm ( "add %0, %0, #1" : "+r" (i) )

static double get_wall_seconds() {
    uint64_t unixtime;
    timeGetCurrentTime(TimeType_LocalSystemClock, &unixtime);
    double seconds = unixtime;
    return seconds;
}

double dumb_speedtest (void) {
    int nBillions = 20;
    unsigned long int N_one_billion = 1000000000;
    unsigned long int N = (unsigned) nBillions*N_one_billion;
    double startTime = get_wall_seconds();
    unsigned long i = 0;
    while (i < N) {
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
        inc; inc; inc; inc; inc; inc; inc; inc; inc; inc;
    }
    double timeTaken = get_wall_seconds() - startTime;
    double ops_per_second = (double)N / timeTaken;
    return ops_per_second/N_one_billion;
}

int test(uint32_t mode, uint32_t profile, double* vroom) {
    uint32_t perf_conf;
    int fail = 0;

    if (!R_SUCCEEDED(apmSetPerformanceConfiguration(mode, profile)))
        fail = 1;

    apmGetPerformanceConfiguration(mode, &perf_conf);
    if (perf_conf == profile) {
        consoleUpdate(NULL);
        if (vroom)
            *vroom = dumb_speedtest();
    } else {
        fail = 1;
    }
    consoleUpdate(NULL);

    return fail;
}

int testbed(int mode, char* res, double* speeds) {
    int expect_patched[]        = { 0,0,0,0,0,0,0,0,0,0,0,0 };
    int expect_unpatched_port[] = { 0,1,1,1,0,1,0,0,1,1,0,0 };
    int expect_unpatched_dock[] = { 0,0,1,1,0,1,0,0,1,1,0,0 };

    double *speed_inc = speeds;
    res[0] = test(mode, 0x10000, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[1] = test(mode, 0x10001, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[2] = test(mode, 0x10002, speed_inc);
    if (speeds != NULL) speed_inc++;

    res[3] = test(mode, 0x20000, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[4] = test(mode, 0x20001, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[5] = test(mode, 0x20002, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[6] = test(mode, 0x20003, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[7] = test(mode, 0x20004, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[8] = test(mode, 0x20005, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[9] = test(mode, 0x20006, speed_inc);
    if (speeds != NULL) speed_inc++;

    res[10] = test(mode, 0x92220007, speed_inc);
    if (speeds != NULL) speed_inc++;
    res[11] = test(mode, 0x92220008, speed_inc);

    if (!memcmp(res, expect_patched, 12))
        return 0;
    else if (!memcmp(res, expect_unpatched_port, 12))
        return 1;
    else if (!memcmp(res, expect_unpatched_dock, 12))
        return 2;
    else
        return 3; // WTF? SDEV, maybe?
}

int main(int argc, char **argv) {
    char *result_names[] = { "all/patched", "portable", "docked", "unknown/test failed" };
    uint32_t profiles[] = { 0x10000, 0x10001, 0x10002, 0x20000, 0x20001, 0x20002, 0x020003, 0x20004, 0x20005, 0x20006, 0x92220007, 0x92220008 };
    uint32_t orig_prof = 0, mode = 0;
    int cur_prof = 0;
    char res[12];
    double speed[12];

    //Initialize console. Using NULL as the second argument tells the console library to use the internal console structure as current one.
    consoleInit(NULL);
    apmInitialize();

    mode      = !!appletGetOperationMode();
    apmGetPerformanceConfiguration(mode, &orig_prof);
    for (int i=0; i < 12; i++) {
        if (profiles[i] == orig_prof) {
            cur_prof = i;
            break;
        }
    }

    printf("APM Test Program\n");
    consoleUpdate(NULL);

    int result = testbed(mode, res, NULL);
    test(mode, profiles[cur_prof], NULL); // Back to default.

    printf("Profiles supported: %s\n", result_names[result]);
    printf("Up, Down: Change Profile | +: Exit | -: Full test (slow)\n");
    printf("Current profile (will not persist on exit): %X     ", profiles[cur_prof]);

    consoleUpdate(NULL);

    while(appletMainLoop()) {
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();

        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            break; // break in order to return to hbmenu
        } else if (kDown & KEY_MINUS) {
            // Exhaustive test.
            printf("Please wait, this takes a will take 12-ish minutes, we're still alive.\n");
            consoleUpdate(NULL);
            testbed(mode, res, speed);
            test(mode, profiles[cur_prof], NULL); // Back to default.
            for (int i=0; i < 12; i++)
                printf("Profile %08x: %s, %4.2f\n", profiles[i], res[i] ? "ok" : "fail", speed[i]);
        } else if (kDown & KEY_UP) {
            if (cur_prof++ >= 12) cur_prof = 0;
            test(mode, profiles[cur_prof], NULL); // Next.
            printf("\rCurrent profile: %X", profiles[cur_prof]);
        } else if (kDown & KEY_DOWN) {
            if (cur_prof-- < 0) cur_prof = 11;
            test(mode, profiles[cur_prof], NULL); // Prev.
            printf("\rCurrent profile: %X", profiles[cur_prof]);
        }

        svcSleepThread(1000000);

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    apmExit();

    return 0;
}
