BDIR := build
SRC := main.c
BIN := $(BDIR)/keyz-ro

all: clean build run

$(BIN): $(SRC)
	@mkdir -p $(BDIR)
	clang -O3 -march=native -flto -s $(SRC) -o $(BIN) -lraylib -lcjson -lm

build: $(BIN)

run: $(BIN)
	@$(BIN)

clean:
	@rm -f $(BIN)

