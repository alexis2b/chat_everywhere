/* Header for commands.c */


#ifndef _COMMANDS_H_
#define _COMMANDS_H_

/* defines */

#define COM_UNKNOWN 0
#define COM_TALK 1
#define COM_PMSG 2
#define COM_MSG 3
#define COM_ERROR 4


/* Functions declarations */

int treat_server_command(void);
int whatis_command(char *);

int dispatch_talk_command(struct chat_message *);
int dispatch_msg_command(struct chat_message *);
int dispatch_pmsg_command(struct chat_message *);
int dispatch_error_command(struct chat_message *);


#endif
