LCFLAGS=`pkg-config --cflags --libs cairo`
LIBS=-lfl

all: parser lexer present

parser: parser.y
	bison $(BISONFLAGS) -d -opresent.tab.c parser.y

lexer: lexer.l present.tab.c present.tab.h
	flex $(FLEXFlAGS) -opresent.lex.c lexer.l 

present: present.lex.c present.tab.h present.tab.c present.h present.c
	$(CC) present.c present.lex.c present.tab.c $(CFLAGS) ${LCFLAGS} $(LIBS) -o $@ 

clean:
	rm -f present present.lex.c present.tab*
