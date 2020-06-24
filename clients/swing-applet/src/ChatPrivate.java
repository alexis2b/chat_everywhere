/* Chat
 * Copyright (C) 1998-2004 Alexis de Bernis <alexis@bernis.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
/* ChatPrivate
 * manages one-to-one private chat in a separate window
 */


import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import org.chateverywhere.*;

public class ChatPrivate extends JFrame implements KeyListener, WindowListener,
 JChatPaneScroller
{
	private static final long serialVersionUID = 8736530465489373588L;
	private Chat parent;
	private Container a;
	private String nick, my_nick;
	private JTextField usr_input;
	private JChatPane main_text;
	private JScrollPane main_text_scroll;
	private Font txt_fnt;
	private boolean msg_indicator;
	private InputStack input_stack;
	private ResourceBundle captions;
	


	ChatPrivate(Chat parent, String nick)
	{
		super();

		// Variables
		this.parent = parent;
		a = getContentPane();
		captions = parent.get_translated_messages();
		this.setTitle(captions.getString("MSG_CHAT")+" " + nick);
		this.my_nick = parent.get_my_nick();
		this.nick = nick;
		input_stack = new InputStack();
		txt_fnt = parent.get_txt_font();

		// Creating components
		a.setForeground(parent.getForeground());
		a.setBackground(parent.getBackground());
		usr_input = new JTextField(30);
			usr_input.setFont(txt_fnt);
			usr_input.setForeground(parent.get_fginput_color());
			usr_input.setBackground(parent.get_bginput_color());
			usr_input.addKeyListener(this);
		/* FIXME : needs parent variable for set_smileys */
		main_text = new JChatPane(this, my_nick, false);
			main_text.setForeground(parent.get_fgtext_color());
			main_text.setBackground(parent.get_bgtext_color());
			main_text.setEditable(false);
			main_text.setFont(txt_fnt);
			main_text.setMargin(new Insets(5, 5, 5, 5));
		main_text_scroll = new JScrollPane(main_text);
			main_text_scroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
			main_text_scroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);

		// Build the UI
		a.setLayout(new BorderLayout());
		a.add("Center", main_text_scroll);
		a.add("South", usr_input);
		setSize(new Dimension(400, 300));
		a.doLayout();
		addWindowListener(this);
		parent.center_window(this);
		setVisible(true);
	}

	public void disp_pmsg(String msg)
	{
		display_from(nick, msg);

		if(getFocusOwner() == null) {  // Window is not active
			setTitle("!! "+captions.getString("MSG_CHAT")+" " + nick + " !!");
			msg_indicator = true;
		}
	}

	public void display_from(String f_nick, String msg)
	{
		main_text.disp_talk(f_nick, msg);
	}

	public void scroll_to_bottom()
	{	
		main_text_scroll.getVerticalScrollBar().setValue(
		 main_text_scroll.getVerticalScrollBar().getMaximum());
	}

	public void change_font(Font new_fnt) {
		txt_fnt = new_fnt;
		main_text.setFont(new_fnt);
		usr_input.setFont(new_fnt);
	}


	public void child_terminate()
	{
		parent.pchat_closed(nick);
		dispose();
	}


	private void send_user_text()
	{
		String temp, orig = usr_input.getText();
		int nlp;

		temp = orig.trim();
		if(temp.length() > 0) {
			if(temp.startsWith("/")) {
				parent.proceed_command(temp.substring(1));
				input_stack.add(temp);
			} else {
				// Cut the message if multiple lines
				while((nlp = orig.indexOf("\n")) != -1) {
					String n = orig.substring(0, nlp);
					parent.send_pmsg(nick, n);
					display_from(my_nick, n);
					input_stack.add(n);
					orig = orig.substring(nlp + 1);
				}
				parent.send_pmsg(nick, orig);
				display_from(my_nick, orig);
				input_stack.add(orig);
			}
		}
		usr_input.setText("");
	}

	/******* stop private chat if correspond quit ********/
	public void correspondant_has_quit(String byemsg)
	{
		main_text.disp_message(nick + " "+captions.getString("MSG_USER_QUIT")+" (" + byemsg +")");
		usr_input.setEnabled(false);
	}


	/******** disable chat if we are deconnected *********/
	public void notify_deconnection()
	{
		main_text.disp_message(captions.getString("MSG_DISCONNECTED"));
		usr_input.setEnabled(false);
	}


	/*****************************************************
	 *                                                   *
	 *                  Event Handlers                   *
	 *                                                   *
	 *****************************************************/
	/******************** Keys input *********************/
	public void keyPressed(KeyEvent e) {
		int k_code = e.getKeyCode();
		
		if(k_code == KeyEvent.VK_ENTER) {
			send_user_text();
		} else if(k_code == KeyEvent.VK_UP) {
			String n = input_stack.up();

			usr_input.setText(n);
			usr_input.setCaretPosition(n.length() + 1);
		} else if(k_code == KeyEvent.VK_DOWN) {
			String n = input_stack.down();

			if(n != null) {
				usr_input.setText(n);
				usr_input.setCaretPosition(n.length() + 1);
			}
		} else if(k_code == KeyEvent.VK_ESCAPE) {
			child_terminate();
		}
	}


	/************ the user closed the window *************/
	public void windowClosing(WindowEvent e)
	{
		child_terminate();
	}

	/************ the user gave us the focus *************/
	public void windowActivated(WindowEvent e)
	{
		if(msg_indicator) {
			setTitle(captions.getString("MSG_CHAT") + " " + nick);
			msg_indicator = false;
		}
	}

	/**************** unused event handlers **************/
	public void keyReleased(KeyEvent e) {}
	public void keyTyped(KeyEvent e) {}
	public void windowClosed(WindowEvent e) {}
	public void windowDeactivated(WindowEvent e) {}
	public void windowDeiconified(WindowEvent e) {}
	public void windowIconified(WindowEvent e) {}
	public void windowOpened(WindowEvent e) {}

}
