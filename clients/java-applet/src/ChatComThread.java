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

public class ChatComThread extends Thread
{
	private Chat parent;
	private InetAddress address;
	private Socket sock;
	private BufferedReader from_server;
	private BufferedWriter for_server;
	private String received, my_nick;
	private ResourceBundle captions;

	ChatComThread(Chat parent, InetAddress address, Socket sock)
	{
		this.parent=parent;
		this.address=address;
		this.sock=sock;
		captions = parent.get_translated_messages();
		my_nick = parent.get_my_nick();

		try
		{
			from_server=new BufferedReader(new InputStreamReader(
			 sock.getInputStream()));
			for_server=new BufferedWriter(new OutputStreamWriter(
			 sock.getOutputStream()));
		}catch(Exception e)
		{
			return;
		}
	}

	public void run()
	{
		try {
			while(true) {
				received=from_server.readLine();

				if(received==null)
					return;

				dispatch_msg(received);
			}
		}catch(Exception e) {
			return;
		}
	}

	public void dispatch_msg(String msg)
	{
		String tmp;
		
		msg = msg.substring(msg.indexOf("<|>")+4).trim();

//		parent.disp_message("*** DEBUG : Dispatching message "+msg+"\n");

		if(msg.startsWith("TALK <|>")) {
			msg=msg.substring(msg.indexOf("<|>")+4).trim();
			tmp=msg.substring(msg.indexOf("<|>")+4).trim();
			
			msg=msg.substring(0,msg.indexOf("<|>")).trim();
			tmp=tmp.substring(0,tmp.indexOf("<|>")).trim();
			
			if(!msg.equals(my_nick))
				parent.disp_message("<"+msg+"> "+tmp+"\n");

			return;
		}

		if(msg.startsWith("ACTION <|>")) {
			msg=msg.substring(msg.indexOf("<|>")+4).trim();
			tmp=msg.substring(msg.indexOf("<|>")+4).trim();
			
			msg=msg.substring(0,msg.indexOf("<|>")).trim();
			tmp=tmp.substring(0,tmp.indexOf("<|>")).trim();
			
			parent.disp_message("* "+msg+" "+tmp+"\n");
			return;
		}

		if(msg.startsWith("CONNECTION <|>")) {
			msg=msg.substring(msg.indexOf("<|>")+4).trim();
			parent.add_user(msg.substring(0,msg.indexOf("<|>")).trim());
		}
		
		if(msg.startsWith("DECONNECTION <|>")) {
			msg=msg.substring(msg.indexOf("<|>")+4).trim();
			tmp=msg.substring(msg.indexOf("<|>")+4).trim();
			
			msg=msg.substring(0,msg.indexOf("<|>")).trim();
			tmp=tmp.substring(0,tmp.indexOf("<|>")).trim();
			parent.remove_user(msg, tmp);
		}

		if(msg.startsWith("ERROR <|>")) {
			msg=msg.substring(msg.indexOf("<|>")+4).trim();
			parent.disp_message("*** "+captions.getString("MSG_ERROR")+" : "+msg.substring(0,msg.indexOf("<|>"))+"\n");
		}

		if(msg.startsWith("MSG <|>")) {
			msg=msg.substring(msg.indexOf("<|>")+4).trim();
			parent.disp_message("** "+msg.substring(0,msg.indexOf("<|>"))+"\n");
		}

		if(msg.startsWith("PING")) {
			tmp = msg.substring(msg.indexOf("<|>")+4).trim();
			tmp = tmp.substring(0,tmp.indexOf("<|>")).trim();
			send_msg("CLIENT <|> PONG <|> " + tmp + " <|> CLIENT");
		}

		if(msg.startsWith("PMSG")) {
			msg=msg.substring(msg.indexOf("<|>")+4).trim();
			tmp=msg.substring(msg.indexOf("<|>")+4).trim();
			
			msg=msg.substring(0,msg.indexOf("<|>")).trim();
			tmp=tmp.substring(0,tmp.indexOf("<|>")).trim();
			
			parent.disp_private_msg(msg, tmp);
			return;
		}
	}

	public void send_msg(String msg)
	{
		try {
			for_server.write(msg+"\n");
			for_server.flush();
		}catch(Exception e) {
			System.out.println("Error :: send_msg() : " + e);
			e.printStackTrace();
		}
	}

	public void send_message(String nick, String text)
	{
		try
		{
			for_server.write("CLIENT <|> TALK <|> " + text + " <|> CLIENT\n");
			for_server.flush();
		} catch(Exception e)
		{
			return;
		}
	}

	public void finish_all()
	{
		try
		{
			sock.close();
		}catch(Exception e)
		{
			return;
		}
	}
}
