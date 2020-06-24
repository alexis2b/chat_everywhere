/* MyTextField
 * simple JTextField that can receive a TAB key
 */  


package org.chateverywhere;

import javax.swing.JTextField;

public class MyTextField extends JTextField {

	public boolean isManagingFocus() {
		return true;
	}
}
