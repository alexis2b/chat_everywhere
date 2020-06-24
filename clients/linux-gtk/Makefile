FL=`gtk-config --cflags`

all: nc_chat

nc_chat: nc_chat.c nc_chat.h config.h\
		   io_comm.o sock_utils.o fd_utils.o\
		   str_utils.o commands.o screen_utils.o\
		   user_input.o
	gcc ${FL} -Wall -g -lncurses nc_chat.c io_comm.o fd_utils.o str_utils.o sock_utils.o screen_utils.o commands.o user_input.o -o nc_chat

io_comm.o: io_comm.c io_comm.h
	gcc -Wall -g -c io_comm.c -o io_comm.o

sock_utils.o: sock_utils.c sock_utils.h
	gcc -Wall -g -c sock_utils.c -o sock_utils.o
	
fd_utils.o: fd_utils.c fd_utils.h
	gcc -Wall -g -c fd_utils.c -o fd_utils.o
	
str_utils.o: str_utils.c str_utils.h
	gcc -Wall -g -c str_utils.c -o str_utils.o

screen_utils.o: screen_utils.c screen_utils.h
	gcc ${FL} -Wall -g -c screen_utils.c -o screen_utils.o

commands.o: commands.c commands.h
	gcc -Wall -g -c commands.c -o commands.o
	
user_input.o: user_input.c user_input.h
	gcc -Wall -g -c user_input.c -o user_input.o


clean:
	rm -f *.o
	rm -f nc_chat
