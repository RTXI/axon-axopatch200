###Axon AxoPatch 200 Controller

**Requirements:** None  
**Limitations:** None  

![Module GUI](axon-axopatch200.png)

<!--start-->
Amplifier control module to compensate for scaling properties of the Axon AxoPatch 200 controller. This module essentially acts as an interface that replicated functionality of the control panel, but in a manner specific to the controller's own functionality. 
<!--end-->

####Input Channels
1. input(0) - Mode Telegraph : the telegraph used in Auto mode
2. input(0) - Gain Telegraph : the telegraph used in Auto mode

####Output Channels
None

####Parameters
1. Input Channel - input channel to scale (#)
2. Output Channel - output channel to scale (#)
3. Headstage Gain - gain set by the headstage
4. Command Sensitivity - sensitivity setting from amplifier (mV/V)
5. Output Gain - output gain from amplifier
6. Amplifier Mode - mode setting on the amplifier (vclamp, iclamp, or i=0)

####States
None
