REL_FLAGS = -O2 -DNDEBUG
DEB_FLAGS = -O0 -DDEBUG -ggdb3

samplepad: samplepad.cpp Config.cpp Config.h JackHelper.cpp JackHelper.h Types.cpp Types.h
	g++ $(DEB_FLAGS) samplepad.cpp Config.cpp JackHelper.cpp Types.cpp -o samplepad -std=c++17 -I./json/include -ljack -lsndfile -lsamplerate -lrubberband

metronome: midimetronome.cpp Metronome.cpp Metronome.h
	g++ midimetronome.cpp Metronome.cpp Metronome.h -o metronome -ljack -lsndfile -lsamplerate

looper: midilooper.cpp Metronome.cpp Metronome.h
	g++ midilooper.cpp Metronome.cpp Metronome.h -o looper -ljack -lsndfile -lsamplerate