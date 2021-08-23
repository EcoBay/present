IDIR=include
ODIR=obj
SDIR=src
BDIR=bin

MYLFLAGS=`pkg-config --libs librsvg-2.0 cairo`
MYCFLAGS=`pkg-config --cflags librsvg-2.0 cairo` -I$(IDIR)
LIBS=-lfl -lm

_DEPS = present.h object.h draw.h tex.h symtable.h ast.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
_OBJ = present.lex.o present.tab.o present.o object.o draw.o tex.o symtable.o ast.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: parser lexer present

parser: $(SDIR)/parser.y
	bison $(BISONFLAGS) -d --output=$(SDIR)/present.tab.c $(SDIR)/parser.y

lexer: $(SDIR)/lexer.l $(SDIR)/present.tab.c $(SDIR)/present.tab.h
	flex $(FLEXFLAGS) --outfile=$(SDIR)/present.lex.c $(SDIR)/lexer.l 

$(OBJ): $(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) ${MYCFLAGS}

present: $(OBJ)
	$(CC) $^ $(CFLAGS) ${MYCFLAGS} ${MYLFLAGS} $(LIBS) -o $(BDIR)/$@ 

clean:
	rm -f $(BDIR)/present $(SDIR)/present.lex.c $(SDIR)/present.tab* $(ODIR)/*.o *.mp4
