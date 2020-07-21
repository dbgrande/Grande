Grande Modules for VCV Rack 1.0
===============================

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



Quant31
-------

31-TET Quantizer with Equi-likely mode.

- Same features as Quant, except no scale input, and 31 buttons.



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

To use this module as a tuner, combine it with a voltmeter that displays voltages as notes—preferably one that goes from –50¢ to +50¢, so the display doesn't bounce around too wildly.



License
-------

GPL-3.0+