PPC speed override shenanigans
----------------------------------

WARNING: I'M NOT RESPONSIBLE IF YOUR CONSOLE MELTS INTO SLAG FROM RUNNING IT AT NON-INTENDED CLOCK SPEEDS. ALSO, NO USER SUPPORT WILL BE PROVIDED.

Have you noticed that the Nintendo switch doesn't allow docked mode performance in portable mode, and that 1.224 Ghz is behind a SDEV wall (but perfectly usable in lakka, because Linux doesn't care?)

I reversed the PPC system module (the one responsible for clock speeds), and figured out that there was a table set up as such:


| profile    | portable | docked |
|:---------- |:-------- |:------ |
| 0x10000    | 1        | 1      |
| 0x10001    | 0        | 1      |
| 0x10002    | 0        | 0      |
| 0x20000    | 0        | 0      |
| 0x20001    | 1        | 1      |
| 0x20002    | 0        | 0      |
| 0x20003    | 1        | 1      |
| 0x20004    | 1        | 1      |
| 0x20005    | 0        | 0      |
| 0x20006    | 0        | 0      |
| 0x92220007 | 1        | 1      |
| 0x92220008 | 1        | 1      |

The second two fields indicate whether the mode is "allowed" in a mode. Changing the second two fields to `1` allows to set the mode whenever (assuming we're privelleged, as with HBL.) As a result, userspace homebrew can now use apmSetPerformanceConfiguration to set the performance profile to any of these no matter the current dock state (including the ones which are both zero, which would normally require SDEV.)

The IPS patch in ppc_patches is intended for 6.0.0 or 6.0.1. I'm not going to backport it, although backporting shouldn't be that difficult assuming the module is set up the same.

The other directory here (apmspeedtest) is a test program to verify that it works properly (e.g. no error codes, proper speed with stress test.)The switch doesn't provide millisecond clock resolution, so we run it for sixty seconds to be accurate. The 1.224 Ghz ones report as higher under *most* circumstances.

Caveats
---------

Unfortunately, there are some.

When the switch is not plugged into a power source (dock or cord), some profiles will become incredibly slow (0.25Ghz???). I'm as of yet unsure if this is a bug or intended behavior, but what's certain is this is not a tested configuration so some issues are unsurprising. Don't set the clock to a profile which ups the CPU speed/GPU speed unless you're plugged into a power source.

This is not particularly useful for overriding game clock speeds, since clock speeds must be set by the application and are dropped as soon as the application is killed. We need a mitm module for intercepting apm.
 
