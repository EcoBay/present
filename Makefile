IDIR=include
ODIR=obj

MYLFLAGS=`pkg-config --libs cairo`
MYCFLAGS=`pkg-config --cflags cairo` -I$(IDIR)
LIBS=-lfl -lm

_DEPS = present.h object.h draw.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
_OBJ = present.lex.o present.tab.o present.o object.o draw.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: parser lexer present

parser: parser.y
	bison $(BISONFLAGS) -d -opresent.tab.c parser.y

lexer: lexer.l present.tab.c present.tab.h
	flex $(FLEXFlAGS) -opresent.lex.c lexer.l 

$(OBJ): $(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) ${MYCFLAGS}

present: $(OBJ)
	$(CC) $^ $(CFLAGS) ${MYCFLAGS} ${MYLFLAGS} $(LIBS) -o $@ 
clean:
	rm -f present present.lex.c present.tab* $(ODIR)/*.o *.mp4
