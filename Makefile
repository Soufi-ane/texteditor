Flags = -Iinclude -lraylib -lm -lpthread -ldl -lrt -lGL
Debug = -g -Wall -Werror
InstallDir = /usr/local/bin
FontsDir = /usr/local/share/fonts
AssetsDir = ./assets
FontName = JetBrainsMonoNF.ttf

CC = gcc
SRC = src/*.c
OUT = build/note

all: $(OUT) fonts

run : $(OUT)
	./$(OUT)

install: $(OUT) fonts 
	mkdir -p $(InstallDir)
	cp $(OUT) $(InstallDir)
	fc-cache -fv

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(Flags) 

debug : $(SRC)
	$(CC) -o $(OUT) $(SRC) $(Flags) $(Debug)
		
fonts : 
	mkdir -p $(FontsDir)
	cp $(AssetsDir)/fonts/$(FontName) $(FontsDir)

clean:
	rm -f $(OUT)


