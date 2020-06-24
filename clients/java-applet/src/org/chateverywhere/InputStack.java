/* InputStack
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

import java.util.*;

public class InputStack
{
	private int max_elements_in_stack = 30;
	private int input_stack_pos;
	private Vector input_stack;

	public InputStack()
	{
		input_stack = new Vector(max_elements_in_stack);
		input_stack_pos = 0;
	}
	
	public InputStack(int max)
	{
		max_elements_in_stack = max;
	}

	/******************* Add an entry ********************/
	public void add(String entry)
	{
		if(input_stack.size() == max_elements_in_stack)
			input_stack.removeElementAt(max_elements_in_stack - 1);

		input_stack.insertElementAt(entry, 0);
		input_stack_pos = 0;
	}


	/**************** Call previous entry ****************/
	public String up()
	{
		if(input_stack_pos < input_stack.size())
			return (String) input_stack.elementAt(input_stack_pos++);

		return (String) input_stack.elementAt(input_stack_pos - 1);
	}

	/****************** Call next entry ******************/
	public String down()
	{
		if(input_stack_pos == 0) {
			return null;
		} else if(input_stack_pos == 1) {
			input_stack_pos = 0;
			return "";
		}

		return (String) input_stack.elementAt(--input_stack_pos - 1);
	}
}
