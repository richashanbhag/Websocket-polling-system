#include <stdio.h>
#include <string.h>
#include "database.h"

int check_user(char *user, char *pass)
{
    FILE *f = fopen("users.db", "r");
    char u[50], p[50];

    while (fscanf(f, "%s %s", u, p) != EOF)
    {
        if (!strcmp(u, user) && !strcmp(p, pass))
        {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int check_voted(char *user)
{
    FILE *f = fopen("votes.db", "r");
    char u[50];
    int op;

    while (fscanf(f, "%s %d", u, &op) != EOF)
    {
        if (!strcmp(u, user))
        {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void store_vote(char *user, int option)
{
    FILE *f = fopen("votes.db", "a");
    fprintf(f, "%s %d\n", user, option);
    fclose(f);
}

void get_results(int *a, int *b)
{
    FILE *f = fopen("votes.db", "r");
    char u[50];
    int op;
    *a = *b = 0;

    while (fscanf(f, "%s %d", u, &op) != EOF)
    {
        if (op == 1)
            (*a)++;
        else
            (*b)++;
    }
    fclose(f);
}