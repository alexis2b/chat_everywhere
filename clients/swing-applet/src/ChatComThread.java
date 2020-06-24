/* Chat
 * Copyright (C) 1998-2004 Alexis de Bernis
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
 *
 */

import java.io.*;
import java.net.*;
import java.util.*;
import org.chateverywhere.*;

public class ChatComThread extends Thread
{
	private Chat parent;

	private String server_host;
	private int server_port;
	private Socket sock;
	private BufferedReader from_server;
	/* private DataOutputStream for_server; */
	private BufferedWriter for_server;

	private String received, my_nick;
	private ResourceBundle captions;

	private boolean keep_running;

	/* connection state */
	private int connection_state;
	public static final int NOT_CONNECTED = 0;
	public static final int CONNECTION_IN_PROGRESS = 1;
	public static final int CONNECTED = 2;
	public static final int LOGGED = 3;
	public static final int CONNECTION_FAILED = 99;
	private String fail_reason;




	/*****************************************************
	 *                                                   *
	 *         Initialization / Finalization             *
	 *                                                   *
	 *****************************************************/
	/****************** Object Creation ******************/
	ChatComThread(Chat parent, String server, int port)
	{
		this.parent = parent;
		this.server_host = server;
		this.server_port = port;
		captions = parent.get_translated_messages();
		my_nick = " ";  // Not set yet

		/* variable initialisations */
		keep_running = true;
		connection_state = NOT_CONNECTED;
	}

	/******************** Finalization *******************/
	public void finish_all()
	{
		keep_running = false;

		try {
			sock.close();
		}catch(Exception e) {
			return;
		}
	}


	/*****************************************************
	 *                                                   *
	 *               Main receiving loop                 *
	 *                                                   *
	 *****************************************************/
	/************* top-level input function **************/
	public void run()
	{
		ChatMessage msg;

		// First, we have to make the connection
		if(!connect_to_server())
			return;

		try {
			while(keep_running) {
				received = from_server.readLine();

				if(received == null) {
					notify_deconnection();
					return;
				}
		
				/* parses the message */
				msg = new ChatMessage(received);
				if(!msg.is_valid())
					System.out.println("Invalid msg received = %" + received + "%");
				else
					dispatch_command(msg);
			}
		}catch(Exception e) {
			notify_deconnection();
			return;
		}
	}







	/*****************************************************
	 *                                                   *
	 *                Server operations                  *
	 *                                                   *
	 *****************************************************/
	/********** connection to the chat server ************/
	private boolean connect_to_server()
	{
		InetAddress server_address;

		connection_state = CONNECTION_IN_PROGRESS;

		// Try to request more permissions in case we are signed
		try {
			netscape.security.PrivilegeManager.enablePrivilege(
			 "UniversalConnect");
		} catch (Exception ex) {
			System.out.println(captions.getString("MSG_NO_PRIV_NS"));
		}
		try {
		    com.ms.security.PolicyEngine.checkPermission(
			 com.ms.security.PermissionID.NETIO);
			com.ms.security.PolicyEngine.assertPermission(
			 com.ms.security.PermissionID.NETIO);
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
			InetAddress [] addresses = InetAddress.getAllByName(server_host);

			/* Choose a random ip address in all the possibles */
			server_address = addresses[(int) Math.floor(Math.random() * addresses.length)];

		} catch(UnknownHostException e) {
			System.out.println(e);
			System.out.println(server_host);
			fail_reason = captions.getString("MSG_NOT_FOUND");
			connection_state = CONNECTION_FAILED;
			return false;

		} catch(SecurityException e) {
			fail_reason = captions.getString("MSG_NOT_SIGNED");
			connection_state = CONNECTION_FAILED;
			return false;
		}

		System.out.println("* " + captions.getString("MSG_CONN")+" " + server_address + ":" + server_port);

		try
		{ 
			sock=new Socket(server_address, server_port);
			from_server = new BufferedReader(new InputStreamReader(
			 sock.getInputStream()));
			/* for_server = new DataOutputStream(sock.getOutputStream()); */
			for_server = new BufferedWriter(new OutputStreamWriter(
			 sock.getOutputStream()));
		}catch(Exception e)
		{
			fail_reason = captions.getString("MSG_SRV_DOWN");
			connection_state = CONNECTION_FAILED;
			return false;
		}

		/* we did it... */
		connection_state = CONNECTED;

		return true;
	}

	private void notify_deconnection()
	{
		connection_state = NOT_CONNECTED;
		System.out.println("-- disconnected --");

		parent.server_deconnection();
	}





	/*****************************************************
	 *                                                   *
	 *             Input / Output functions              *
	 *                                                   *
	 *****************************************************/
	/************* top-level output function *************/
	private void send_to_server(String msg)
	{
		try {
			for_server.write("CLIENT <|> " + msg + " <|> CLIENT\n");
			for_server.flush();
		} catch(Exception e) {
			notify_deconnection();
			return;
		}
	}

