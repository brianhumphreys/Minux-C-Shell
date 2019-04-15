#include <stdio.h>
#include <time.h>

int main (void) {
    char buff[20];
    struct tm *sTm;

    time_t now = time (0);
    sTm = gmtime (&now);

    strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
    printf ("%s %s\n", buff, "Event occurred now");

    return 0;
}