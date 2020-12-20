REL_FLAGS = -O2 -DNDEBUG
DEB_FLAGS = -O0 -DDEBUG -ggdb3

release: ./src/samplepad.cpp ./src/Config.cpp ./src/Config.hpp ./src/JackHelper.cpp ./src/JackHelper.hpp ./src/Types.cpp ./src/Types.hpp
	mkdir -p bin/release
	g++ $(REL_FLAGS) ./src/samplepad.cpp ./src/Config.cpp ./src/JackHelper.cpp ./src/Types.cpp -o ./bin/debug/samplepad -std=c++17 -I./json/include -ljack -lsndfile -lsamplerate -lrubberband

debug: ./src/samplepad.cpp ./src/Config.cpp ./src/Config.hpp ./src/JackHelper.cpp ./src/JackHelper.hpp ./src/Types.cpp ./src/Types.hpp
	mkdir -p bin/debug
	g++ $(DEB_FLAGS) ./src/samplepad.cpp ./src/Config.cpp ./src/JackHelper.cpp ./src/Types.cpp -o ./bin/release/samplepad -std=c++17 -I./json/include -ljack -lsndfile -lsamplerate -lrubberband

metronome: ./src/midimetronome.cpp ./src/Metronome.cpp ./src/Metronome.hpp
	mkdir -p bin
	g++ ./src/midimetronome.cpp ./src/Metronome.cpp ./src/Metronome.hpp -o ./bin/metronome -ljack -lsndfile -lsamplerate

looper: ./src/midilooper.cpp ./src/Metronome.cpp ./src/Metronome.hpp
	mkdir -p bin
	g++ ./src/midilooper.cpp ./src/Metronome.cpp ./src/Metronome.hpp -o ./bin/looper -ljack -lsndfile -lsamplerate

all: release metronome looper