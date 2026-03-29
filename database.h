#ifndef DATABASE_H
#define DATABASE_H

int check_user(char *user, char *pass);
int check_voted(char *user);
void store_vote(char *user, int option);
void get_results(int *a, int *b);

#endif