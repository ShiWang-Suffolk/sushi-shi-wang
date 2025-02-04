CXXFLAGS = -Wall -Wextra -Wpedantic -Wno-unused-function -g -fPIC

OBJS = Main.o Sushi.o sushi_parse.o sushi_yyparser.tab.o lex.yy.o # New objects added

BIN = ./sushi

all: $(BIN)

include DEPEND

$(BIN): $(OBJS) 
	$(CXX) -o $(BIN) $(OBJS) $(LDLAGS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

# New rule: use CXX for C files
.c.o:
	$(CXX) $(CXXFLAGS) -c $<

# Modified
clean:
	rm -f $(BIN) $(OBJS) DEPEND # lex.yy.c sushi_yyparser.tab.c sushi_yyparser.tab.h

test: $(BIN)
	echo "my command" | $(BIN)

DEPEND: 
	$(CXX) -MM *.cc > DEPEND

# New dependencies
sushi_yyparser.tab.c sushi_yyparser.tab.h: sushi_yyparser.y
	bison -d sushi_yyparser.y

sushi_yyparser.tab.o: sushi_yyparser.tab.c Sushi.hh

lex.yy.c: sushi_yylexer.l
	flex -I sushi_yylexer.l 

sushi_parse.o: sushi_parse.cc sushi_yyparser.tab.h Sushi.hh

lex.yy.o: lex.yy.c Sushi.hh sushi_yyparser.tab.h
