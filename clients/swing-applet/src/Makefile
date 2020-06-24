# Makefile for Chat Everywhere java-applet client
# Copyright(c) 2003-2005, A. de Bernis <alexis@bernis.org>


# Tools that we use (standard Sun JDK)
# uncomment the second JFLAGS line for non-developper build
JAVAC=javac
JFLAGS=-target 1.3 -deprecation
#JFLAGS=-O -g:none
JAR=jar

OBJS=org/chateverywhere/ChatMessage.class \
     org/chateverywhere/InputStack.class \
     org/chateverywhere/JChatPaneScroller.class \
     org/chateverywhere/JChatPane.class \
     org/chateverywhere/MD5.class \
     org/chateverywhere/MD5State.class \
     org/chateverywhere/MyTextField.class \
     org/chateverywhere/SimpleLabelCellRenderer.class \
     ChatComThread.class \
     ChatPrivate.class \
     Chat.class

SECIE=com/ms/security/PolicyEngine.class \
      com/ms/security/PermissionID.class

SECNS=netscape/security/PrivilegeManager.class

RESOURCES=org/chateverywhere/resources/Messages.class \
          org/chateverywhere/resources/Messages_ca.class \
          org/chateverywhere/resources/Messages_cs.class \
          org/chateverywhere/resources/Messages_da_DK.class \
          org/chateverywhere/resources/Messages_de.class \
          org/chateverywhere/resources/Messages_en.class \
          org/chateverywhere/resources/Messages_en_GB.class \
          org/chateverywhere/resources/Messages_en_US.class \
          org/chateverywhere/resources/Messages_es.class \
          org/chateverywhere/resources/Messages_fr.class \
	  org/chateverywhere/resources/Messages_fr_FR.class \
          org/chateverywhere/resources/Messages_pl.class \
          org/chateverywhere/resources/Messages_ru.class \
          org/chateverywhere/resources/Messages_ru_RU.class \
	  org/chateverywhere/resources/sounds/beep.au \
	  org/chateverywhere/resources/pics/happy.gif \
	  org/chateverywhere/resources/pics/very_happy.gif \
	  org/chateverywhere/resources/pics/neutral.gif \
	  org/chateverywhere/resources/pics/wink.gif


.SUFFIXES: .java .class
.java.class:
	$(JAVAC) $(JFLAGS) $<


all: encode $(RESOURCES) $(OBJS) $(SECIE) $(SECNS) archive
	@echo ""
	@echo "==== compilation successful, result in ../build/ ===="
	@echo ""


encode:
	@cd org/chateverywhere/resources && make


archive:
	$(JAR) -cf ChatEverywhere-swing.jar $(OBJS) $(SECIE) $(SECNS) $(RESOURCES) 'Chat$$1.class'
	mv ChatEverywhere-swing.jar ../build


clean:
	@cd org/chateverywhere/resources && make clean
	rm -f $(OBJS) $(SECIE) $(SECNS) 'Chat$$1.class'


realclean: clean
	rm -f ../build/ChatEverywhere-swing.jar
	rm -f ../build/ChatEverywhere-swing-signed-sun.jar
	rm -f ../build/ChatEverywhere-swing-signed-ie.cab


sign: all
	cp ../build/ChatEverywhere-swing.jar ChatEverywhere-swing-signed-sun.jar
#	echo "jarsigner -keystore /home/alexis/.java/alexisb ChatEverywhere-swing-signed-sun.jar signFiles"
	jarsigner -keystore /home/alexis/.java/chateverywhere ChatEverywhere-swing-signed-sun.jar chateverywhere
	mv ChatEverywhere-swing-signed-sun.jar ../build
	@echo ""
	@echo "==== signing successful, result in ../build/ ===="
	@echo ""

