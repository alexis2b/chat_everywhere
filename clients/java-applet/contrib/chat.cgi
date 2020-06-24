#!/bin/sh

# If anything below fails at least the header will be shown.
echo "Content-Type: text/html"
echo

# Get parameters.
IFS='&'
for i in ${QUERY_STRING}; do
    case $i in
	nick=*)	CHAT_NICK=$(echo $i | cut -d '=' -f 2)
    esac
done    

# If no nick given...
[ -z ${CHAT_NICK} ] &&
    {
    # ...then show the form.
    echo "<form action=\"$(basename $0)\" method=\"GET\">"
    echo " <input name=\"nick\"/>"
    echo " <input type=\"submit\" value=\"Enter\"/>"
    echo "</form>"
    } \
||
# Else,
    {
    # return HTML code that will run the applet.
    echo "<applet archive=\"ChatEverywhere.jar\" code=\"Chat.class\" width=\"600\" height=\"400\">"
    echo " <param name=\"Nick\" value=\"${CHAT_NICK}\"/>"
    echo " <param name=\"ServerPort\" value=\"5656\"/>"
    echo "</applet>"
    }
