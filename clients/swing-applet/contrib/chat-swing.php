<?php

    $chat_cookie_prefix = "CHAT_";
    $chat_cookie_expires = "0"; // delete when browser is closed

    // get nick from cookie or, preferably, via GET
    if(!$nick = $HTTP_GET_VARS["nick"]) $nick = $HTTP_COOKIE_VARS[$chat_cookie_prefix."NICK"];
	else setcookie($chat_cookie_prefix."NICK","$nick",$chat_cookie_expires);

    // if no nick given...
    if(!$nick)
	{
	// ...then show the form.
	echo("<form action=\"".basename($HTTP_SERVER_VARS['SCRIPT_NAME'])."\" method=\"GET\">\n");
	echo(" <input name=\"nick\"/>\n");
	echo(" <input type=\"submit\" value=\"Enter\"/>\n");
	echo("</form>\n");
	}
    // else,
    else
	{
	// return HTML code that will run the applet.
        echo("<applet archive=\"ChatEverywhere-swing.jar\" code=\"Chat.class\" width=\"600\" height=\"400\">");
        echo(" <param name=\"Nick\" value=\"${CHAT_NICK}\"/>");
        echo(" <param name=\"ServerPort\" value=\"5656\"/>");
        echo("</applet>");
	}

?>
