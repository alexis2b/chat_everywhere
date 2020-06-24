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

import java.applet.Applet;
import java.applet.AudioClip;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.Label;
import java.awt.Rectangle;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.Vector;

import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.border.EmptyBorder;

import org.chateverywhere.ChatMessage;
import org.chateverywhere.InputStack;
import org.chateverywhere.JChatPane;
import org.chateverywhere.JChatPaneScroller;
import org.chateverywhere.MyTextField;
import org.chateverywhere.SimpleLabelCellRenderer;




public class Chat extends JApplet implements ActionListener, FocusListener,
 ItemListener, KeyListener, MouseListener, JChatPaneScroller
{
	private static final long serialVersionUID = 8693123908964281771L;
	private static String version = "Java Applet / 1.0.0";
	private Container a;
	private JChatPane main_text;
	private JScrollPane main_text_scroll;
	private MyTextField user_text;
	private JTextField user_nick;
	private JButton cleartext, go_login;
	private JLabel error_message, usernum_lbl, user_prompt, nick_lbl;
	private JList user_list;
	private JScrollPane user_list_scroll;
	private String nick;
	private Hashtable<String, JLabel> users;
	private ChatComThread com_thread;
	private Font fnt, ui_fnt;
	private JComboBox usr_fontsize, usr_fontname;
	private JLabel l_font;
	private JPopupMenu usr_popup;
	private Hashtable<String, ChatPrivate> pmsg_windows;
	private boolean login_completed = false;
	private Locale user_locale;
	private InputStack input_stack;
	private ResourceBundle captions;
	private AudioClip bip_sound;

	private JButton option_menu_trigger;
	private JPopupMenu option_menu;
	private JCheckBoxMenuItem do_notify_connection, do_auto_scrolling;
	private JCheckBoxMenuItem do_bip_on_new_line, do_bip_as_sound_file;
	private JCheckBoxMenuItem do_smileys_as_pics;

	private boolean tab_pressed = false;



	/*****************************************************
	 *                                                   *
	 *              Applet initialisation                *
	 *                                                   *
	 *****************************************************/
	public void start()
	{
		String auto_nick;

		// New instanciations
		pmsg_windows = new Hashtable<String, ChatPrivate>();
		input_stack = new InputStack();
		a = getContentPane();

		/* Various initialisations */
		check_locale();
		launch_communication_thread();
		set_applet_main_colors();
		set_applet_font();
		System.out.println("Draw the splash screen...");
		draw_splash_screen();
		System.out.println("Done!");
		initialize_main_components();
		draw_connection_in_progress();

		/* See if the server is up, and proceed */
		if(wait_for_server_connection()) {
			send_capabilities();
			draw_login_window();

			/* Request the user list */
			com_thread.send_client_msg("USERS");

			/* Check if the nick has been given as a parameter */
			if((auto_nick = get_param("auto.nick", "Nick", null)) != null) {
				user_nick.setText(auto_nick);
				request_login(auto_nick);
			}
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

	/******* Start the server communication thread *******/
	private void launch_communication_thread()
	{
		String server_host;
		int    server_port;

		server_host = get_param("server.name", "ServerHost", getCodeBase().getHost());
		server_port = Integer.parseInt(get_param("server.port", "ServerPort", "5656"));
		
		/* For local testing */
		if(server_host.length() == 0)
			server_host = "localhost";
		
		/* Create and launch the communication thread
		 * it will automatically attempt to initiate a connection */
		com_thread = new ChatComThread(this, server_host, server_port);
		com_thread.start();
	}

	/******* Applet foreground and background color ******/
	private void set_applet_main_colors()
	{
		String param_val;
		Color  color_val;

		/* Check for a background color */
		if((param_val = get_param("form.backgroundcolor", "BgColor", null)) != null) {
			color_val = Color.decode(param_val);
			if(color_val != null)
				a.setBackground(color_val);
		}

		/* Check for a foreground color */
		if((param_val = get_param("form.text.color", "FgColor", null)) != null) {
			color_val = Color.decode(param_val);
			if(color_val != null)
				a.setForeground(color_val);
		}
	}

	/******************** Working fonts ******************/
	private void set_applet_font()
	{
		String ui_fnt_name;
		String fnt_name;

		ui_fnt_name = get_param("form.text.font", "UIFontName",   "dialog");
		fnt_name    = get_param("chat.text.font", "TextFontName", "monospaced");

		ui_fnt = new Font(ui_fnt_name, Font.PLAIN, 14);
		fnt    = new Font(fnt_name,    Font.PLAIN, 12);
	}


	/*****************************************************
	 *                                                   *
	 *                  User interface                   *
	 *                                                   *
	 *****************************************************/
	/*************** draw the splash screen **************/
	private void draw_splash_screen()
	{
		JLabel welcome, connecting;

		a.setLayout(new BorderLayout());

		welcome = new JLabel(captions.getString("MSG_WELCOME"), JLabel.CENTER);
			welcome.setFont(new Font(ui_fnt.getName(), Font.BOLD, ui_fnt.getSize()+2));
		connecting = new JLabel(captions.getString("MSG_LOADING"), JLabel.CENTER);
			connecting.setFont(new Font(ui_fnt.getName(), Font.ITALIC, ui_fnt.getSize()));

		a.add("North", welcome);
		a.add("Center", connecting);

		a.validate();
		a.repaint();
//		Thread.yield();    // Make sure that these are executed
	}

	/************** Components initialisation ************/
	private void initialize_main_components()
	{
		String param_val;
		int i;
		Color a_color;
		JMenuItem menu_item;

		error_message = new JLabel("                                         ");
			error_message.setFont(ui_fnt);
		main_text = new JChatPane(this, nick, false);
			main_text.setEditable(false);
			main_text.setFont(fnt);
			main_text.setLocale(user_locale);
			main_text.setMargin(new Insets(5, 5, 5, 5));
		main_text_scroll = new JScrollPane(main_text);
			main_text_scroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
			main_text_scroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
		user_text = new MyTextField();  // 80
			user_text.setFont(fnt);
			user_text.setLocale(user_locale);
			user_text.setEnabled(true);
			user_text.addKeyListener(this);
			user_text.addFocusListener(this);
		usernum_lbl = new JLabel(captions.getString("MSG_USERS")+":      ", JLabel.CENTER);
			usernum_lbl.setFont(ui_fnt);
			usernum_lbl.setVisible(false);
		nick_lbl = new JLabel(" ");
			nick_lbl.setFont(fnt);
		user_prompt = new JLabel(captions.getString("MSG_TYPE_HERE"), JLabel.LEFT);
			user_prompt.setFont(ui_fnt);
		cleartext = new JButton(captions.getString("MSG_CLEAR"));
			cleartext.setFont(ui_fnt);
			cleartext.addActionListener(this);
		user_list = new JList();
			user_list.setFont(ui_fnt);
			user_list.setLocale(user_locale);
			user_list.setCellRenderer(new SimpleLabelCellRenderer());
			user_list.setBorder(new EmptyBorder(new Insets(1, 1, 1, 5)));
			user_list.setFixedCellWidth(100);
			user_list.setFixedCellHeight(14);
			user_list.addMouseListener(this);
		user_list_scroll = new JScrollPane(user_list);
			user_list_scroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
			user_list_scroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
			user_list_scroll.setVisible(false);
		usr_fontsize = new JComboBox();
			for(i = 10; i <= 24; i++) {
				usr_fontsize.addItem(Integer.toString(i));
			}
			usr_fontsize.setFont(ui_fnt);
			usr_fontsize.setSelectedItem("12");
			usr_fontsize.addItemListener(this);
		usr_fontname = new JComboBox();
			usr_fontname.addItem("monospaced");
			usr_fontname.addItem("sansserif");
			usr_fontname.addItem("serif");
			usr_fontname.setFont(ui_fnt);
			usr_fontname.setSelectedItem(fnt.getFamily());
			usr_fontname.addItemListener(this);
		l_font = new JLabel(captions.getString("MSG_FONT")+" : ", JLabel.RIGHT);
			l_font.setFont(ui_fnt);
		usr_popup = new JPopupMenu(captions.getString("MSG_ACTION"));
			usr_popup.setFont(ui_fnt);
			menu_item = new JMenuItem("msg");
				menu_item.addActionListener(this);
				usr_popup.add(menu_item);
			menu_item = new JMenuItem("stats");
				menu_item.addActionListener(this);
				usr_popup.add(menu_item);
			menu_item = new JMenuItem("ignore");
				menu_item.addActionListener(this);
				usr_popup.add(menu_item);
			menu_item = new JMenuItem("unignore");
				menu_item.addActionListener(this);
				usr_popup.add(menu_item);
			menu_item = new JMenuItem("kick");
				menu_item.addActionListener(this);
				usr_popup.add(menu_item);
			menu_item = new JMenuItem("ban");
				menu_item.addActionListener(this);
				usr_popup.add(menu_item);
			menu_item = new JMenuItem("unban");
				menu_item.addActionListener(this);
				usr_popup.add(menu_item);

			do_auto_scrolling = new JCheckBoxMenuItem(
			 captions.getString("MSG_AUTO_SCROLLING"), true);
			do_notify_connection = new JCheckBoxMenuItem(
			 captions.getString("MSG_NOTIFY_CONNECT"), false);
			do_bip_on_new_line = new JCheckBoxMenuItem(
			 captions.getString("MSG_BIP_ON_TEXT"), false);
			do_bip_as_sound_file = new JCheckBoxMenuItem(
			 captions.getString("MSG_BIP_AS_SOUND_FILE"),false);
			do_smileys_as_pics = new JCheckBoxMenuItem(
			 captions.getString("MSG_SMILEYS_AS_PICS"),
			 main_text.get_smileys_as_pics());
				do_smileys_as_pics.addActionListener(this);
			option_menu_trigger = new JButton(      // I don't want to set
			 captions.getString("MSG_OPTIONS"));    // a JMenuBar for that
				option_menu_trigger.setFont(ui_fnt);
				option_menu_trigger.setActionCommand("options");
				option_menu_trigger.addActionListener(this);
			option_menu = new JPopupMenu();
				option_menu.add(do_auto_scrolling);
				option_menu.add(do_bip_on_new_line);
				option_menu.add(do_bip_as_sound_file);
				option_menu.add(do_notify_connection);
				option_menu.add(do_smileys_as_pics);

		/* Check for a users zone background color */
		if((param_val = get_param("chat.userlist.backgroundcolor", "UsersBgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_list.setBackground(a_color);
		}

		/* Check for a users zone foreground color */
		if((param_val = get_param("chat.userlist.text.color", "UsersFgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_list.setForeground(a_color);
		}

		/* Check for a main text zone background color */
		if((param_val = get_param("chat.textarea.backgroundcolor", "TextBgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				main_text.setBackground(a_color);
		}

		/* Check for a main text zone foreground color  */
		if((param_val = get_param("chat.textarea.text.color", "TextFgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				main_text.setForeground(a_color);
		}

		/* Check for an input zone background color */
		if((param_val = get_param("chat.input.backgroundcolor", "InputBgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_text.setBackground(a_color);
		}

		/* Check for an input zone foreground color */
		if((param_val = get_param("chat.input.text.color", "InputFgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_text.setForeground(a_color);
		}
	}

	/************* connection is in progress *************/
	private void draw_connection_in_progress()
	{
		error_message.setText("Connection in progress...");
		a.add("South", error_message);

		a.repaint();
	}

	/************** connecting to the server *************/
	private boolean wait_for_server_connection()
	{
		/* wait for the connection to be completed */
		while(com_thread.get_connection_state() ==
		 ChatComThread.CONNECTION_IN_PROGRESS) {
			System.out.println("=> connecting...");

			/* Wait a bit */
			try {
				Thread.sleep(100);
			} catch(Exception e) { }

		}

		/* connection failed */
		if(com_thread.get_connection_state() ==
		 ChatComThread.CONNECTION_FAILED) {
			error_message.setText(com_thread.get_fail_reason());
			error_message.repaint();
			return false;
		}

		/* connection succeeded */
		if(com_thread.get_connection_state() ==
		 ChatComThread.CONNECTED) {
		 	System.out.println("!! Connected !!");
			return true;
		}

		/* should not be reached */
		System.out.println("==> unknown connection state  <==");
		return false;
	}

	/*************** send some client infos **************/
	private void send_capabilities()
	{
		String u_a = version;
		String ref;
		String lng;
		
		ref = getCodeBase().toString(); /* getDocumentBase ? */
		lng = user_locale.toString();
		com_thread.send_client_msg("CAPABILITIES", u_a, ref, lng);
	}


	/******************** Login window *******************/
	private void draw_login_window()
	{
		String param_val;
		Color a_color;
		JLabel splash;
		JPanel login_panel, nick_panel, list_panel;

		a.removeAll();
		a.setLayout(new BorderLayout());

		// Initialize the components
		user_nick = new JTextField(14);
			user_nick.setFont(ui_fnt);
			user_nick.setLocale(user_locale);
			user_nick.getCaret().setVisible(true);
			user_nick.addActionListener(this);
		go_login = new JButton(" "+captions.getString("MSG_GO")+" ! ");
			go_login.setFont(ui_fnt);
			go_login.addActionListener(this);
		splash = new JLabel(captions.getString("MSG_WELCOME"), JLabel.CENTER);
			splash.setFont(new Font(ui_fnt.getName(), Font.BOLD, ui_fnt.getSize()+2));
		nick_panel = new JPanel();
			nick_panel.setOpaque(false);
			nick_panel.add(new JLabel(captions.getString("MSG_NICK")+" :"));
			nick_panel.add(user_nick);
			nick_panel.add(go_login);
			nick_panel.setVisible(false);
		list_panel = new JPanel();
			list_panel.setOpaque(false);
			list_panel.add(user_list_scroll);
		login_panel = new JPanel(new GridLayout(5, 1));
			login_panel.setOpaque(false);
			login_panel.add(splash);
			login_panel.add(new JLabel(""));
			login_panel.add(nick_panel);
			login_panel.add(error_message);
			login_panel.add(new JLabel(""));

		/* Check for a login zone background color */
		if((param_val = get_param("login.backgroundcolor", "LoginBgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_nick.setBackground(a_color);
		}

		/* Check for a login zone foreground color */
		if((param_val = get_param("login.text.color", "LoginFgColor", null)) != null) {
			if((a_color = Color.decode(param_val)) != null)
				user_nick.setForeground(a_color);
		}

		// draw the panel
		a.add("North", login_panel);
		a.add("Center", list_panel);
		a.add("South", usernum_lbl);

		// do the layout
		a.validate();

		error_message.setText(captions.getString("MSG_SERVER_UP"));
		user_list_scroll.setVisible(true);
		usernum_lbl.setVisible(true);
		nick_panel.setVisible(true);
		a.repaint();
		user_nick.requestFocus();
	}

	/**************** draw the main window ***************/
	private void change_ui()
	{
		JPanel cmd_panel, prompt_panel, cmd_font_panel, cmd_but_panel;

		a.removeAll();

		a.setLayout(new BorderLayout());
		user_list.add(usr_popup);
		usernum_lbl.setHorizontalAlignment(JLabel.RIGHT);
		nick_lbl.setText(nick + " ");

		cmd_but_panel = new JPanel(new FlowLayout(FlowLayout.CENTER, 10, 3));
			cmd_but_panel.setOpaque(false);
			cmd_but_panel.add(option_menu_trigger);
			cmd_but_panel.add(cleartext);
		cmd_font_panel = new JPanel(new FlowLayout(FlowLayout.CENTER, 10, 3));
			cmd_font_panel.setOpaque(false);
			cmd_font_panel.add(l_font);
			cmd_font_panel.add(usr_fontname);
			cmd_font_panel.add(usr_fontsize);
		cmd_panel = new JPanel(new BorderLayout(0, 10));
			cmd_panel.setOpaque(false);
			cmd_panel.add("West", cmd_but_panel);
			cmd_panel.add("Center", cmd_font_panel);
			cmd_panel.add("East", usernum_lbl);
		prompt_panel = new JPanel(new BorderLayout());
			prompt_panel.setOpaque(false);
			prompt_panel.add("West", nick_lbl);
			prompt_panel.add("Center",user_text);
			prompt_panel.add("East", user_prompt);

		main_text.set_my_nick(nick);

		a.add("North", cmd_panel);
		a.add("Center", main_text_scroll);
		a.add("East", user_list_scroll);
		a.add("South", prompt_panel);
		a.validate();
		user_text.requestFocus();
  
		/* Request the new user list */
  		com_thread.send_client_msg("USERS");
	}

	/******** refresh the display of the user list *******/
	public void refresh_user_list(ChatMessage msg)
	{
		int count=0;
		String cur;
		int i, n = msg.get_args_num();

		/* first, make the user list */
		users = new Hashtable<String, JLabel>();
		for(i = 1; i <= n; i++) {
			cur = msg.get_arg(i);
			if(!cur.equals(""));
				users.put(cur, get_label_for(cur));
		}

		/* then update the list */
		user_list.setListData(new Vector<JLabel>(users.values()));
		count = users.size();

		usernum_lbl.setText(captions.getString("MSG_USERS")+": " + Integer.toString(count));
	}

	/******** return the JLabel for a given nick *********/
	private JLabel get_label_for(String nick) {
		JLabel res;
		
		res = new JLabel(nick, JLabel.RIGHT);
		res.setOpaque(true);
		res.setFont(res.getFont().deriveFont(0));    // We do not want it Bold

		return res;
	}

	/***************** password dialog *******************/
	private String ask_password()
	{
		JPasswordField pf = new JPasswordField();
		Object[] message = new Object[] {captions.getString("MSG_ASK_PASSWORD"), pf};

		if(JOptionPane.showOptionDialog(this, message, captions.getString("MSG_NICK_PROTECTED"),
		 JOptionPane.OK_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE, null, null, null) == 0) // OK
			return String.valueOf(pf.getPassword());
		else
			return null; // Cancel
	}

	/********** print a notification to the user *********/
	// FIXME : needs to appear centered !
	private void notify_user(String title, String msg)
	{
		// We can not just use a JOptionPane here, because it needs
		// to be *not* modal.
		final JDialog d = new JDialog((Frame) null, title, false);
		JButton ok_b;
		JPanel but_p;
		Container d_p = d.getContentPane();


		ok_b = new JButton(captions.getString("MSG_OK"));
			ok_b.setOpaque(false);
			ok_b.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent evt) {
					d.dispose();
				}
			});

		but_p = new JPanel(new FlowLayout(FlowLayout.CENTER));
			but_p.setOpaque(false);
			but_p.add(ok_b);

		d_p.setLayout(new GridLayout(2, 1));
		d_p.add(new Label("     " + msg + "     ", Label.CENTER));   // Provide some margins
		d_p.add(but_p);
		d_p.setForeground(a.getForeground());
		d_p.setBackground(a.getBackground());
		d.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);

		d.pack();
		center_window(d);
		d.setVisible(true);


	}

	/******************** plays a beep *******************/
	private void bip()
	{
		if(do_bip_as_sound_file.isSelected()) {
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
	private void request_login(String nick_txt)
	{
		nick = nick_txt.trim();
		com_thread.send_client_msg("LOGIN", nick);
	}

	/******************* login accepted ******************/
	public void login_ok()
	{
		String param_val;
		
		com_thread.set_my_nick(nick);
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
	}

	/*************** the applet must stop ****************/
	public void stop()
	{
		if(login_completed) {
			Enumeration children = pmsg_windows.elements();

			while(children.hasMoreElements())
				((ChatPrivate) children.nextElement()).child_terminate();

			com_thread.send_client_msg("QUIT", "Quit : client left the page");
		}
	}

	/************** must we authenticate ? ***************/
	public void check_auth(ChatMessage msg)
	{
		org.chateverywhere.MD5 hasher;
		String challenge, password, p_hash, f_hash;

		challenge = msg.get_arg(1);
			
		password = ask_password();

		if(password == null) {
			error_message.setText(captions.getString("MSG_AUTH_FAILED"));
			return;
		}

		hasher = new org.chateverywhere.MD5();
		hasher.Update(password);
		p_hash = hasher.asHex();
		hasher.Init();
		hasher.Update(p_hash + challenge);
		f_hash = hasher.asHex();

		com_thread.send_client_msg("AUTH", f_hash);
	}

	/********** choice in the contextual menu ************/
	private void popupmenu_selected(String command)
	{
		String d_nick = ((JLabel) user_list.getSelectedValue()).getText();
		
		if(command.equals("msg")) {
			ChatPrivate child;
			
			if((child = ((ChatPrivate) pmsg_windows.get(d_nick))) == null)
				child = create_private_window(d_nick);
			else
				child.toFront();

		} else if(command.equals("stats")) {
			com_thread.send_client_msg("STATS", d_nick);

		} else if(command.equals("ignore")) {
			com_thread.send_client_msg("IGNORE", d_nick);

		} else if(command.equals("unignore")) {
			com_thread.send_client_msg("UNIGNORE", d_nick);

		} else if(command.equals("kick")) {
			com_thread.send_client_msg("KICK", d_nick);

		} else if(command.equals("ban")) {
			com_thread.send_client_msg("BAN", d_nick);

		} else if(command.equals("unban")) {
			com_thread.send_client_msg("UNBAN", d_nick);

		}
	}






	/*****************************************************
	 *                                                   *
	 *                   Chat Events                     *
	 *                                                   *
	 *****************************************************/
	/****** scroll the main text zone to the bottom ******/
	// FIXME : that should be put in JChatPane with InvokeLater()
	public void scroll_to_bottom()
	{
		if(login_completed && do_auto_scrolling.isSelected()) {
			Thread.yield();
			main_text.scrollRectToVisible(new Rectangle(0,
			 main_text.getHeight() - 2, 1, 1));
		}

		if(login_completed && do_bip_on_new_line.isSelected())
			bip();
	}

	/****************** a new user joined ****************/
	public void add_user(String name)
	{
		users.put(name, get_label_for(name));
		user_list.setListData(new Vector<JLabel>(users.values()));
		main_text.disp_message(name + " "+captions.getString("MSG_NEW_USER"));
		usernum_lbl.setText(captions.getString("MSG_USERS")+": " + Integer.toString(users.size()));

		if(do_notify_connection.isSelected())
			notify_user(captions.getString("MSG_NEW_CONNECTION"),
			 name + " " + captions.getString("MSG_NEW_USER"));
	}

	/******************* a user has quit *****************/
	public void remove_user(String name, String byemsg)
	{
		ChatPrivate child;

		users.remove(name);
		user_list.setListData(new Vector<JLabel>(users.values()));
		main_text.disp_message(name+" "+captions.getString("MSG_USER_QUIT")+" (" + byemsg +")");
		usernum_lbl.setText("Users: " + Integer.toString(users.size()));
		
		// Were we private chatting with him ?
		if((child = (ChatPrivate) pmsg_windows.get(name)) != null)
			child.correspondant_has_quit(byemsg);
	}

	/************ new ERROR message received *************/
	public void received_error(String error)
	{
		if(login_completed) {
			main_text.disp_error(captions.getString("MSG_ERROR") + " : " + error);
			scroll_to_bottom();
		} else {
			error_message.setText(error);
			error_message.repaint();
		}
	}

	/************ new TALK message received **************/
	public void received_talk(String nick, String txt)
	{
		main_text.disp_talk(nick, txt);
		scroll_to_bottom();
	}

	/*********** new server message received *************/
	public void received_server_msg(String txt)
	{
		main_text.disp_message(txt);
		scroll_to_bottom();
	}

	/*********** new ACTION message received *************/
	public void received_action(String nick, String txt)
	{
		main_text.disp_action(nick, txt);
		scroll_to_bottom();
	}

	/********** the server cut the connection ************/
	public void server_deconnection()
	{
		/* Notify the user */
		main_text.disp_message(captions.getString("MSG_DISCONNECTED"));
		scroll_to_bottom();

		/* disable the textfields */
		user_text.setEnabled(false);
		/* FIXME : do it for the private chat windows too 
		
		/* disable the private chat windows */
		if(login_completed) {
			Enumeration children = pmsg_windows.elements();

			while(children.hasMoreElements())
				((ChatPrivate) children.nextElement()).notify_deconnection();
		}
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
					com_thread.send_client_msg("TALK", n);
					main_text.disp_talk(nick, n);
					input_stack.add(n);
					orig = orig.substring(nlp + 1);
				}
				com_thread.send_client_msg("TALK", orig);
				main_text.disp_talk(nick, orig);
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
			com_thread.send_client_msg(com);
		} else {
			com=txt.substring(0,txt.indexOf(" ")).toUpperCase();
			txt=txt.substring(txt.indexOf(" ")+1);
			if(txt.indexOf(" ") == -1) {
				com_thread.send_client_msg(com, txt);
			} else {	
				com2=txt.substring(0,txt.indexOf(" "));
				txt=txt.substring(txt.indexOf(" ")+1);
				com_thread.send_client_msg(com, com2, txt);
			}

			/* If it's a private message, print it to the sender */
			if(com.equals("MSG") && com2 != null)
				main_text.disp_sent_pmsg(com2, txt);
		}


		/* If it's quit, we stop */
		if(com.equals("QUIT"))
			user_text.setEnabled(false);

	}

	/*********** auto nick completion with TAB ***********/
	private void nick_completion()
	{
		String to_complete, candidate, orig;
		int cp, sp;
		Enumeration all_nicks = users.keys();
		Vector<String> possible_nicks = new Vector<String>();


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

			main_text.disp_proposal(captions.getString("MSG_POSS_NICKS")+" : " + candidate);
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
		com_thread.send_client_msg("MSG", to, msg);
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

		if(src.equals(user_nick) || src.equals(go_login)) {
			request_login(user_nick.getText());
		}

		if(src.equals(cleartext))
			main_text.clear();

		if(src.equals(do_smileys_as_pics)) {
			main_text.set_smileys_as_pics(do_smileys_as_pics.isSelected());
			return;
		}

		if(src instanceof JMenuItem) {
			popupmenu_selected(evt.getActionCommand());
		}

		if(evt.getActionCommand().equals("options")) {
			option_menu.show(option_menu_trigger, 0, option_menu_trigger.getHeight());
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
			user_text.setCaretPosition(n.length());
		} else if(k_code == KeyEvent.VK_DOWN) {
			String n = input_stack.down();

			if(n != null) {
				user_text.setText(n);
				user_text.setCaretPosition(n.length());
			}
		}
	}

	/************ combo box (choice) changed *************/
	public void itemStateChanged(ItemEvent evt)
	{
		Object src = evt.getSource();
		
		if((src.equals(usr_fontsize) || src.equals(usr_fontname)) &&
		    evt.getStateChange() == ItemEvent.SELECTED) {
			Enumeration children;

			fnt = new Font((String) usr_fontname.getSelectedItem(), Font.PLAIN,
			               Integer.parseInt((String) usr_fontsize.getSelectedItem()));
			main_text.setFont(fnt);
			user_text.setFont(fnt);
			nick_lbl.setFont(fnt);
			a.validate();

			// Tells children windows to do so
			children = pmsg_windows.elements();
			while(children.hasMoreElements())
				((ChatPrivate) children.nextElement()).change_font(fnt);
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

	/************** click in the user list ***************/
	public void mousePressed(MouseEvent evt) {
		Object src = evt.getSource();
	
		/* Activate a pop-up menu if the click is on a nickname */
		if(src.equals(user_list) && login_completed) {
			if(user_list.locationToIndex(evt.getPoint()) != -1) {
				user_list.setSelectedIndex(user_list.locationToIndex(evt.getPoint()));
				usr_popup.show(user_list, evt.getX(), evt.getY());
			}
		}
	
	}

	/**************** unused event handlers **************/
	public void keyReleased(KeyEvent e) {}
	public void keyTyped(KeyEvent e) {}
	public void focusGained(FocusEvent e) {}
	public void mouseClicked(MouseEvent e) {}
	public void mouseEntered(MouseEvent e) {}
	public void mouseExited(MouseEvent e) {}
	public void mouseReleased(MouseEvent e) {}





	/*****************************************************
	 *                                                   *
	 *                    Utilities                      *
	 *                                                   *
	 *****************************************************/
	/**************** click on a button ******************/
	public void center_window(Window w)
	{
		Dimension screen = this.getToolkit().getScreenSize();

		/* Center the dialog on the screen */
		w.setBounds(screen.width/2 - w.getWidth()/2,
		            screen.height/2 - w.getHeight()/2,
					w.getWidth(), w.getHeight());
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
	
	/***************** chat text window ******************/
	public JChatPane get_chat_pane() { return main_text; }

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
