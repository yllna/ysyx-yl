#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

word_t expr(char *e, bool *success);
void display_watch_point();
bool delete_watch_point(int no);
int new_watch_point(char *e);
char *check_watch_point();
#endif
