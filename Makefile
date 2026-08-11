FLAGS = -Iinclude -lraylib -lm -lpthread -ldl -lrt -lGL
DEBUG_FLAGS  = -Wpedantic -g 
PROD_FLAGS   = -DPROD -O3
INSTALL_DIR  = /usr/local/bin
FONTS_DIR    = /usr/local/share/fonts
DATA_DIR     = /usr/local/share/texteditor
APP_DIR      = /usr/local/share/applications
HOME_DIR     = $(shell getent passwd $(SUDO_USER) | cut -d: -f6)
CONFIG_DIR   = $(HOME_DIR)/.config/texteditor
ASSETS_DIR   = ./assets

CC = gcc
SRC = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)
OUT = build/texteditor_dev
OUT_PROD = build/texteditor

all: $(OUT)

run : $(OUT)
	./$(OUT)

install: $(OUT_PROD)
	mkdir -p $(INSTALL_DIR) $(DATA_DIR) $(CONFIG_DIR) $(FONTS_DIR)
	cp $(ASSETS_DIR)/fonts/JetBrainsMono-Regular.ttf $(FONTS_DIR)
	cp $(OUT_PROD) $(INSTALL_DIR)
	cp $(ASSETS_DIR)/help.txt $(ASSETS_DIR)/messages.log $(ASSETS_DIR)/texteditor.png $(DATA_DIR)
	cp $(ASSETS_DIR)/texteditor.conf $(CONFIG_DIR)
	cp $(ASSETS_DIR)/texteditor.desktop $(APP_DIR)
	update-desktop-database -q
	chown -R $(SUDO_USER):$(SUDO_USER) $(CONFIG_DIR) $(DATA_DIR)

$(OUT_PROD) : $(SRC) $(HEADERS)
	$(CC) -o $(OUT_PROD) $(SRC) $(FLAGS) $(PROD_FLAGS)

$(OUT): $(SRC) $(HEADERS)
	$(CC) -o $(OUT) $(SRC) $(FLAGS) 

debug : $(SRC)
	$(CC) -o $(OUT) $(SRC) $(FLAGS) $(DEBUG_FLAGS)
		
clean:
	rm -f $(OUT) $(OUT_PROD)


