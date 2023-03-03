# GQGMC driver (Raspberry Pi)
Forked from https://sourceforge.net/p/gqgmc/code/ci/master/tree/

Updated `make` config to build `gqgmc` driver only, compatible with Raspberry Pi (`CPUSIZE` to `mbe32`).

With `main` test printing CPM every second, in a short loop

# Author
Phil Gillaspy

# Original README

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

    51-gqgmc.rules
    
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

 6a. Copy the 51-gqgmc.rules file to the /etc/udev/rules.d directory and
    force the reload of udev rules:
    
    sudo cp ./51-gqgmc.rules /etc/udev/rules.d/51-gqgmc.rules
    sudo udevadm control --reload-rules

    Disconnect the GQ GMC-300 from the computer and then reconnect.
    Verify that there exists a /dev/gqgmc in the /dev directory with
    read/write permission for all users.

    ls -la /dev/gqgmc
 
    
 6b. With the GQ geiger counter connected to your PC via USB and turned
    on, start either bin/gqgmc or bin/gqgmc_gui from a console command
    line. The USB device name must be supplied from the command line, e.g.,

    bin/gqgmc /dev/gqgmc
    bin/gqgmc_gui /dev/gqgmc
    
    The code expects the GQ GMC USB device name to be /dev/gqgmc. This
    device name was created at step 6a.
    
 7. The following is a list of prerequisites for successful compilation:
 
    a. Ubuntu Linux 11.10, 12.04 or 14.04
    b. GNU g++ 4.6.x or more recent compiler installed
    c. Qt4 Software Development Kit (SDK) installed
       or Qt Creator installed (required only for GUI)

