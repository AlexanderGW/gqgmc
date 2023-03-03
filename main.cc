// **************************************************************************
// File: main.cc
//
// Author:    Phil Gillaspy
//
// Description: Demonstration program for GQ GMC (geiger-muller counter).
//
//    Usage: gqgmc <usb-port-device-name>
//    Example: gqgmc /dev/ttyUSB0
//
// **************************************************************************
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <unistd.h>

#include "gqgmc.hh"
using namespace GQLLC;

// Utility to show message to user. To be adapted to a pop-up window
// when code developed for GUI.
void Display_message(string msg)
{
  cout << msg << endl;
}

// Utility to encapsulate the code to display an error message. This is
// separated from Display_message() because this is specialized to
// accessing and formulating the GQGMC error status, but not displaying.
void Display_error(const GQGMC & gmc)
{
  gmc_error_t  err;
  err = const_cast<GQGMC &>(gmc).getErrorCode();
  stringstream msg;
  msg << const_cast<GQGMC &>(gmc).getErrorText(err);
  Display_message(msg.str());
  return;
}

int
main(int argc, char **argv)
{
  // Open the USB port using a USB to serial converter device driver.
    // Using UDEV rule file 51-gqgmc.rules to create symlink to /dev/gqgmc.
  string  usb_device = "/dev/gqgmc";

  if (argc > 1)
    usb_device = argv[1];
  else
  {
    cout << "Usage: gqgmc <usb-port-device-name>" << endl;
    cout << "Example: gqgmc /dev/ttyUSB0" << endl;
    return 0;
  }

//  cout << usb_device << endl; // debug

  // Instantiate the GQGMC object on the heap
  GQGMC * gqgmc = new GQGMC;

  // Open USB port
  gqgmc->openUSB(usb_device);

  // Check success of opening USB port
  if (gqgmc->getErrorCode() == eNoProblem)
  {
    stringstream msg;
    msg << "USB is opened" << endl;
    Display_message(msg.str());
  }
  else
  {
    Display_error(*gqgmc); // dereference to pass by reference
    gqgmc->closeUSB();
    return 0;
  }

  // cout << "get CPM" << endl;
  // Get CPM
  uint16_t cpm;

  for(uint32_t i=0; i<10; i++)
  {
    cpm = gqgmc->getCPM();
    if (gqgmc->getErrorCode() == eNoProblem)
    {
      stringstream msg;
      msg << dec << cpm << ":CPM" << endl;
      Display_message(msg.str());
    }
    else
      Display_error(*gqgmc);

    sleep(1);
  } // end for loop


  // cout << "Turning on auto-CPS data" << endl;
  // // Turn on CPS
  // gqgmc->turnOnCPS();
  // sleep(1);
  // uint16_t cps_int   = 0;
  // for(uint32_t i=0; i<10; i++)
  // {
  //   cps_int = gqgmc->getAutoCPS();
  //   if (gqgmc->getErrorCode() == eNoProblem)
  //   {
  //     cout << dec << "s=" << i << " " << cps_int << endl;  // debug
  //   }
  //   else
  //     Display_error(*gqgmc);

  //   sleep(1);
  // } // end for loop

  // // Turn off CPS reporting
  // cout << "turning off CPS" << endl;
  // gqgmc->turnOffCPS();
  // if (gqgmc->getErrorCode() != eNoProblem)
  //   Display_error(*gqgmc);


  // Close USB port
  gqgmc->closeUSB();

  delete gqgmc;

  return 0;
} // end main()
