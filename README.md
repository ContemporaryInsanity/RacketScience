# Racket Science
Modules for the VCV Rack virtual modular synthesizer.

![](./img/boogie.gif)

## Boogie Bay

You've seen flying faders, with Boogie Bay you have sliding sockets for all of your wire wobbling needs.  Also doubles up as a voltage indicator, right click for range menu, though BB H8 is currently full scale -10 to +10 volts only.

## Vector Victor

Shortly after I first got into VCV around May 2019 I found myself looking for a real time CV loop recorder yet couldn't find one, once the VCV Prototype module appeared I created a simple precursor of Vector Victor, also inpsired by the ZZC phase based way of timing.

Vector Victor is a phase driven CV recorder, if you're familiar with programming think of it as a thousand element array indexed by the phase input as that's exactly what it is.  You get two for the price of one!

Typically you would feed the phase input with a slow rising sawtooth from a LFO, or ZZC clock & divider modules to keep everything in sync.  A simple use case is shown below where Vector Victor loops input from a MIDI keyboard.  It can also be used to record knob movements or whatever really but is not designed with audio rates in mind.

![](./img/RSVectorVictor.png)

As a typical use case for recording knob movements requires a MIDI mapped knobs module and buttons module feeding VV, the next version of VV (VV WKO) will have knobs & buttons which can be directly MIDI mapped so other modules aren't required.

Note that VV saves state, so if you record a loop with it and then duplicate or save the preset the state will persist.



## Notices

**Racket Science** is copyright © 2019 Ewen Bates, specifically:

All **source code** is copyright © 2019 Ewen Bates and is licensed under the [GNU General Public License v3.0](gpl-3.0.txt).

All **graphics** in the `res` directory are copyright © 2019 Ewen Bates and licensed under [CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/).

Many thanks to the VCV developer community, particularly Andrew Belt of VCV and Adam Verspaget of Count Modular whose modules I've studied and learned from.
