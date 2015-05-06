

 INSTALLATION INSTRUCTIONS
 
 1. Download the code from sourceforge into a directory of your choice.
    This will be referred to as your root directory.
 
 2. The following list of files need to be downloaded. Obtain them from
    the gqgmc folder.
    Makefile
    Defines.mk
    Patterns.mk
    Targets.mk
    gqgmc.cc
    gqgmc.hh
    main.cc
    main_gui.cc
    plotter.hh
    plotter.cc
    
 3. Create the following subdirectories of your root directory:
    bin
    libs
    obj
    
    For example, "mkdir bin", "mkdir libs", "mkdir obj".
    
 4. Edit Defines.mk and change the definition of BASE to the
    path of your root directory. If you will be compiling for 32-bit
    Linux, change the definition of CPUSIZE from "-m64" to "-m32"
    in Defines.mk. 
    
 5. From the command line, you should be able to build the 
    'gqgmc_gui' and 'gqgmc' executables which if the build is successful
    will be put into bin subdirectory. Try "make all" as the build
    command. 
    
 6. With the GQ geiger counter connected to your PC via USB and turned
    on, start either bin/gqgmc or bin/gqgmc_gui from a console command
    line. However, you must supply the device name of the USB port on
    the command line, for example on Ubuntu 11.10 try 
    "bin/gqgmc /dev/usb/ttyUSB0". On Ubuntu 12.04 try
    "bin/gqgmc /dev/ttyUSB0".
    
    You can try to identify USB port using the following scheme.
    (a) Before plugging in the GMC-300, run the command "dir /dev/ttyUSB*".
        Take note of what, if any, devices are listed. After plugging 
        in the GMC-300 and turning it on, wait one minute,
        then run the command line command "lsusb".  You should see a line
        with "Prolific Technology Inc." at the end of the line. This 
        indicates that Linux has successfully recognized and setup a port
        for the GMC-300. Now try the command line command "dir /dev/ttyUSB*".
        The device name for the GMC-300 should be the newest port added
        to the previous listing of the same command. (On Ubuntu 11.10,
        try changing /dev/ttyUSB* to /dev/usb/ttyUSB*)
    
 7. The following is a list of prerequisites for successful compilation:
 
    a. Ubuntu Linux 12.04 or 11.10
    b. GNU g++ 4.6.x compiler installed
    c. Qt4 Software Development Kit (SDK) installed
       or Qt Creator installed (required only for GUI)
