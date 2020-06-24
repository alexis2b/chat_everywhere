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
/* JChatPane
 * styled text area designed for Chat
 */


package org.chateverywhere;

import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.util.*;


public class JChatPane extends JTextPane
{
	private SimpleAttributeSet set_default;
	private SimpleAttributeSet set_nick;
	private SimpleAttributeSet set_my_nick;
	private SimpleAttributeSet set_highlight_nick;
	private SimpleAttributeSet set_symbols;
	private SimpleAttributeSet set_my_symbols;
	private SimpleAttributeSet set_talk;
	private SimpleAttributeSet set_action;
	private SimpleAttributeSet set_msg;
	private SimpleAttributeSet set_error;
	private SimpleAttributeSet set_proposal;
	private Document doc;
	private JChatPaneScroller parent;
	private String my_nick;
	private boolean smileys_as_pics;
	private Hashtable smileys;

	public JChatPane(JChatPaneScroller parent, String my_nick, boolean smileys_as_pics)
	{
		super();
		this.parent = parent;
		doc = getStyledDocument();
		this.my_nick = my_nick;
		this.smileys_as_pics = smileys_as_pics;

		/* Initialize the styles */
		set_default = new SimpleAttributeSet();
			StyleConstants.setForeground(set_default, Color.black);
			StyleConstants.setItalic(set_default, false);
			StyleConstants.setBold(set_default, false);

		set_nick = new SimpleAttributeSet(set_default);
			StyleConstants.setForeground(set_nick, Color.black);
			StyleConstants.setBold(set_nick, true);

		set_my_nick = new SimpleAttributeSet(set_default);
			StyleConstants.setForeground(set_my_nick, Color.decode("#C00000"));
			StyleConstants.setBold(set_my_nick, true);

		set_highlight_nick = new SimpleAttributeSet(set_default);
			StyleConstants.setForeground(set_highlight_nick, Color.decode("#EEEE00"));
			StyleConstants.setBold(set_highlight_nick, true);

		set_symbols = new SimpleAttributeSet(set_nick);
			StyleConstants.setBold(set_symbols, true);

		set_my_symbols = new SimpleAttributeSet(set_nick);
			StyleConstants.setForeground(set_my_symbols, Color.gray);

		set_talk = new SimpleAttributeSet(set_default);

		set_action = new SimpleAttributeSet(set_default);
			StyleConstants.setForeground(set_action, Color.green.darker());
//			StyleConstants.setItalic(set_action, true);

		set_msg = new SimpleAttributeSet(set_default);
			StyleConstants.setForeground(set_msg, Color.blue);

		set_error = new SimpleAttributeSet(set_default);
			StyleConstants.setForeground(set_error, Color.red);
			StyleConstants.setBold(set_error, true);

		set_proposal = new SimpleAttributeSet(set_default);
			StyleConstants.setForeground(set_proposal, Color.lightGray);

	}


	public void disp_talk(String from, String msg)
	{
		SimpleAttributeSet symbols, nick;
		
		
		if(my_nick.equals(from)) { /* We are talking */
			symbols = set_my_symbols;
			nick = set_my_nick;

		} else if(msg.toLowerCase().indexOf(my_nick.toLowerCase()) != -1) {
			/* Somebody is talking to us */
			symbols = set_symbols;
			nick = set_highlight_nick;
		
		} else {
			/* normal message */
			symbols = set_symbols;
			nick = set_nick;
		}


		try {
			doc.insertString(doc.getLength(), "<", symbols);
			doc.insertString(doc.getLength(), from, nick);
			doc.insertString(doc.getLength(), "> ", symbols);

			/* Do we need to pay attention to smileys */
			if(smileys_as_pics) {
				if(smileys == null)
					load_smileys();
				print_smiley_msg(msg + "\n");
			} else {
				doc.insertString(doc.getLength(), msg + "\n", set_talk);
			}
		} catch (Exception e) {
			System.out.println("doc.insertString error !");
			System.out.println(e);
		}

		parent.scroll_to_bottom();
	}
	
	
	public void disp_message(String msg)
	{
		try {
			doc.insertString(doc.getLength(), "** " + msg + "\n", set_msg);
		} catch (Exception e) { }

		parent.scroll_to_bottom();
	}
	
	
	public void disp_error(String msg)
	{
		try {
			doc.insertString(doc.getLength(), "*** " + msg + "\n", set_error);
		} catch (Exception e) { }

		parent.scroll_to_bottom();
	}
	
	public void disp_action(String from, String msg)
	{
		try {
			doc.insertString(doc.getLength(), "* " + from + " " + msg + "\n", set_action);
		} catch (Exception e) { }

		parent.scroll_to_bottom();
	}

	public void disp_sent_pmsg(String to, String msg)
	{
		try {
			doc.insertString(doc.getLength(), ">", set_symbols);
			doc.insertString(doc.getLength(), to, set_nick);
			doc.insertString(doc.getLength(), "< ", set_symbols);
			doc.insertString(doc.getLength(), msg + "\n", set_talk);
		} catch (Exception e) { }

		parent.scroll_to_bottom();
	}

	/********** print a nick completion proposal *********/
	public void disp_proposal(String msg)
	{
		try {
			doc.insertString(doc.getLength(), "++ " + msg + "\n", set_proposal);
		} catch (Exception e) { }

		parent.scroll_to_bottom();
	}

	/***************** clear the chat text ***************/
	public void clear() { this.setText(""); }


	/***************** clear the chat text ***************/
	private void load_smileys()
	{
		smileys = new Hashtable();

		smileys.put(":-)", new ImageIcon(getClass().getResource(
		 "resources/pics/happy.gif")));
		smileys.put(":->", new ImageIcon(getClass().getResource(
		 "resources/pics/very_happy.gif")));
		smileys.put(":-|", new ImageIcon(getClass().getResource(
		 "resources/pics/neutral.gif")));
		smileys.put(";-)", new ImageIcon(getClass().getResource(
		 "resources/pics/wink.gif")));
	}

	/******* recursively print a msg with smileys ********/
	private void print_smiley_msg(String msg)
	 throws Exception
	{
		Enumeration smileys_txt = smileys.keys();
		String smiley_cur;
		int p;
		
		while(smileys_txt.hasMoreElements()) {

			smiley_cur = (String) smileys_txt.nextElement();
			
			if((p = msg.indexOf(smiley_cur)) != -1) {

				print_smiley_msg(msg.substring(0, p));

				/* Brain-dead method to add an icon in a JTextPane... */
				setCaretPosition(doc.getLength());
				setEditable(true);
				insertIcon((Icon) smileys.get(smiley_cur));
				setEditable(false);

				print_smiley_msg(msg.substring(p + smiley_cur.length()));
				
				return;
			}
		}

		/* No smiley found */
		doc.insertString(doc.getLength(), msg, set_talk);
	}


	/*****************************************************
	 *                                                   *
	 *                    Accessors                      *
	 *                                                   *
	 *****************************************************/
	/**************** chat pane parameters ***************/
	public void set_my_nick(String nick) { my_nick = nick; }
	public void set_smileys_as_pics(boolean s) { smileys_as_pics = s; }
	public boolean get_smileys_as_pics() { return smileys_as_pics; }
}
