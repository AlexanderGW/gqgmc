// @file main.cc
// @author Phil Gillaspy (original author) & Alexander Gailey-White
// @date 2023-03-04

// Demonstration program for GQ GMC (geiger-muller counter).

// Usage: gqgmc <usb-port-device-name> <command>
// Example: gqgmc /dev/ttyUSB0 cpm

#include <chrono>
#include <csignal>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <unistd.h>

#include "gqgmc.hh"
using namespace GQLLC;

static volatile sig_atomic_t sigExit = 0;

// Basic signal handler to break out of main loop, and cleanup
void signalHandler(int signum) {
  sigExit = 1;
}

// Utility to show message to user. To be adapted to a pop-up window
// when code developed for GUI.
void outMessage(string msg) {
  std::time_t t
    = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::cout << std::put_time( std::localtime( &t ), "%FT%T%z" ) << "; " << msg << endl;
}

// Utility to encapsulate the code to display an error message. This is
// separated from outMessage() because this is specialized to
// accessing and formulating the GQGMC error status, but not displaying.
void outError(const GQGMC & gmc) {
  gmc_error_t  err;
  err = const_cast<GQGMC &>(gmc).getErrorCode();
  stringstream msg;
  msg << const_cast<GQGMC &>(gmc).getErrorText(err);
  outMessage(msg.str());
  return;
}

int
main(int argc, char **argv) {
  // register signal SIGABRT and signal handler
  signal(SIGINT, signalHandler);

  // Open the USB port using a USB to serial converter device driver.
  // Using UDEV rule file 51-gqgmc.rules to create symlink to /dev/gqgmc.
  string usb_device = "/dev/gqgmc";

  // Default to CPM output
  string gqgmc_command = "cpm";

  if (argc == 2)
    usb_device = argv[1];
  else if (argc == 3) {
    usb_device = argv[1];
    gqgmc_command = argv[2];
  } else {
    cout << "Usage: gqgmc <usb-port-device-name> <command>" << endl;
    cout << "Example: gqgmc /dev/ttyUSB0 cpm" << endl;
    return 0;
  }

  // Instantiate the GQGMC object on the heap
  GQGMC * gqgmc = new GQGMC;

  // Open USB port
  gqgmc->openUSB(usb_device);

  // Check success of opening USB port
  if (gqgmc->getErrorCode() == eNoProblem) {
    // cout << "GQGMC; USB open: " << usb_device << "; Command: " << gqgmc_command << endl;
    cout << "GQ GMC data feed" << endl;
  } else {
    outError(*gqgmc); // dereference to pass by reference
    gqgmc->closeUSB();
    return 0;
  }

  // Output CPM every second
  if (gqgmc_command == "cpm") {
    uint16_t cpm;

    while(1) {
      if (sigExit)
        break;

      cpm = gqgmc->getCPM();
      if (gqgmc->getErrorCode() == eNoProblem) {
        stringstream msg;
        msg << "CPM:" << cpm;
        outMessage(msg.str());
      } else
        outError(*gqgmc);

      sleep(1);
    } // end for loop
  }
  
  // Output CPS
  // else if (gqgmc_command == "cps") {
  //   uint16_t cps = 0;

  //   gqgmc->turnOnCPS();

  //   while(1) {
  //     if (sigExit)
  //       break;
      
  //     cps = gqgmc->getAutoCPS();
  //     if (gqgmc->getErrorCode() == eNoProblem)
  //     {
  //       cout << dec << "s=" << i << " " << cps << endl;  // debug
  //     }
  //     else
  //       outError(*gqgmc);

  //     sleep(1);
  //   } // end for loop

  //   // Turn off CPS reporting
  //   cout << "turning off CPS" << endl;
  //   gqgmc->turnOffCPS();
  //   if (gqgmc->getErrorCode() != eNoProblem)
  //     outError(*gqgmc);
  // }
  
  // Unknown command
  else {
    std::cout << "Unknown command" << endl;
  }
  
  std::cout << "Exiting..." << endl;

  // Close USB port
  gqgmc->closeUSB();

  delete gqgmc;

  return 0;
} // end main()
