#include <stdio.h>
#include <time.h>
#include "logger.h"

void log_event(char *event, char *user, int option, char *ip)
{
    FILE *f = fopen("logs.db", "a");

    if (!f)
    {
        printf("Error opening logs.db\n");
        return;
    }

    time_t t = time(NULL);

    if (option == -1)
    {
        // login / disconnect
        fprintf(f, "%s: %s [IP: %s] - %s",
                event, user, ip, ctime(&t));
    }
    else
    {
        // vote
        fprintf(f, "%s: %s (option %d) [IP: %s] - %s",
                event, user, option, ip, ctime(&t));
    }

    fclose(f);
}