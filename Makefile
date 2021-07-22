LCFLAGS=`pkg-config --cflags --libs cairo`
LIBS=-lfl

all: present

parser: parser.y
	bison -d -opresent.tab.c parser.y

lexxer: lexxer.l
	flex -opresent.lex.c lexxer.l 

present: parser lexxer present.h present.c
	$(CC) present.c present.lex.c present.tab.c ${LCFLAGS} $(LIBS) -o $@ 

clean:
	rm -f present present.lex.c present.tab*
