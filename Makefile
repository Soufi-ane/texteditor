FLAGS = -Iinclude -lraylib -lm -lpthread -ldl -lrt -lGL
DEBUG_FLAGS  = -Wpedantic -g 
PROD_FLAGS   = -DPROD -O3
INSTALL_DIR  = /usr/local/bin
FONTS_DIR    = /usr/local/share/fonts
DATA_DIR     = /usr/local/share/texteditor
HOME_DIR     = $(shell getent passwd $(SUDO_USER) | cut -d: -f6)
CONFIG_DIR   = $(HOME_DIR)/.config/texteditor
ASSETS_DIR   = ./assets
FONT_NAME    = JetBrainsMonoNF.ttf

CC = gcc
SRC = src/*.c
OUT = build/texteditor_dev
OUT_PROD = build/texteditor

all: $(OUT)

run : clean $(OUT)
	./$(OUT)

install: $(OUT_PROD) fonts 
	mkdir -p $(INSTALL_DIR) $(DATA_DIR) $(CONFIG_DIR)
	cp $(OUT_PROD) $(INSTALL_DIR)
	cp $(ASSETS_DIR)/help.txt $(DATA_DIR)
	cp $(ASSETS_DIR)/texteditor.conf $(CONFIG_DIR)
	chown -R $(SUDO_USER):$(SUDO_USER) $(CONFIG_DIR)

$(OUT_PROD) : $(SRC)
	$(CC) -o $(OUT_PROD) $(SRC) $(FLAGS) $(PROD_FLAGS)

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(FLAGS) 

debug : $(SRC)
	$(CC) -o $(OUT) $(SRC) $(FLAGS) $(DEBUG_FLAGS)
		
fonts : 
	mkdir -p $(FONTS_DIR)
	cp $(ASSETS_DIR)/fonts/$(FONT_NAME) $(FONTS_DIR)

clean:
	rm -f $(OUT) $(OUT_PROD)


