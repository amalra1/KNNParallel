#include <time.h>

typedef struct
{
    struct timespec xadd_time1, xadd_time2;
    long long xtotal_ns;
    long xn_events;

} chronometer_t;

void chrono_reset(chronometer_t *chrono);

void chrono_start(chronometer_t *chrono);

long long chrono_gettotal(chronometer_t *chrono);

long long chrono_getcount(chronometer_t *chrono);

void chrono_stop(chronometer_t *chrono);

void chrono_reportTime(chronometer_t *chrono, char *s);

void chrono_report_TimeInLoop(chronometer_t *chrono, char *s, int loop_count);
