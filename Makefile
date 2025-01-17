CXXFLAGS = -Wall -Wextra -Wpedantic -Wno-unused-function -g -fPIC

OBJS = Main.o Sushi.o 

BIN = ./sushi

all: $(BIN)

include DEPEND

$(BIN): $(OBJS) 
	$(CXX) -o $(BIN) $(OBJS) $(LDLAGS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(BIN) $(OBJS) DEPEND

test: $(BIN)
	$(BIN)

DEPEND: 
	$(CXX) -MM *.cc > DEPEND
