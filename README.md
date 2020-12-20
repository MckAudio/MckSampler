# MckSampler

## Description

MckSampler is a one shot sample player that allows the user to trigger drum samples in WAV format with MIDI note messages.

At this point MckSampler is just a command line program that creates its configuration in the folder ```$HOME/.mcksampler/config.json```.

The configuration is created with the command ```./mcksampler --config```.

The desired WAV files have to go in the folder ```$HOME/.mcksampler/audio/```.

## Getting started

```
git submodule update --init
sudo apt install build-essential libsamplerate0-dev libsndfile1-dev libjack-jackd2-dev

make release
```

## Features (including planned stuff)

- [x] JSON config file
- [x] Samplerate conversion
- [x] WAV file import
- [x] Configuration of pads, samples and controls per cli
- [ ] Web interface
- [ ] Sample import from any directory
- [ ] Modification per pad
  - [ ] Pitch
  - [ ] LowPass Filter
  - [ ] ADSR / Length
