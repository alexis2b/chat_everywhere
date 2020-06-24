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
/*
 * ChatMessage
 * parses a chat server message for easy use
 */

package org.chateverywhere;

import java.util.*;


public class ChatMessage
{
	private String server_msg;
	private String emitter;
	private String command;
	private Vector args;
	private int current_arg = 0;
	private boolean valid_msg;
	private static String delimiter = "<|>";
	private static int del_len = delimiter.length();

	public ChatMessage(String server_msg)
	{
		this.server_msg = server_msg;
		args = new Vector();
		
		valid_msg = parse_message(server_msg);
	}


	private boolean parse_message(String msg)
	{
		int pos;
		int pos_s;
		int cur_arg = 0;
		
		msg = msg.trim();

		/*** get the emitter of the message ***/
		if(msg.startsWith("SERVER") && msg.endsWith("SERVER"))
			emitter = "SERVER";
		else if(msg.startsWith("CLIENT") && msg.endsWith("CLIENT"))
			emitter = "CLIENT";
		else
			return false;


		/*** take off the emitter at the beginning and at the end ***/
		if((pos = msg.indexOf(delimiter)) == -1)
			return false;
		
		msg = msg.substring(pos + del_len).trim();
		if((pos = msg.lastIndexOf(delimiter)) == -1)
			return false;
		msg = msg.substring(0, pos).trim();


		/*** get the command of the message ***/
		if((pos = msg.indexOf(delimiter)) == -1) {
			/*** there may be no args ***/
			if(!msg.equals("")) {
				command = msg;
				return true;
			}
			return false;
		}

		command = msg.substring(0, pos).trim();
		msg = msg.substring(pos + del_len).trim();

		while((pos = msg.indexOf(delimiter)) != -1) {
			/*** get the argument ***/
			current_arg++;
			args.add(msg.substring(0, pos).trim());
			msg = msg.substring(pos + del_len).trim();
		}
		current_arg++;
		args.add(msg);

		return true;
	}


	/* Returns a text-representation of the msg */
	public String toString()
	{
		int i;
		String res = "msg = %" + server_msg + "%\n"
		       + "valid = " + valid_msg + "\n"
			   + "emitter = %" + emitter + "%\n"
			   + "command = %" + command + "%\n"
			   + "num_args = " + current_arg + "\n";
	   
		for(i = 0; i < current_arg; i++)
			res = res + "arg(" + i + ") = %" + args.elementAt(i) + "%\n";

		return res;
	}




	/*****************************************************
	 *                                                   *
	 *                    Accessors                      *
	 *                                                   *
	 *****************************************************/
	/************** message characteristics **************/
	public boolean is_valid() { return valid_msg; }
	public String get_emitter() { return emitter; }
	public String get_command() { return command; }
	public int get_args_num() { return args.size(); }
	public String get_arg(int i) { return (String) args.elementAt(i-1); }
}
