/* InputDialog
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

package org.chateverywhere;

import java.lang.*;
import java.awt.*;
import java.awt.event.*;


public class InputDialog extends Dialog implements ActionListener
{
	private TextField password;
	private Button ok, cancel;
	private boolean data_ok = false;
	private String data;

	public InputDialog(Frame owner, String title, String text,
	 Color fg_color, Color bg_color, Color c_fg_color, Color c_bg_color)
	{
		super(owner, title, true);
		
		draw_dialog(text, fg_color, bg_color, c_fg_color, c_bg_color);
	
		center_on_screen();
		this.setVisible(true);
		password.requestFocus();
	}


	private void draw_dialog(String text,
	 Color fg_color, Color bg_color, Color c_fg_color, Color c_bg_color)
	{
		Panel buttons;

		this.setLayout(new GridLayout(3, 1));
		
		this.setBackground(bg_color);
		this.setForeground(fg_color);
		
		ok = new Button("OK");
		 ok.addActionListener(this);
		 ok.setForeground(c_fg_color);
		 ok.setBackground(c_bg_color);
		cancel = new Button("Cancel");
		 cancel.addActionListener(this);
		 cancel.setForeground(c_fg_color);
		 cancel.setBackground(c_bg_color);
		password = new TextField(10);
		 password.setEchoChar('*');
		 password.addActionListener(this);
		 password.setForeground(c_fg_color);
		 password.setBackground(c_bg_color);
		buttons = new Panel();
		 buttons.add(ok);
		 buttons.add(cancel);
		
		this.add(new Label(text));
		this.add(password);
		this.add(buttons);

		setSize(new Dimension(250, 130));
		validate();
	}


	public void actionPerformed(ActionEvent evt)
	{
		if(evt.getSource().equals(ok) || evt.getSource().equals(password)) {
			data = password.getText();
			data_ok = true;
			dispose();
		}

		if(evt.getSource().equals(cancel)) {
			data = "";
			data_ok = false;
			dispose();
		}
	}

	public boolean is_data_ok()
	{
		return data_ok;
	}

	public String get_data()
	{
		return data;
	}

	private void center_on_screen()
	{
		Dimension screen;
		Dimension w = this.getSize();
		
//		try {
			screen = this.getToolkit().getScreenSize();
//		} catch(Exception e) {
			// getToolkit() can cause a security exception on IE
//			screen = new Dimension(800, 600);
//		}

		/* Center the dialog on the screen */
		this.setBounds(screen.width/2 - w.width/2,
                       screen.height/2 - w.height/2,
		               w.width, w.height);
	}




}


