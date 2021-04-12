# MckSampler

## Description

MckSampler is a one shot sample player that allows the user to trigger drum samples in WAV format with MIDI note messages.

At this point MckSampler is just a command line program that creates its configuration in the folder ```$HOME/.mck/sampler/config.json```.

The configuration is created with the command ```./mck-sampler --config```.

The desired WAV files have to go in the folder ```$HOME/.mck/sampler/audio/```.

## Build Dependencies

### Debian / Ubuntu
```
sudo apt install build-essential node npm git
sudo apt install libsamplerate0-dev libsndfile1-dev libjack-jackd2-dev
sudo apt install libgtk-3-dev libwebkit2gtk-4.0-dev
```

### Fedora
```
sudo dnf install make automake libtool gcc-c++ nodejs npm git
sudo dnf install libsamplerate-devel libsndfile-devel jack-audio-connection-kit-devel
sudo dnf install gtk3-devel webkit2gtk3-devel
```

## Getting started

```
make dependencies

sudo make install
```

## Features (including planned stuff)

- [x] JSON config file
- [x] Samplerate conversion
- [x] WAV file import
- [x] Configuration of pads, samples and controls per cli
- [x] Web interface
- [ ] Sample import from any directory
- [ ] Choke groups (stop one sample if another is triggered)
- [ ] N * 16 Step Sequencer
  - [ ] Listen to Jack Transport
  - [ ] Lead Jack Transport
  - [ ] Polyrhythm with variable step length 
- [ ] Modification per pad
  - [ ] Pitch
  - [ ] LowPass Filter
  - [ ] ADSR / Length
