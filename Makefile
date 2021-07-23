LCFLAGS=`pkg-config --cflags --libs cairo`
LIBS=-lfl

all: present

parser: parser.y
	bison $(BISONFLAGS) -d -opresent.tab.c parser.y

lexer: lexer.l
	flex $(FLEXFlAGS) -opresent.lex.c lexer.l 

present: parser lexer present.h present.c
	$(CC) present.c present.lex.c present.tab.c $(CFLAGS) ${LCFLAGS} $(LIBS) -o $@ 

clean:
	rm -f present present.lex.c present.tab*
