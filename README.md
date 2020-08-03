Grande Modules for VCV Rack 1.0
===============================
%%%%%%%%%%%%%%%%%%%%%%%%%%%

***See below for new Microtonal Collection!***

%%%%%%%%%%%%%%%%%%%%%%%%%%%

By David Grande

![Grande Plugins](images/grande_plugins.png "Available Modules")


Blank1HP
--------

Blank panel.



Scale
-----

Provides extra scales for Quant quantizer.

- Use a polyphonic switch and plug into Scale input of Quant.
- Sends 12 control signals using polyphonic cable (0V or 10V).

![Additional Scales](images/additional_scales.png "Additional Scales")



Quant
-----

12-TET Quantizer with Equi-likely mode.

- Scale (input): Use Scale modules and polyphonic switch to allow selecting different scales.
- Root (input): Defines root note of scale (1V/Oct, polyphonic) **Now quantized.**
- Rounding mode (switch): –1 (down) = round down, 0 (center) = round nearest, 1 (up) = round up
- Equi-likely mode (switch): 0 (down) = off, 1 (up) = on
- In (input): CV input (1V/Oct, polyphonic)
- Out (output): Quantized CV output (1V/Oct, polyphonic)
- Trigger (output): Trigger whenever note changes (polyphonic)
- Twelve buttons: Set current scale, or display externally defined scale. Root on bottom. Defaults to major scale. If no notes are selected, defaults to just the root note.


**Blues Scale using Normal quantization and rounding down:**

![Blues Normal](images/blues_normal.png "Normal")

The probability of randomly getting each note (x-axis) depends on the interval between notes. Closely spaced notes are much less likely than farther spaced notes.

The Blues Scale is particularly obvious with its intervals of 321132, where the probability of picking individual notes varies from 8% to 25%.


**Blues Scale using Equi-likely quantization and rounding down:**

![Blues Equi-likely](images/blues_equilikely.png "Equi-likely")

For comparison, here is the Blues Scale using Equi-likely mode.

**Note:** This equi-likely mode is best for random input frequencies. It may cause unexpected note shifting for inputs that are already more-or-less quantized.



SampleDelays (SD)
-----------------

Provides three sample-delay buffer chains giving one or two sample delays each.

- Internally chained together to give up to six sample delays.



CVInts (Ints)
-----------

Provides 3 pairs of integer CVs from 0V to 10V (both + and –).



CVBigInts (BI)
-------------

Provides 3 pairs of integer CVs from 0V to 32V (both + and –).

***Exceeds normal voltage standards, so use with caution. ;-)***



Frequency (Freq)
---------------

Converts a frequency to V/Oct.

- Thresh sets input threshold voltage (–10V to 10V).

- Smooth sets number of measurements to average together (from 1 to 16).

To use this module as a tuner, combine it with a voltmeter that displays voltages as notes—preferably one that goes from –50 to +50 cents, so the display doesn't bounce around too wildly.



*New:* Microtonal Collection
============================

In addition to the previously existing Quant31, I have now added four other microtonal modules.

![Grande Microtonal Modules](images/microtonal_collection.png "Microtonal Collection")



Quant31
-------

A 31-TET microtonal quantizer with Equi-likely mode.

- Same features as Quant (above), except no external scale input, and 31 buttons.



QuantMT
-------

An equal temperament microtonal quantizer where the valid notes are defined directly.

- Notes/Oct knob defines temperament, from 1-TET through 31-TET (default 12-TET).

- The note buttons between the two blue lights select valid notes. Notes outside this range are ignored.

- The two small unmarked buttons at the top of the note selection column are "Set All" and "Clear All".

- Quantizer features are the same as Quant (above), minus the external scale input. 



QuantMTIntervals
----------------

An equal temperament microtonal quantizer where the valid notes are defined indirectly by pitch intervals.

- Notes/Oct knob defines temperament, from 1-TET through 31-TET (default 12-TET).

- Tolerance knob defines required accuracy for matching, from 0 to 50 cents (default 20 cents).

- The interval buttons select desired pitch intervals, but they need to be mapped to notes in the selected temperament. Valid notes are those that match selected intervals within the specified tolerance. These are indicated by a blue light, with the brightness indicating how close the match is.

- Set All - enables all interval buttons.

- Clear All - disables all interval buttons, except 1/1 unison.

- Show Valid - Turns on blue lights of intervals nearest to valid matching notes (based on tolerance), including disabled buttons.

- Clear Invalid - Deselects all intervals that don't have a light showing.

- Quantizer features are the same as Quant (above), minus the external scale input. 

*Note that there is currently room for adding three more interval buttons—submit your favorite interval.*



QuantMTInt11 (QuantMTIntervals 11)
----------------------------------

A variation of QuantMTIntervals (above), except that it's pitch intervals are based on the 11-Limit Tonality Diamond. However, I included three additional intervals: 16:15 (minor 2nd), 15:8 (major 7th), and 27:14 (Septimal major 7th).

**New:** Show Notes button has been added to show the actual notes being used. Useful for understanding what this quantizer is doing, and allows converting its results to another quantizer.

*This feature will be added to QuantMTIntervals once the number of intervals in that module have been increased to 31.*

[Link to Wikipedia article on Tonality Diamonds](https://en.wikipedia.org/wiki/Tonality_diamond)



NoteMT
------

An equal temperament microtonal note generator and display preprocessor.

- Notes/Oct knob defines temperament, from 1-TET through 31-TET (default 12-TET).

- Octave and Note knobs set voltage of V/Oct output based current temperament, with Note knob clamped to Notes/Oct – 1.

- Octave and Note outputs convert V/Oct-In voltage (or knobs if input unconnected) into a pair of integer voltages. These can be displayed on a voltmeter, like the one from ML Modules.

- A blue light at the top turns on when the Note output values are integers, indicating that the Notes/Oct setting is likely matching the V/Oct-In source. However, this is not guaranteed.



License
-------

GPL-3.0+