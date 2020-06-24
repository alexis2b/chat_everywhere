/* Header file for functions in sock_utils.c */

#ifndef _SOCK_UTILS_H_
#define _SOCK_UTILS_H_


/* Functions declarations */
struct in_addr address_resolve(char *);
int open_tcp_socket(struct in_addr, int);
int send_string(int, char *);


#endif
