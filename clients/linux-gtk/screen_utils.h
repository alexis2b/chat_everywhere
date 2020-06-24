/* header file for screen_utils.c */

#ifndef _SCREEN_UTILS_H_
#define _SCREEN_UTILS_H_


#ifndef BUF_SIZE
#define BUF_SIZE 2048
#endif

/* functions declarations */

int init_screen(void);
void refresh_users_window(int, char**);
void print_on_screen(char *, ...);
void place_cursor(void);
void get_user_string(char *);

/* Global variables */


#endif