	/*********** send a command to the server ************/
	public void send_client_msg(String cmd)
	{
		send_to_server(cmd);
	}	
	public void send_client_msg(String cmd, String arg1)
	{
		send_to_server(cmd + " <|> " + arg1);
	}
	public void send_client_msg(String cmd, String arg1, String arg2)
	{
		send_to_server(cmd + " <|> " + arg1 + " <|> " + arg2);
	}
	public void send_client_msg(String cmd, String arg1, String arg2, String arg3)
	{
		send_to_server(cmd + " <|> " + arg1 + " <|> " + arg2 + " <|> " + arg3);
	}


	/*****************************************************
	 *                                                   *
	 *                  Server commands                  *
	 *                                                   *
	 *****************************************************/
	/*************** main dispatch function **************/
	private void dispatch_command(ChatMessage msg)
	{
		String cmd = msg.get_command();

		if(cmd.equals("TALK"))
			dispatch_talk_cmd(msg);
		else if(cmd.equals("MSG"))
			dispatch_msg_cmd(msg);
		else if(cmd.equals("PING"))
			dispatch_ping_cmd(msg);
		else if(cmd.equals("ERROR"))
			dispatch_error_cmd(msg);
		else if(cmd.equals("ACTION"))
			dispatch_action_cmd(msg);
		else if(cmd.equals("CONNECTION"))
			dispatch_connection_cmd(msg);
		else if(cmd.equals("DECONNECTION"))
			dispatch_deconnection_cmd(msg);
		else if(cmd.equals("PMSG"))
			dispatch_pmsg_cmd(msg);
		else if(cmd.equals("USERS"))
			dispatch_users_cmd(msg);
		else if(cmd.equals("AUTH"))
			dispatch_auth_cmd(msg);
		else if(cmd.equals("LOGIN OK"))
			dispatch_login_ok_cmd(msg);
		else
			System.out.println("Unknown msg received : " + msg);
	}

	/********* somebody talked in the main chat **********/
	private void dispatch_talk_cmd(ChatMessage msg)
	{
		String nick;
		
		if(msg.get_args_num() != 2) {
			System.out.println("Invalid TALK msg : " + msg);
			return;
		}
		
		nick = msg.get_arg(1);
		if(!nick.equals(my_nick)) {
			parent.received_talk(nick, msg.get_arg(2));
		}
	}

	/********** somebody sent an action message **********/
	private void dispatch_action_cmd(ChatMessage msg)
	{
		if(msg.get_args_num() != 2) {
			System.out.println("Invalid ACTION msg : " + msg);
			return;
		}

		parent.received_action(msg.get_arg(1), msg.get_arg(2));
	}

	/**************** new user on the chat ***************/
	private void dispatch_connection_cmd(ChatMessage msg)
	{
		if(msg.get_args_num() != 1) {
			System.out.println("Invalid CONNECTION msg : " + msg);
			return;
		}
		
		parent.add_user(msg.get_arg(1));
	}

	/************** a user has quit the chat *************/
	private void dispatch_deconnection_cmd(ChatMessage msg)
	{
		if(msg.get_args_num() != 2) {
			System.out.println("Invalid DECONNECTION msg : " + msg);
			return;
		}
		
		parent.remove_user(msg.get_arg(1), msg.get_arg(2));
	}


	/*********** error message from the server ***********/
	private void dispatch_error_cmd(ChatMessage msg)
	{
		if(msg.get_args_num() != 1) {
			System.out.println("Invalid ERROR msg : " + msg);
			return;
		}

		parent.received_error(msg.get_arg(1));
	}

	/************ server information message *************/
	private void dispatch_msg_cmd(ChatMessage msg)
	{
		if(msg.get_args_num() != 1) {
			System.out.println("Invalid MSG msg : " + msg);
			return;
		}

		parent.received_server_msg(msg.get_arg(1));
	}

	/*************** ping from the server ****************/
	private void dispatch_ping_cmd(ChatMessage msg)
	{
		if(msg.get_args_num() != 1) {
			System.out.println("Invalid PING msg : " + msg);
			return;
		}
		
		send_client_msg("PONG", msg.get_arg(1));
	}

	/***************** private message *******************/
	private void dispatch_pmsg_cmd(ChatMessage msg)
	{
		if(msg.get_args_num() != 2) {
			System.out.println("Invalid PMSG msg : " + msg);
			return;
		}
		
		parent.disp_private_msg(msg.get_arg(1), msg.get_arg(2));
	}

	/*************** login request is ok *****************/
	private void dispatch_login_ok_cmd(ChatMessage msg)
	{
		if(msg.get_args_num() != 0) {
			System.out.println("Invalid LOGIN OK msg : " + msg);
			return;
		}

		connection_state = LOGGED;
		parent.login_ok();
	}

	/************ We receive the users list **************/
	private void dispatch_users_cmd(ChatMessage msg)
	{
		parent.refresh_user_list(msg);
	}

	/************** we must send a password **************/
	private void dispatch_auth_cmd(ChatMessage msg)
	{
		parent.check_auth(msg);	
	}



	/*****************************************************
	 *                                                   *
	 *                    Accessors                      *
	 *                                                   *
	 *****************************************************/
	public int get_connection_state() { return connection_state; }
	public String get_fail_reason() { return fail_reason; }
	public void set_my_nick(String nick) { my_nick = nick; }

}
