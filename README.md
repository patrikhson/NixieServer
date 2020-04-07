# NixieServer
Simple Raspberry Pi Nixie Clock server, allowing outside scipt driven display

When I purchased my Nixie Clock and connected it to my Raspberry Pi, the firmware
which was suggested by the vendor didn't have much capability and only allowed the
clock to do one function, always initiated by running the driver and only from the
command line.  I wanted more.

I wanted a background server which could take commands from a Linux Pipe, thus
allowing me to have a simple script which could anything I wanted and tell the
display to change at any time, without having it stop and start again.

Since the clock is being driven from a Raspberry Pi running Linux, it already has
NTP, which is synced to atomic clocks, so it doesn't need to rely on any internal
clock chip.  Hardware clock chips are notorious for drifting and using the Linux
system time means the clock automatically adjusts for daylight savings, etc.

I have simplified the original "DisplayNixie.cpp" code, removing parts which are not
needed and including comments to better describe what's happening.  It now reads
commands from stdin (which can be a pipe file) in a simple loop.

You can create a simple Linux pipe using the command:

% mkfifo pipe-name
  
where "pipe-name" is the name of your pipe file.  I used "input".

You can then run the server with the command:

% NixieServer "clock" <input

You can feed the server commands using:

% echo "clock" >input

That's it!  Simple!  I like simple!  Here are the list commands:

* clock - Display HH:MM:SS and keep it updated every second (normal clock display)  
* leds R G B - Set the LEDs under the digits to the RGB values given  
* dots L R - Turn on (1) or off (0) the dots between the digits  
* settime - Set the time in the clock's clock chip (not really needed now)  
* roulette - Run all digits through displays of all values very quickly for a few seconds  
* setsystime - Set the clock's clock chip to agree with Linux system time (not needed)  

Any other command which is not one of the above is expected to be a sequence of
either digits or spaces.  For example, my own script reads the inside and outside
temperatures and displays them every few seconds with inside on the left and outside
on the right, separated by blank digits (spaces).

You can blank the entire display (such as when you are going on vacation and don't need
to place additional wear on the digits) by simply doing:

% echo "      " >input  
% dots 0 0 >input  
% leds 0 0 0 >input  

When you return, simply restart your script to drive the display.

I'm open on implementation of other commands, please send me email!

(The "roulette" function is pretty widely documented on the net as a way to reset
the cathodes inside the tubes and keep them working well.  Otherwise, the digits
will become more and more dim over time.  I run it at midnight!)
