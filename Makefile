MYLFLAGS=`pkg-config --libs cairo`
MYCFLAGS=`pkg-config --cflags cairo`
LIBS=-lfl -lm
DEPS = present.tab.h present.h object.h
OBJ = present.lex.o present.tab.o present.o object.o

all: parser lexer present

parser: parser.y
	bison $(BISONFLAGS) -d -opresent.tab.c parser.y

lexer: lexer.l present.tab.c present.tab.h
	flex $(FLEXFlAGS) -opresent.lex.c lexer.l 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(MYCFLAGS);

present: $(OBJ)
	$(CC) $^ $(CFLAGS) ${MYCFLAGS} ${MYLFLAGS} $(LIBS) -o $@ 

clean:
	rm -f present present.lex.c present.tab* *.o *.mp4
