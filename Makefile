REL_FLAGS = -O2 -DNDEBUG -std=c++17
DEB_FLAGS = -O0 -DDEBUG -ggdb3 -std=c++17
INCLUDES = -I./src/gui -I./src/gui/concurrentqueue -I./src/gui/json/include -I./src/helper -I./src/q/q_lib/include -I./src/q/infra/include `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
SOURCES = ./src/main.cpp ./src/Config.cpp ./src/ConfigFile.cpp ./src/gui/GuiWindow.cpp ./src/Processing.cpp ./src/helper/JackHelper.cpp ./src/helper/DspHelper.cpp ./src/helper/Transport.cpp ./src/helper/WaveHelper.cpp ./src/SampleExplorer.cpp ./src/Types.cpp
HEADER = ./src/Config.hpp ./src/ConfigFile.hpp ./src/gui/GuiWindow.hpp ./src/Processing.hpp ./src/helper/JackHelper.hpp ./src/helper/DspHelper.hpp ./src/helper/Transport.hpp ./src/helper/WaveHelper.hpp ./src/SampleExplorer.hpp ./src/Types.hpp
LINKS = `pkg-config --libs gtk+-3.0 webkit2gtk-4.0` -ljack -lsndfile -lsamplerate #-lrubberband

release: ${SOURCES} ${HEADER}
	mkdir -p bin/release
	g++ $(REL_FLAGS) $(INCLUDES) $(SOURCES) -o ./bin/release/mck-sampler $(LINKS)

debug: ${SOURCES} ${HEADER}
	mkdir -p bin/debug
	g++ $(DEB_FLAGS) $(INCLUDES) $(SOURCES) -o ./bin/debug/mck-sampler $(LINKS)

dependencies:
	git submodule update --init --recursive
	cd gui && npm install
	cd gui/mck-sampler-ui && npm install
	cd deps/TypeGenerator && npm install

types:
	node deps/TypeGenerator src/types/Transport.yaml
	mv src/types/ts/* gui/mck-sampler-ui/src/types/

install: gui release
	mkdir -p /usr/share/mck-sampler/gui/
	cp ./bin/release/mck-sampler /usr/bin
	cp -r ./gui/mck-sampler-ui/dist/* /usr/share/mck-sampler/gui/
	cp -r ./ressource/*.desktop /usr/share/applications/

metronome: ./src/midimetronome.cpp ./src/Metronome.cpp ./src/Metronome.hpp
	mkdir -p bin
	g++ ./src/midimetronome.cpp ./src/Metronome.cpp ./src/Metronome.hpp -o ./bin/metronome -ljack -lsndfile -lsamplerate

looper: ./src/midilooper.cpp ./src/Metronome.cpp ./src/Metronome.hpp
	mkdir -p bin
	g++ ./src/midilooper.cpp ./src/Metronome.cpp ./src/Metronome.hpp -o ./bin/looper -ljack -lsndfile -lsamplerate

.PHONY: gui
gui:
	cd gui/mck-sampler-ui && npm run build

guitest: ./src/wvtest.cpp
	mkdir -p bin
	g++ ./src/wvtest.cpp `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` -o ./bin/wvtest

all: release metronome looper

run-debug: debug
	cd gui && npm run dev &
	WEBKIT_INSPECTOR_SERVER=127.0.0.1:1234 ./bin/debug/mck-sampler
	#epiphany inspector://127.0.0.1:1234

run:
	WEBKIT_INSPECTOR_SERVER=127.0.0.1:1234 ./bin/debug/mck-sampler