Logomatic Firmware with SDHC and FAT32
Chris Jeppesen
http://stkwans.blogspot.com/
19 May 2012

OK, this empty file came with the package, so I am going to use it. This is the
stock Logomatic firmware with the bare minimum adjustments needed to use the
latest sd_raw and fat libraries which have SDHC and fat32 activated, 
respectively. To use, copy FW.SFE to the root directory of the Logomatic card,
then power up the card with the USB disconnected. The USB bootloader will
copy the firmware to the controller's flash memory then delete FW.SFE off of 
the card. Once done, the logger program should work exactly as before, just 
with the ability to work with SDHC cards and FAT32 file systems. The program 
still works with SD normal capacity and FAT16 file systems as well.

While this program recognizes SDHC and FAT32, by itself it does not make the 
bootloader do also. I have a USB bootloader modified similarly, but if you 
don't use that bootloader, you will need to write FW.SFE to ad SD normal 
capacity (<=2GB, not marked SDHC) card formatted as FAT16. It still might not 
work, since the bootloader tries to read the card each time it starts up to see 
if it needs to install FW.SFE, and since it can't read an SDHC card, it may 
never boot the main firmware. Best to get my USB bootloader also, just in case. 
Find it on my blog in the same place you found this code.

Sparkfun published the code with no explicit license, but since so much of it 
is GPL and LGPL, I can presume that all of it is, so my changes are GPL'd also. 
I really didn't do that much. I stand on the shoulders of giants. Anyone is 
allowed to do whatever they want with it within the limits of the GPL (3.0 or 
later). Specifically, Sparkfun is allowed to use my changes and do whatever 
they need to do to put this program onto production Logomatics, just as they 
have been doing, whether this is technically allowed by the GPL or not.

The code works for me, but there are almost certainly bugs in it. It is hoped 
that this code is useful, but there is NO WARRANTY, as specified by the GPL. If 
this code causes your house to burn down, it's your fault. If you have any bug 
fixes, you are welcome to send them to me, and if the changes work for me, I 
will include them in the hypothetical next revision.

