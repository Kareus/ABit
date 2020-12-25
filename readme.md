# ABit

Simple VST Instrument project, for studying some stuffs



### Build
I didn't set VST_ID at ABitcids.h so you may set any unique value to use it.

Build as VST 2.4 if you want to use dll files. You can set the flag 'BUILD_AS_VST2' in version.h (otherwise you will build in VST3)

Both the options would generate .vst3 file.

There would be missing files if you build in VST 2.4, so you should get them from the old SDK.



You can create a project using the project generator in Steinberg VST SDK, and change or add files from this git.

(Sorry for bad build instructions, I messed up with my files a lot so cannot organize things)



Tested on Windows 10, Debug-x64, FL Studio 20



### Note
- GUI developed with VSTGUI. (and I recommend JUCE or something else. hard to find some tips)

- Pulse 25% sounds same as Pulse 75%.

- Some bugs on re-focus (not redrawing or waveform text resets to default value, 'Triangle'. not affecting the real waveform)

- Parameter values - Processor, GUI update - Controller

  (I tried to set parameters in the controller but failed. So... don't be silly like me)

- There are some to-do lists, but I would not work on

  - volume property (attack-decay section)
  - volume compression when multiple notes on
  - some other ADSR things (like, release without sustain)
  - portamento mode (I don't know how to)
  - changing noise pitch in GUI (implemented functions only)



### Reference

based on a lot of things from [VSTSID](https://github.com/igorski/VSTSID), thanks to igorski.