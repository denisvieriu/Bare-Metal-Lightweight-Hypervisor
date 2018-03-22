#include "cmd.h"
#include "time.h"
#include "dbglog.h"


static void CmdDateTime(UINT argc, const char **argv)
{
    char buf[TIME_STRING_SIZE];

    DateTime dt;
    SplitTime(&dt, 0, 0);
    FormatTime(buf, sizeof(buf), &dt);

    LOG("%s\n", buf);
}