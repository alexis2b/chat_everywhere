# Makefile for Chat Everywhere java-applet client
# Copyright(c) 2003-2005, A. de Bernis <alexis@bernis.org>


JAVAC=javac
JFLAGS=-target 1.1 -deprecation
#JFLAGS=-O -g:none
JAR=jar

OBJS=org/chateverywhere/InputDialog.class \
     org/chateverywhere/InputStack.class \
     org/chateverywhere/MD5.class \
     org/chateverywhere/MD5State.class \
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
          org/chateverywhere/resources/sounds/beep.au


.SUFFIXES: .java .class
.java.class:
	$(JAVAC) $(JFLAGS) $<


all: encode $(RESOURCES) $(OBJS) $(SECIE) $(SECNS) archive
	@echo ""
	@echo "==== compilation successful, result in ../build/ ===="
	@echo ""


encode:
	@cd org/chateverywhere/resources && make


archive: ../build/ChatEverywhere.jar
	$(JAR) -cf ChatEverywhere.jar $(OBJS) $(SECIE) $(SECNS) $(RESOURCES) 'Chat$$1.class'
	mv ChatEverywhere.jar ../build


clean:
	@cd org/chateverywhere/resources && make clean
	rm -f $(OBJS) $(SECIE) $(SECNS) 'Chat$$1.class'


realclean: clean
	rm -f ../build/ChatEverywhere.jar
	rm -f ../build/ChatEverywhere-signed-sun.jar
	rm -f ../build/ChatEverywhere-signed-ie.cab


sign: all
	cp ../build/ChatEverywhere.jar ChatEverywhere-signed-sun.jar
	jarsigner -keystore /home/alexis/.java/chateverywhere ChatEverywhere-signed-sun.jar chateverywhere
	mv ChatEverywhere-signed-sun.jar ../build
	@echo ""
	@echo "==== signing successful, result in ../build/ ===="
	@echo ""

