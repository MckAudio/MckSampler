REL_FLAGS = -O2 -DNDEBUG -std=c++17
DEB_FLAGS = -O0 -DDEBUG -ggdb3 -std=c++17
INCLUDES = -I./src/json/include `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
SOURCES = ./src/main.cpp ./src/Config.cpp ./src/helper/JackHelper.cpp ./src/Types.cpp
LINKS = `pkg-config --libs gtk+-3.0 webkit2gtk-4.0` -ljack -lsndfile -lsamplerate #-lrubberband

release: ./src/main.cpp ./src/Config.cpp ./src/Config.hpp ./src/helper/JackHelper.cpp ./src/helper/JackHelper.hpp ./src/Types.cpp ./src/Types.hpp
	mkdir -p bin/release
	g++ $(REL_FLAGS) $(INCLUDES) $(SOURCES) -o ./bin/release/mck-sampler $(LINKS)

debug: ./src/main.cpp ./src/Config.cpp ./src/Config.hpp ./src/JackHelper.cpp ./src/JackHelper.hpp ./src/Types.cpp ./src/Types.hpp
	mkdir -p bin/debug
	g++ $(DEB_FLAGS) $(INCLUDES) $(SOURCES) -o ./bin/debug/mck-sampler $(LINKS)

install: gui release
	cp ./bin/release/mck-sampler /usr/bin

metronome: ./src/midimetronome.cpp ./src/Metronome.cpp ./src/Metronome.hpp
	mkdir -p bin
	g++ ./src/midimetronome.cpp ./src/Metronome.cpp ./src/Metronome.hpp -o ./bin/metronome -ljack -lsndfile -lsamplerate

looper: ./src/midilooper.cpp ./src/Metronome.cpp ./src/Metronome.hpp
	mkdir -p bin
	g++ ./src/midilooper.cpp ./src/Metronome.cpp ./src/Metronome.hpp -o ./bin/looper -ljack -lsndfile -lsamplerate

.PHONY: gui
gui:
	cd gui && npm run build

guitest: ./src/wvtest.cpp
	mkdir -p bin
	g++ ./src/wvtest.cpp `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -o ./bin/wvtest

all: release metronome looper