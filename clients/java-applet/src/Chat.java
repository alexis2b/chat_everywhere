/* Chat
 * Copyright (C) 1998-2005 Alexis de Bernis <alexis@bernis.org>
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

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.util.*;
import java.applet.*;
import org.chateverywhere.*;

public class Chat extends Applet implements ActionListener, FocusListener, ItemListener, KeyListener
{
	private static String version = "Swing Applet / 1.0.1";
	private TextArea main_text;
	private TextField user_text, user_nick;
	private Button cleartext, go_login;
	private Label error_message, usernum_lbl, user_prompt;
	private java.awt.List user_list;
	private Socket com_sock;
	private InetAddress server;
	private String nick;
	private Hashtable users;
	private ChatComThread com_thread;
	private Font fnt, ui_fnt;
	private Choice usr_fontsize, usr_fontname;
	private Label l_font;
	private PopupMenu usr_popup;
	private Hashtable pmsg_windows;
	private boolean login_completed = false;
	private Locale user_locale;
	private InputStack input_stack;
	private ResourceBundle captions;
	private AudioClip bip_sound;

	private Button option_menu_trigger;
	private PopupMenu option_menu;
	private CheckboxMenuItem do_notify_connection, do_auto_scrolling;
	private CheckboxMenuItem do_bip_on_new_line, do_bip_as_sound_file;

	private boolean tab_pressed = false;



	/*****************************************************
	 *                                                   *
	 *              Applet initialisation                *
	 *                                                   *
	 *****************************************************/
	public void start()
	{
		String auto_nick;

//		System.out.println("Starting ChatEverywhere, build 200402110945");

		// New instanciations
		pmsg_windows = new Hashtable();
		input_stack = new InputStack();

		// Various initialisations
		check_locale();
		set_applet_main_colors();
		set_applet_font();
		draw_splash_screen();
		initialize_main_components();
		draw_login_window();

		// Check if the nick has been given as a parameter
		if((auto_nick = get_param("auto.nick", "Nick", null)) != null) {
			user_nick.setText(auto_nick);
			make_changements(auto_nick);
		}
	}

	/********************** Locales **********************/
	private void check_locale()
	{
		String lang, country;
	
		if((lang    = get_param("locale.language", "Language", null)) != null &&
		   (country = get_param("locale.country",  "Country",  null)) != null)
			user_locale = new Locale(lang, country);
		else
			user_locale = Locale.getDefault();

//		System.out.println("System locale is " + Locale.getDefault());
//		System.out.println("User locale is " + user_locale);

		captions = ResourceBundle.getBundle("org.chateverywhere.resources.Messages", user_locale);
	}

	/******* Applet foreground and background color ******/
	private void set_applet_main_colors()
	{
		String param_val;
		Color  color_val;

		// Check for a background color
		if((param_val = get_param("form.backgroundcolor", "BgColor", null)) != null) {
			color_val = Color.decode(param_val);
			if(color_val != null)
				setBackground(color_val);
		}

		// Check for a foreground color
		if((param_val = get_param("form.text.color", "FgColor", null)) != null) {
			color_val = Color.decode(param_val);
			if(color_val != null)
				setForeground(color_val);
		}
	}

	/******************** Working fonts ******************/
	private void set_applet_font()
	{
		String ui_fnt_name;
		String fnt_name;
		
		ui_fnt_name = get_param("form.text.font", "UIFontName",   "Arial");
		fnt_name    = get_param("chat.text.font", "TextFontName", "Courier");
		
		ui_fnt = new Font(ui_fnt_name, Font.PLAIN,  12);
		fnt    = new Font(fnt_name,    Font.PLAIN,  12);
	}


	/*****************************************************
	 *                                                   *
	 *                  User interface                   *
	 *                                                   *
	 *****************************************************/
	/************** Components initialisation ************/
	private void initialize_main_components()
	{
		int i;
		Color a_color;
		String param_val;

		main_text = new TextArea("", 20, 60, TextArea.SCROLLBARS_VERTICAL_ONLY);
			main_text.setEditable(false);
			main_text.setFont(fnt);
			main_text.setLocale(user_locale);
		user_text = new TextField(80);
			user_text.setFont(ui_fnt);
			user_text.setLocale(user_locale);
			user_text.addKeyListener(this);
			user_text.addFocusListener(this);
		usernum_lbl = new Label(captions.getString("MSG_USERS")+":      ", Label.CENTER);
			usernum_lbl.setVisible(false);
		user_prompt = new Label(captions.getString("MSG_TYPE_HERE"), Label.LEFT);
		cleartext = new Button(captions.getString("MSG_CLEAR"));
			cleartext.addActionListener(this);
		user_list = new java.awt.List(8);
			user_list.setFont(ui_fnt);
			user_list.setLocale(user_locale);
			user_list.setVisible(false);
			user_list.addItemListener(this);
		usr_fontsize = new Choice();
			for(i = 10; i <= 24; i++) {
				usr_fontsize.add(Integer.toString(i));
			}
			usr_fontsize.setFont(ui_fnt);
			usr_fontsize.select("12");
			usr_fontsize.addItemListener(this);
		usr_fontname = new Choice();
			usr_fontname.add("Arial");
			usr_fontname.add("Courier");
			usr_fontname.setFont(ui_fnt);
			usr_fontname.select("Courier");
			usr_fontname.addItemListener(this);
		l_font = new Label(captions.getString("MSG_FONT")+" : ", Label.RIGHT);
			l_font.setFont(ui_fnt);
		usr_popup = new PopupMenu(captions.getString("MSG_ACTION"));
			usr_popup.setFont(ui_fnt);
			usr_popup.add("msg");
			usr_popup.add("stats");
			usr_popup.add("ignore");
			usr_popup.add("unignore");
			usr_popup.add("kick");
			usr_popup.add("ban");
			usr_popup.add("unban");
			usr_popup.addActionListener(this);

			do_auto_scrolling = new CheckboxMenuItem(
			 captions.getString("MSG_AUTO_SCROLLING"), true);
			do_notify_connection = new CheckboxMenuItem(
			 captions.getString("MSG_NOTIFY_CONNECT"), false);
			do_bip_on_new_line = new CheckboxMenuItem(
			 captions.getString("MSG_BIP_ON_TEXT"), false);
			do_bip_as_sound_file = new CheckboxMenuItem(
			 captions.getString("MSG_BIP_AS_SOUND_FILE"),false);
			option_menu = new PopupMenu();
				option_menu.add(do_auto_scrolling);
				option_menu.add(do_bip_on_new_line);
				option_menu.add(do_bip_as_sound_file);
				option_menu.add(do_notify_connection);
			option_menu_trigger = new Button(       // I don't want to set
			 captions.getString("MSG_OPTIONS"));    // a MenuBar for that
				option_menu_trigger.setFont(ui_fnt);
				option_menu_trigger.add(option_menu);
				option_menu_trigger.addActionListener(this);

		// Check for a users zone background color
		if((param_val = get_param("chat.userlist.backgroundcolor", "UsersBgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_list.setBackground(a_color);
		}

		// Check for a users zone foreground color
		if((param_val = get_param("chat.userlist.text.color", "UsersFgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_list.setForeground(a_color);
		}

		// Check for a main text zone background color
		if((param_val = get_param("chat.textarea.backgroundcolor", "TextBgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				main_text.setBackground(a_color);
		}

		// Check for a main text zone foreground color
		if((param_val = get_param("chat.textarea.text.color", "TextFgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				main_text.setForeground(a_color);
		}

		// Check for an input zone background color
		if((param_val = get_param("chat.input.backgroundcolor", "InputBgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_text.setBackground(a_color);
		}

		// Check for an input zone foreground color
		if((param_val = get_param("chat.input.text.color", "InputFgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_text.setForeground(a_color);
		}
	}

	/*************** draw the splash screen **************/
	private void draw_splash_screen()
	{
		Label welcome, connecting;

		setLayout(new BorderLayout());
		
		welcome = new Label(captions.getString("MSG_WELCOME"), Label.CENTER);
			welcome.setFont(new Font(ui_fnt.getName(), Font.BOLD, ui_fnt.getSize()+2));
		connecting = new Label(captions.getString("MSG_LOADING"), Label.CENTER);
			connecting.setFont(new Font(ui_fnt.getName(), Font.ITALIC, ui_fnt.getSize()));
		
		add("North", welcome);
		add("Center", connecting);

		validate();
		repaint();
	}


	/******************** Login window *******************/
	private void draw_login_window()
	{
		Color a_color;
		Label splash, info;
		Panel login_panel, nick_panel, list_panel;
		Rectangle bounds;
		String param_val;

		removeAll();
		setLayout(new BorderLayout());

		// Initialize the components
		user_nick = new TextField(14);
			user_nick.setFont(ui_fnt);
			user_nick.setLocale(user_locale);
			user_nick.addActionListener(this);
		go_login = new Button(" "+captions.getString("MSG_GO")+" ! ");
			go_login.setFont(ui_fnt);
			go_login.addActionListener(this);
		error_message = new Label("                                         ");
			error_message.setFont(ui_fnt);
		splash = new Label(captions.getString("MSG_WELCOME"), Label.CENTER);
			splash.setFont(new Font(ui_fnt.getName(), Font.BOLD, ui_fnt.getSize()+2));
		nick_panel = new Panel();
			nick_panel.add(new Label(captions.getString("MSG_NICK")+" :"));
			nick_panel.add(user_nick);
			nick_panel.add(go_login);
			nick_panel.setVisible(false);
		list_panel = new Panel();
			list_panel.add(user_list);
		login_panel = new Panel(new GridLayout(5, 1));
			login_panel.add(splash);
			login_panel.add(new Label(""));
			login_panel.add(nick_panel);
			login_panel.add(error_message);
			login_panel.add(new Label(""));

		// Check for a login zone background color
		if((param_val = get_param("login.backgroundcolor", "LoginBgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_nick.setBackground(a_color);
		}

		// Check for a login zone foreground color
		if((param_val = get_param("login.text.color", "LoginFgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_nick.setForeground(a_color);
		}

		// draw the panel
		add("North", login_panel);
		add("Center", list_panel);
		add("South", usernum_lbl);

		// do the layout
		validate();

		// Retrieve the server user list
		if(make_connection(false) == true) {
			refresh_user_list();
			error_message.setText(captions.getString("MSG_SERVER_UP"));
			user_list.setVisible(true);
			usernum_lbl.setVisible(true);
			nick_panel.setVisible(true);
			repaint();
			user_nick.requestFocus();
		}
	}

	/**************** draw the main window ***************/
	private void change_ui()
	{
		Panel cmd_panel, cmd_font_panel, cmd_but_panel;

		removeAll();

		setLayout(new BorderLayout());
		user_list.add(usr_popup);
		usernum_lbl.setAlignment(Label.RIGHT);

		cmd_but_panel = new Panel(new FlowLayout(FlowLayout.CENTER, 10, 3));
			cmd_but_panel.add(option_menu_trigger);
			cmd_but_panel.add(cleartext);
		cmd_font_panel = new Panel(new FlowLayout(FlowLayout.CENTER, 10, 3));
			cmd_font_panel.add(l_font);
			cmd_font_panel.add(usr_fontname);
			cmd_font_panel.add(usr_fontsize);
		cmd_panel = new Panel(new BorderLayout(0, 10));
			cmd_panel.add("West", cmd_but_panel);
			cmd_panel.add("Center", cmd_font_panel);
			cmd_panel.add("East", usernum_lbl);

		add("North", cmd_panel);
		add("Center", main_text);
		add("East", user_list);
		add("South", user_prompt);
		add("South", user_text);
		validate();
		user_text.requestFocus();
  
  		refresh_user_list();
	}

	/******** refresh the display of the user list *******/
	private void refresh_user_list()
	{
		Enumeration users_nicks = users.keys();
		int count=0;

		user_list.removeAll();
		while(users_nicks.hasMoreElements()) {
			count++;
			user_list.add((String) users_nicks.nextElement());
		}
		usernum_lbl.setText(captions.getString("MSG_USERS")+": " + Integer.toString(count));
	}

	/******* return the Frame object of the applet *******/
	private Frame get_my_frame()
	{
		Frame result = null;
		Container c = this;

		while(result == null && c != null) {
			if(c instanceof Frame)
				result = (Frame) c;

			c = c.getParent();
		}
		return result;
	}

	/********** print a notification to the user *********/
	private void notify_user(String title, String msg)
	{
		final Dialog d = new Dialog(get_my_frame(), title, false);
		Button ok_b;
		Panel but_p;

		ok_b = new Button(captions.getString("MSG_OK"));
			ok_b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent evt) {
					d.dispose();
				}
			});

		but_p = new Panel(new FlowLayout(FlowLayout.CENTER));
			but_p.add(ok_b);

		d.setLayout(new GridLayout(2, 1));
		d.add(new Label("     " + msg + "     ", Label.CENTER));   // Provide some margins
		d.add(but_p);
		d.setForeground(getForeground());
		d.setBackground(getBackground());
		
		d.pack();
		center_window(d);
		d.setVisible(true);
	}

	/******************** plays a beep *******************/
	private void bip()
	{

		if(do_bip_as_sound_file.getState()) {
			/* We must play a sound file */
			if(bip_sound == null) /* Load the bip on demand */
				bip_sound = Applet.newAudioClip(getClass().getResource(
				 "org/chateverywhere/resources/sounds/beep.au"));
			
			bip_sound.play();

			return;
		}


		/* Else, we just play a system bip */

		try {
			java.awt.Toolkit.getDefaultToolkit().beep();
		} catch(Exception e) {
			System.out.println("Bip!");
		}
	}







	/*****************************************************
	 *                                                   *
	 *                 Internal Events                   *
	 *                                                   *
	 *****************************************************/
	/****************** request UI change ****************/
	private boolean make_changements(String nick_txt)
	{
		String param_val;
	
		nick = nick_txt.trim();
		if(!make_connection(true))
			return(false);
    
		launch_threads();
		change_ui();
		login_completed = true;

		/* Start a private message if that was requested by the
		 * user in a parameter (ex: click on a name in a web user list)
		 */
		if((param_val = get_param("auto.startchatwith", "StartChatWith", null)) != null) {
			ChatPrivate child;
			String d_nick = param_val;
			
			if((child = ((ChatPrivate) pmsg_windows.get(d_nick))) == null)
				child = create_private_window(d_nick);
			else
				child.toFront();
		}

		return true;
	}

	/**************** connect to the server **************/
	private boolean make_connection(boolean complete_login)
	{
		String param_val;
		BufferedReader com_from_server;
		BufferedWriter com_for_server;
		String received;
		int port;

		error_message.setText(captions.getString("MSG_CONNECTING"));
		error_message.repaint();

		// Try to request more permissions in case we are signed
		try {
			netscape.security.PrivilegeManager.enablePrivilege("UniversalConnect");
		} catch (Exception ex) {
			System.out.println(captions.getString("MSG_NO_PRIV_NS"));
		}
		try {
		    com.ms.security.PolicyEngine.checkPermission(com.ms.security.PermissionID.NETIO);
			com.ms.security.PolicyEngine.assertPermission(com.ms.security.PermissionID.NETIO);
		} catch (Exception ex) {
			System.out.println(captions.getString("MSG_NO_PRIV_IE"));
		}

		/* Get the server to connect to
		 * WARNING : unsigned applet can not connect on another server
		 * than the one where the web page is
		 *
		 * Some code copied from Eirc (eirc.sourceforge.net) thanks !
		 */
		try {
			if((param_val = get_param("server.name", "ServerHost", null)) != null) {
				InetAddress [] addresses = InetAddress.getAllByName(param_val);

				/* Choose a random ip address in all the possibles */
				server = addresses[(int) Math.floor(Math.random() * addresses.length)];
			} else {
				server=InetAddress.getByName(getCodeBase().getHost());
			}
		} catch(UnknownHostException e) {
			System.out.println(e);
			error_message.setText(captions.getString("MSG_NOT_FOUND"));
			error_message.repaint();
			return(false);
		} catch(SecurityException e) {
			System.out.println(e);
			error_message.setText(captions.getString("MSG_NOT_SIGNED"));
			error_message.repaint();
			return(false);
		}

		/* Get the port number to connect to (default : 5656) */
		port = Integer.parseInt(get_param("server.port", "ServerPort", "5656"));

		System.out.println(captions.getString("MSG_CONN")+" " + server + ":" + port);

		try
		{ 
			com_sock=new Socket(server, port);
			
			com_from_server = new BufferedReader(new InputStreamReader(com_sock.getInputStream()));
			com_for_server = new BufferedWriter(new OutputStreamWriter(com_sock.getOutputStream()));

			// Do we want to really login ?
			if(complete_login == true) {

				// Send some client informations (capabilities)
				// might be better elsewhere (once login is completed)
				String ref;
				String lng;
				ref = getCodeBase().toString(); /* getDocumentBase ? */
				lng = user_locale.toString();
				com_for_server.write("CLIENT <|> CAPABILITIES <|> " + version
				 + " <|> " + ref + " <|> " + lng + " <|> CLIENT\n");

				com_for_server.write("CLIENT <|> LOGIN <|> "+nick+" <|> CLIENT\n");
				com_for_server.flush();
				received=com_from_server.readLine();
				
				// Answer analysis : may be LOGIN OK (fine), ERROR (bad login)
				// or AUTH (we have to provide a password
				// Trick : the code is quite ugly, so I should fix that later, but here
				// is how it works : I just intercept the server's answer. If no auth is
				// asked, nothing is done and it is going on as usual. Else, the function
				// will provide the auth and then get back here as if we just tried to login.
				received = check_auth(received, com_for_server, com_from_server);


				if(!received.equals("SERVER <|> LOGIN OK <|> SERVER")) {
					received = received.substring(received.indexOf("<|>") + 3);
					received = received.substring(received.indexOf("<|>") + 3);
					received = received.substring(0, received.indexOf("<|>"));
					error_message.setText(captions.getString("MSG_ERROR") +
					" : " + received);
					error_message.repaint();
					return(false);
				}
			}

			com_for_server.write("CLIENT <|> USERS <|> CLIENT\n");
			com_for_server.flush();
			received = com_from_server.readLine();
			if(!received.startsWith("SERVER <|> USERS <|> ")) {
				if(!received.startsWith("SERVER <|> MSG <|> "))
					return(false);

				// This is the motd
				while(!received.startsWith("SERVER <|> USERS <|> ")) {
					received = received.substring(received.indexOf("<|>")+4).trim();
					received = received.substring(received.indexOf("<|>")+4).trim();
					disp_message("** " + received.substring(0, received.indexOf("<|>")) + "\n");
					received = com_from_server.readLine();
				}
			}

			received = received.substring(received.indexOf("<|>") + 4).trim();
			
			// We just wanted some infos, closing connection
			if(complete_login = false)
				com_sock.close();
			
			return(make_user_list(received));
		}catch(Exception e)
		{
			System.out.println(e);
			error_message.setText(captions.getString("MSG_SRV_DOWN"));
			error_message.repaint();
			return(false);
		}
	}

	/*********** launch communication threads ************/
	private boolean launch_threads()
	{
		com_thread=new ChatComThread(this,server,com_sock);
		com_thread.start();

		return(true);
	}

	/*************** the applet must stop ****************/
	public void stop()
	{
		if(login_completed) {
			Enumeration children = pmsg_windows.elements();

			while(children.hasMoreElements())
				((ChatPrivate) children.nextElement()).child_terminate();

			com_thread.send_msg("CLIENT <|> QUIT <|> Quit : client left the page <|> CLIENT");
		}
	}

	/************** must we authenticate ? ***************/
	private String check_auth(String server_answer, BufferedWriter for_server, BufferedReader from_server)
	{
		String command;
		String challenge;

		command = server_answer.substring(server_answer.indexOf("<|>") + 3);
		command = command.substring(0, command.indexOf("<|>")).trim();
		if(command.equals("AUTH")) {
			org.chateverywhere.InputDialog pass_win;
			org.chateverywhere.MD5 hasher;
			String password, p_hash, f_hash;

			challenge = server_answer.substring(server_answer.indexOf("<|>") + 3);
			challenge = challenge.substring(challenge.indexOf("<|>") + 3);
			challenge = challenge.substring(0, challenge.indexOf("<|>")).trim();
			
			pass_win = new org.chateverywhere.InputDialog(get_my_frame(),
			 captions.getString("MSG_NICK_PROTECTED"),
			 captions.getString("MSG_ASK_PASSWORD"),
			 getForeground(), getBackground(), get_fginput_color(), get_bginput_color());

			if(pass_win.is_data_ok() == false) {
				String ans;

				ans = "SERVER <|> ERROR <|> ";
				ans = ans + captions.getString("MSG_AUTH_FAILED");
				ans = ans + " <|> SERVER";
				return ans;
			}
			
			password = pass_win.get_data();
			pass_win = null;

			hasher = new org.chateverywhere.MD5();
			hasher.Update(password);
			p_hash = hasher.asHex();
			hasher.Init();
			hasher.Update(p_hash + challenge);
			f_hash = hasher.asHex();


			try {
				for_server.write("CLIENT <|> AUTH <|> " + f_hash + " <|> CLIENT\n");
				for_server.flush();
				return from_server.readLine();
			} catch(Exception e) {}
		}

		return server_answer;
	}

	/********** choice in the contextual menu ************/
	private void popupmenu_selected(String command)
	{
		if(command.equals("msg")) {
			ChatPrivate child;
			String d_nick = user_list.getSelectedItem();
			
			if((child = ((ChatPrivate) pmsg_windows.get(d_nick))) == null)
				child = create_private_window(d_nick);
			else
				child.toFront();

		} else if(command.equals("stats")) {
			com_thread.send_msg("CLIENT <|> STATS <|> " + user_list.getSelectedItem() + " <|> CLIENT");

		} else if(command.equals("ignore")) {
			com_thread.send_msg("CLIENT <|> IGNORE <|> " + user_list.getSelectedItem() + " <|> CLIENT");

		} else if(command.equals("unignore")) {
			com_thread.send_msg("CLIENT <|> UNIGNORE <|> " + user_list.getSelectedItem() + " <|> CLIENT");

		} else if(command.equals("kick")) {
			com_thread.send_msg("CLIENT <|> KICK <|> " + user_list.getSelectedItem() + " <|> CLIENT");

		} else if(command.equals("ban")) {
			com_thread.send_msg("CLIENT <|> BAN <|> " + user_list.getSelectedItem() + " <|> CLIENT");

		} else if(command.equals("unban")) {
			com_thread.send_msg("CLIENT <|> UNBAN <|> " + user_list.getSelectedItem() + " <|> CLIENT");

		}
	}







	/*****************************************************
	 *                                                   *
	 *                   Chat Events                     *
	 *                                                   *
	 *****************************************************/
	/**************** display a new message **************/
	public void disp_message(String msg)
	{
		main_text.append(msg);

		// Scroll the main window to the bottom
		// FIXME : put a better value than 30000
		if(login_completed && do_auto_scrolling.getState())
			main_text.setCaretPosition(30000);

		if(login_completed && do_bip_on_new_line.getState())
			bip();
	}

	/****************** a new user joined ****************/
	public void add_user(String name)
	{
		users.put(name,name);
		user_list.add(name);
		disp_message("**** " + name + " " + captions.getString("MSG_NEW_USER") + "\n");
		usernum_lbl.setText(captions.getString("MSG_USERS")+": " + Integer.toString(user_list.getItemCount()));

		if(do_notify_connection.getState())
			notify_user(captions.getString("MSG_NEW_CONNECTION"),
			 name + " " + captions.getString("MSG_NEW_USER"));
	}

	/******************* a user has quit *****************/
	public void remove_user(String name, String byemsg)
	{
		int count = user_list.getItemCount();
		ChatPrivate child;

		users.remove(name);
		for(int i = 0;i < count;i++) {
			if(user_list.getItem(i).equals(name)) {
				user_list.remove(i);
				break;
			}
		}
		disp_message("**** "+name+" "+captions.getString("MSG_USER_QUIT")+" (" + byemsg +")\n");
		usernum_lbl.setText("Users: " + Integer.toString(user_list.getItemCount()));
		
		// Were we private chatting with him ?
		if((child = (ChatPrivate) pmsg_windows.get(name)) != null)
			child.correspondant_has_quit(byemsg);
	}

	/************ new USERS message received *************/
	private boolean make_user_list(String received)
	{
		users = new Hashtable();

		received = received.substring(received.indexOf("<|>")+4);
		while(!received.startsWith("SERVER")) {
			String cur = received.substring(0,received.indexOf("<|>")).trim();
			received = received.substring(received.indexOf("<|>") + 4);

			if(!cur.equals(""));
				users.put(cur,cur);
		}
		return(true);
	}







	/*****************************************************
	 *                                                   *
	 *                   User input                      *
	 *                                                   *
	 *****************************************************/
	/***** <Enter> key pressed in the user text zone *****/
	private void send_user_text()
	{
		String temp, orig = user_text.getText();
		int nlp;

		temp = orig.trim();
		if(temp.length() > 0) {
			if(temp.startsWith("/")) {
				proceed_command(temp.substring(1));
				input_stack.add(temp);
			} else {
				// Cut the message if multiple lines
				while((nlp = orig.indexOf("\n")) != -1) {
					String n = orig.substring(0, nlp);
					com_thread.send_message(nick, n);
					disp_message("<" + nick + "> " + n + "\n");
					input_stack.add(n);					
					orig = orig.substring(nlp + 1);
				}
				com_thread.send_message(nick, orig);
				disp_message("<" + nick + "> " + orig + "\n");
				input_stack.add(orig);
			}
		}
		user_text.setText("");
	}

	/*********** the user has issued a command ***********/
	public void proceed_command(String txt)
	{
		String com,com2;
		
		com2 = null;
		
		if(txt.indexOf(" ")==-1) {
			com = txt.substring(0).toUpperCase();
			com_thread.send_msg("CLIENT <|> " + com + " <|> CLIENT");
		} else {
			com=txt.substring(0,txt.indexOf(" ")).toUpperCase();
			txt=txt.substring(txt.indexOf(" ")+1);
			if(txt.indexOf(" ") == -1) {
				com_thread.send_msg("CLIENT <|> "+ com +" <|> "+ txt +" <|> CLIENT");
			} else {	
				com2=txt.substring(0,txt.indexOf(" "));
				txt=txt.substring(txt.indexOf(" ")+1);
				com_thread.send_msg("CLIENT <|> " + com + " <|> " + com2 + " <|> " + txt + " <|> CLIENT");
			}

			/* If it's a private message, print it to the sender */
			if(com.equals("MSG") && com2 != null)
				disp_message(">" + com2 + "< " + txt + "\n");
		}


		/* If it's quit, we stop */
		if(com.equals("QUIT"))
			user_text.setEnabled(false);

	}

	/*********** auto nick completion with TAB ***********/
	private void nick_completion()
	{
		String to_complete, candidate, orig, nouv;
		int cp, sp;
		Enumeration all_nicks = users.keys();
		Vector possible_nicks = new Vector();


		// Take the part of input to complete
		cp = user_text.getCaretPosition();
		orig = user_text.getText();
		to_complete = orig.substring(0, cp);
		if((sp = to_complete.lastIndexOf(" ")) != -1)
			to_complete = to_complete.substring(sp + 1);

		if(to_complete.equals(""))
			return;

		// Take the possibles choices
		while(all_nicks.hasMoreElements()) {
			candidate = (String) all_nicks.nextElement();
			if(candidate.toLowerCase().startsWith(to_complete.toLowerCase()))
				possible_nicks.addElement(candidate);
		}

		// No candidates
		if(possible_nicks.size() == 0)
			return;

		// Several candidates
		if(possible_nicks.size() > 1) {
			all_nicks = possible_nicks.elements();
			candidate = all_nicks.nextElement() + " ";
			while(all_nicks.hasMoreElements())
				candidate = candidate + all_nicks.nextElement() + " ";

			disp_message("++ "+captions.getString("MSG_POSS_NICKS")+" : " + candidate + "\n");
			return;
		}

		// do completion
		candidate = (String) possible_nicks.firstElement();
		if(sp == -1)
			candidate = candidate + ": ";
		orig = orig.substring(0, cp - to_complete.length()) + candidate + orig.substring(cp);
		user_text.setText(orig);
		user_text.setCaretPosition(cp - to_complete.length() + candidate.length());
	}








	/*****************************************************
	 *                                                   *
	 *        Private chat in separate windows           *
	 *                                                   *
	 *****************************************************/
	/************** new private chat window **************/
	public ChatPrivate create_private_window(String d_nick)
	{
		ChatPrivate child;

		child = new ChatPrivate(this, d_nick);
		pmsg_windows.put(d_nick, child);

		return child;
	}

	/*************** display a private msg ***************/
	public void disp_private_msg(String d_nick, String msg)
	{
		ChatPrivate child = (ChatPrivate) pmsg_windows.get(d_nick);

		if(child == null)
			child = create_private_window(d_nick);

		child.disp_pmsg(msg);
	}

	/************** send a private message ***************/
	public void send_pmsg(String to, String msg)
	{
		com_thread.send_msg("CLIENT <|> MSG <|> " + to + " <|> " + msg + " <|> CLIENT");
	}

	/************ the window has been closed *************/
	public void pchat_closed(String p_nick)
	{
		pmsg_windows.remove(p_nick);
	}





	/*****************************************************
	 *                                                   *
	 *                  Event Handlers                   *
	 *                                                   *
	 *****************************************************/
	/**************** click on a button ******************/
	public void actionPerformed(ActionEvent evt)
	{
		Object src = evt.getSource();
		
//		System.out.println("Event received from " + src);
		
		if(src.equals(user_nick) || src.equals(go_login)) {
			make_changements(user_nick.getText());
		}

		if(src.equals(cleartext))
			main_text.setText("");

		if(src.equals(usr_popup)) {
			popupmenu_selected(evt.getActionCommand());
		}

		if(src.equals(option_menu_trigger)) {
			// getHeight() is not supported by MS JVM
			option_menu.show(option_menu_trigger, 0, option_menu_trigger.getSize().height);
		}
	}


	/******************** Keys input *********************/
	public void keyPressed(KeyEvent e) {
		int k_code = e.getKeyCode();
		
		if(k_code == KeyEvent.VK_ENTER) {
			send_user_text();
		} else if(k_code == KeyEvent.VK_TAB) {
			tab_pressed = true;
			nick_completion();
		} else if(k_code == KeyEvent.VK_UP) {
			String n = input_stack.up();

			user_text.setText(n);
			user_text.setCaretPosition(n.length() + 1);
		} else if(k_code == KeyEvent.VK_DOWN) {
			String n = input_stack.down();

			if(n != null) {
				user_text.setText(n);
				user_text.setCaretPosition(n.length() + 1);
			}
		}
	}

	/************ combo box (choice) changed *************/
	public void itemStateChanged(ItemEvent evt)
	{
		Object src = evt.getSource();
		
		if((src.equals(usr_fontsize) || src.equals(usr_fontname)) &&
		    evt.getStateChange() == evt.SELECTED) {
			Enumeration children;

			fnt = new Font(usr_fontname.getSelectedItem(), Font.PLAIN,
			               Integer.parseInt(usr_fontsize.getSelectedItem()));
			main_text.setFont(fnt);
			validate();

			// Tells children windows to do so
			children = pmsg_windows.elements();
			while(children.hasMoreElements())
				((ChatPrivate) children.nextElement()).change_font(fnt);
		}

		/* display a popup menu when a user clicks on a name in the list */
		/* FIXME : we have a bug in here if the user moves the scrollbar of
		 * the user list down, since we only use a computation on the element
		 * index to place the pop up. I do not see any useful method in the actual
		 * API that would allow me to correct that...
		 * temporary fix: make sure popup does not appear too low
		 */
		if(src.equals(user_list) && evt.getStateChange() == evt.SELECTED && login_completed) {
			// Display the popup menu where the user just clicked
			int item_number = Integer.parseInt(evt.getItem().toString());

			// FIXME : rule of thumb for the click position
			int pos_y = item_number * 17 + 12;
			if(pos_y > user_list.getSize().height)
				pos_y = 3 * user_list.getSize().height / 4;
			usr_popup.show(user_list, 30, pos_y);
		}
	}

	/***** keep the focus on user_txt if TAB pressed *****/
	public void focusLost(FocusEvent e)
	{
		if(tab_pressed) {
			user_text.requestFocus();
			tab_pressed = false;
		}
	}


	/**************** unused event handlers **************/
	public void keyReleased(KeyEvent e) {}
	public void keyTyped(KeyEvent e) {}
	public void focusGained(FocusEvent e) {}







	/*****************************************************
	 *                                                   *
	 *                    Utilities                      *
	 *                                                   *
	 *****************************************************/
	/*************** center on the screen ****************/
	public void center_window(Window win)
	{
		Dimension screen;
		Dimension w = win.getSize();
		
//		try {
			screen = this.getToolkit().getScreenSize();
//		} catch(Exception e) {
			// getToolkit() can cause a security exception on IE
//			screen = new Dimension(800, 600);
//		}

		/* Center the dialog on the screen */
		win.setBounds(screen.width/2 - w.width/2,
		              screen.height/2 - w.height/2,
		              w.width, w.height);
	}

	/******** returns an applet parameter value **********/
	public String get_param(String param_txt, String deflt)
	{ return get_param(param_txt, "dummy_parameter", deflt); }
	public String get_param(String param_txt1, String param_txt2, String deflt)
	{
		String res;
		
		/* This function was made necessary because of the
		 * new parameters, we have to manage old syntax to
		 * keep backward compatibility, and new syntax, with
		 * the added flexibility of a default value.
		 *
		 * So here is how that works :
		 *   -> if the parameter (param_txt1) exists (new
		 *      syntax), use that one
		 *   -> if not, then check the parameter (param_txt2)
		 *      (old syntax), and if it exists, use that one
		 *   -> at last, if none exists, returns the default value
		 */
		if(     (res = getParameter(param_txt1)) != null);
		else if((res = getParameter(param_txt2)) != null);
		else     res = deflt;

		return res;
	}





	/*****************************************************
	 *                                                   *
	 *                    Accessors                      *
	 *                                                   *
	 *****************************************************/
	/******************** applet fonts *******************/
	public Font get_txt_font() { return fnt; }	
	public Font get_ui_font()  { return ui_fnt; }

	/******************* applet colors *******************/
	public Color get_fginput_color() { return user_text.getForeground(); }
	public Color get_bginput_color() { return user_text.getBackground(); }
	public Color get_fgtext_color() { return main_text.getForeground(); }
	public Color get_bgtext_color() { return main_text.getBackground(); }

	/********************* user nick *********************/
	public String get_my_nick() { return nick; }

	/****************** resource bundle ******************/
	public ResourceBundle get_translated_messages() { return captions; }
	
	/*********** applet parameters information ***********/
	public String[][] getParameterInfo()
	{
		String[][] pinfo = {
			{ "form.backgroundcolor", "#000000-#FFFFFF", "Background color of the applet (optional)"},
			{ "form.text.color", "#000000-#FFFFFF", "Foreground color of the applet (optional)"},
			{ "chat.userlist.backgroundcolor", "#000000-#FFFFFF", "Background color of the user list (optional)"},
			{ "chat.userlist.text.color", "#000000-#FFFFFF", "Foreground color of the user list (optional)"},
			{ "chat.textarea.backgroundcolor", "#000000-#FFFFFF", "Background color of the text area (optional)"},
			{ "chat.textarea.text.color", "#000000-#FFFFFF", "Foreground color of the text area (optional)"},
			{ "chat.input.backgroundcolor", "#000000-#FFFFFF", "Background color of the input area (optional)"},
			{ "chat.input.text.color", "#000000-#FFFFFF", "Foreground color of the input area (optional)"},
			{ "login.backgroundcolor", "#000000-#FFFFFF", "Background color of the login area (nick text field) (optional)"},
			{ "login.text.color", "#000000-#FFFFFF", "Foreground color of the login area (nick text field) (optional)"},
			{ "form.text.font", "text", "Font to apply to the user interface components"},
			{ "chat.text.font", "text", "Font to apply to the text zones"},
			{ "server.name", "host.domain.tld", "Adress of the server to connect to"},
			{ "server.port", "1-65535", "Number of the port the server listens to"},
			{ "auto.nick", "text", "Nick of the user (optional)"},
			{ "auto.startchatwith", "nick", "Start a private chat upon connecting"}
		};
		return pinfo;
	}
}
