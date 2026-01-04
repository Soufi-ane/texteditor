Flags = -Iinclude -lraylib -lm -lpthread -ldl -lrt -lGL
Debug = -g -Wall
InstallDir = /usr/local/bin
FontsDir = /usr/local/share/fonts
AssetsDir = ./assets
FontName = JetBrainsMonoNF.ttf

CC = gcc
SRC = src/*.c
OUT = build/note

all: $(OUT)

run : clean $(OUT)
	./$(OUT)

install: $(OUT) fonts 
	mkdir -p $(InstallDir)
	cp $(OUT) $(InstallDir)

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(Flags) 

debug : $(SRC)
	# $(CC) -o $(OUT) $(SRC) $(Flags) $(Debug)
	$(CC) -o $(OUT) $(SRC) $(Flags) -g
		
fonts : 
	mkdir -p $(FontsDir)
	cp $(AssetsDir)/fonts/$(FontName) $(FontsDir)

clean:
	rm -f $(OUT)


