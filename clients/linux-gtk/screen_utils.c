/* Various ncurses related functions */


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <gtk/gtk.h>

#include "screen_utils.h"



/* Declared here because statics */
GtkWidget *MainWindow;


/*
 * Init ncurses and draw the screen
 */
int init_screen(void)
{
	WINDOW *stdscreen;

	/* Let us initialize curses */
	stdscreen = initscr();

	/* Construct the three windows */
	win_msg = newwin(1, 0, 1, 0);
	bmain_panel = newwin(LINES - 2, COLS - 10, 2, 0);
	win_main_panel = newwin(LINES - 4, COLS - 12, 3, 1);
	busers = newwin(LINES - 2, 10, 2, COLS - 10);
	win_users = newwin (LINES - 4, 8, 3, COLS - 9);

	/* Set windows options */
	nocbreak();          /* let ncurses processes the input buffer */
	echo();
	scrollok(win_main_panel, TRUE);
	scrollok(win_users, TRUE);
	scrollok(win_msg, FALSE);
	scrollok(stdscreen, FALSE);
//	wtimeout(win_msg, 0);
//	immedok(win_msg, 1);
	curs_set(1);
//	clearok(curscr, TRUE);

	/* draw the borders */
	waddstr(win_msg, "> ");
	wnoutrefresh(win_msg);
	box(bmain_panel, 0, 0);
	wnoutrefresh(bmain_panel);
	box(busers, 0, 0);
	wnoutrefresh(busers);
	
	wnoutrefresh(win_users);

	doupdate();
	refresh();

/*	input_buffer = malloc(BUF_SIZE);
	input_pos = input_buffer;
	cursor_col = 0; */

	place_cursor();
	return(0);
}


/*
 * Updates the users window
 */
void refresh_users_window(int number, char **users)
{
	int i;

	wclear(win_users);
	for(i = 0; i < number; i++)
	{
		waddstr(win_users, users[i]);
		waddch(win_users, '\n');
	}
	wrefresh(win_users);

	place_cursor();
}


/*
 * Add a line to the main_panel
 */
void print_on_screen(char *fstring, ...)
{
	char* buffer;
	va_list pp;
	
	va_start(pp, fstring);
	
	buffer = malloc(BUF_SIZE);
	vsnprintf(buffer, BUF_SIZE, fstring, pp);
	
	waddstr(win_main_panel, buffer);
	wrefresh(win_main_panel);
	free(buffer);

	place_cursor();
}


/*
 * Place the cursor on the msg window
 */
void place_cursor()
{
	//waddch(win_msg, '\0');
//	refresh();
	move(1, 2);
//	clearok(curscr, TRUE);
	refresh();
}


void get_user_string(char *input)
{
	getstr(input);
	box(bmain_panel, 0, 0);
	box(busers, 0, 0);
	wrefresh(bmain_panel);
}
