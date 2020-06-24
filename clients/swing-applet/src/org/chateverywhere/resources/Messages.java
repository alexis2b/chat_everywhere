/* en_US property file for Chat Everywhere
 * Copyright (c) 2003, A. de Bernis <alexis@bernis.org>
 */


package org.chateverywhere.resources;

import java.util.ListResourceBundle;

public class Messages extends ListResourceBundle {

	public Object[][] getContents() {
		return contents;
	}
 
	static final Object[][] contents = {
		{"MSG_USERS",			"Users"},
		{"MSG_TYPE_HERE",		"<--Type here"},
		{"MSG_CLEAR",			"Clear"},
		{"MSG_FONT",			"Font"},
		{"MSG_ACTION",			"Action"},
		{"MSG_WELCOME",			"Welcome to Chat Everywhere"},
		{"MSG_LOADING",			"Loading..."},
		{"MSG_GO",				"Go"},
		{"MSG_NICK",			"Nick"},
		{"MSG_SERVER_UP",		"The server is up"},
		{"MSG_CONNECTING",		"Connecting..."},
		{"MSG_NO_PRIV_NS",		"Failed to get some more privileges (Netscape)."},
		{"MSG_NO_PRIV_IE",		"Failed to get some more privileges (Internet Explorer)."},
		{"MSG_NOT_FOUND",		"Connection failed : server not found (check ServerHost)"},
		{"MSG_NOT_SIGNED",		"Connection failed : applet needs to be signed to connect to this host"},
		{"MSG_CONN",			"Connecting to server"},
		{"MSG_SRV_DOWN",		"Connection failed : server must be down."},
		{"MSG_NEW_USER",		"joined the chat"},
		{"MSG_USER_QUIT",		"has quit"},
		{"MSG_POSS_NICKS",		"Possible nicks"},
		{"MSG_CHAT",			"Chat with"},
		{"MSG_ERROR",			"ERROR"},
		{"MSG_AUTH_FAILED",		"Authentication failed: choose a non-protected nick"},
		{"MSG_NICK_PROTECTED",	"Nick protected"},
		{"MSG_ASK_PASSWORD",	"Enter your password:"},
		{"MSG_OK",				"OK"},
		{"MSG_CANCEL",			"Cancel"},
		{"MSG_NEW_CONNECTION",	"New user connected"},
		{"MSG_AUTO_SCROLLING",	"Auto-scrolling"},
		{"MSG_NOTIFY_CONNECT",	"Notify new connection"},
		{"MSG_BIP_ON_TEXT",		"Bip on new text"},
		{"MSG_OPTIONS",			"Options"},
		{"MSG_BIP_AS_SOUND_FILE","Play bip as a sound file"},
		{"MSG_SMILEYS_AS_PICS", "Display smileys as pics"},
		{"MSG_DISCONNECTED",	"disconnected"},
	};
}
