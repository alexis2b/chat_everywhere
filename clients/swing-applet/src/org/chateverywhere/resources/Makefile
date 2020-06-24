# Makefile for Chat Everywhere java-applet client resources
# Copyright(c) 2003-2005, A. de Bernis <alexis@bernis.org>


JAVAC=javac
JFLAGS=
ENCODER=native2ascii

TRANS=Messages_ca.java \
      Messages_cs.java \
      Messages_da.java \
      Messages_da_DK.java \
      Messages_de.java \
      Messages_es.java \
      Messages_fr.java \
      Messages_fr_FR.java \
      Messages_pl.java \
      Messages_ru.java \
      Messages_ru_RU.java


all: $(TRANS)

clean:
	rm -f *.class
	rm -f $(TRANS)


Messages_ca.java: Messages_ca.java.native
	$(ENCODER) -encoding 8859_1 Messages_ca.java.native Messages_ca.java
                                                                                                                             
Messages_cs.java: Messages_cs.java.native
	$(ENCODER) -encoding 8859_2 Messages_cs.java.native Messages_cs.java

Messages_da.java: Messages_da.java.native
	$(ENCODER) -encoding 8859_1 Messages_da.java.native Messages_da.java

Messages_da_DK.java: Messages_da_DK.java.native
	$(ENCODER) -encoding 8859_1 Messages_da_DK.java.native Messages_da_DK.java

Messages_de.java: Messages_de.java.native
	$(ENCODER) -encoding 8859_1 Messages_de.java.native Messages_de.java

Messages_es.java: Messages_es.java.native
	$(ENCODER) -encoding 8859_1 Messages_es.java.native Messages_es.java

Messages_fr.java: Messages_fr.java.native
	$(ENCODER) -encoding 8859_1 Messages_fr.java.native Messages_fr.java

Messages_fr_FR.java: Messages_fr_FR.java.native
	$(ENCODER) -encoding 8859_1 Messages_fr_FR.java.native Messages_fr_FR.java

Messages_pl.java: Messages_pl.java.native
	$(ENCODER) -encoding 8859_2 Messages_pl.java.native Messages_pl.java

Messages_ru.java: Messages_ru.java.native
	$(ENCODER) -encoding KOI8_R Messages_ru.java.native Messages_ru.java

Messages_ru_RU.java: Messages_ru_RU.java.native
	$(ENCODER) -encoding KOI8_R Messages_ru_RU.java.native Messages_ru_RU.java

