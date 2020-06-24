
package org.chateverywhere;


import java.awt.*;
import javax.swing.*;


public class SimpleLabelCellRenderer implements ListCellRenderer
{

	public Component getListCellRendererComponent(
	 JList list,
	 Object value,
	 int index,
	 boolean isSelected,
	 boolean cellHasFocus)
	{
		JLabel me = (JLabel) value;
		
		//me.setOpaque(true);	// dedicated to the parent...
		
		if(isSelected) {
			me.setForeground(list.getSelectionForeground());
			me.setBackground(list.getSelectionBackground());
		} else {
			me.setForeground(list.getForeground());
			me.setBackground(list.getBackground());
		}

		return me;
	}
}
