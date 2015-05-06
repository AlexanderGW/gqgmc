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


void mypause()
{
  cout << "Paused: to continue, press ENTER.. " << flush;
  cin.ignore(1, '\n');
  return;
}

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

  // Test string for date and time
  string date = "030513";
  string time = "171730";

  // Set date
  cout << "date: " << date << endl;
  gqgmc->setDate(date);

  // Set time of day
  cout << "time: " << time << endl;
  gqgmc->setTime(time);


  // Get version
  cout << "get version" << endl;
  string version;
  version = gqgmc->getVersion();
  if (gqgmc->getErrorCode() == eNoProblem)
  {
    stringstream msg;
    msg << version << endl;
    Display_message(msg.str());
  }
  else
    Display_error(*gqgmc);

  cout << "get serial number" << endl;
  mypause();
  // Get serial number. The serial number is returned as 7 binary bytes.
  // So it is necessary to convert the numeric byte value to an ASCII
  // digit equivalent.
  string serial_number;
  serial_number = gqgmc->getSerialNumber();
  if (gqgmc->getErrorCode() == eNoProblem)
  {
    stringstream msg;
    msg << hex << serial_number << endl;
    Display_message(msg.str());
  }
  else
    Display_error(*gqgmc);

  cout << "get CPM" << endl;
  mypause();
  // Get CPM
  uint16_t cpm;
  cpm = gqgmc->getCPM();
  if (gqgmc->getErrorCode() == eNoProblem)
  {
    stringstream msg;
    msg << dec << cpm << endl;
    Display_message(msg.str());
  }
  else
    Display_error(*gqgmc);

  cout << "get 5 samples of CPS" << endl;
  mypause();
  // Get CPS
  for(int i=0;i<5;i++)
  {
    uint16_t cps;
    cps = gqgmc->getCPS();
    if (gqgmc->getErrorCode() == eNoProblem)
    {
      stringstream msg;
      msg << dec << cps;
      Display_message(msg.str());
    }
    else
      Display_error(*gqgmc);

    sleep(1);
  }

  cout << "get battery voltage" << endl;
  mypause();
  // Get battery voltage
  float  voltage;
  voltage = gqgmc->getBatteryVoltage();
  if (gqgmc->getErrorCode() == eNoProblem)
  {
    stringstream msg;
    msg.precision(1);
    msg << fixed << voltage << "V" << endl;
    Display_message(msg.str());
  }
  else
    Display_error(*gqgmc);

  cout << "Turning on auto-CPS data" << endl;
  mypause();
  // Turn on CPS
  gqgmc->turnOnCPS();
  sleep(1);
  uint16_t cps_int   = 0;
  for(uint32_t i=0; i<10; i++)
  {
    cps_int = gqgmc->getAutoCPS();
    if (gqgmc->getErrorCode() == eNoProblem)
    {
      cout << dec << "s=" << i << " " << cps_int << endl;  // debug
    }
    else
      Display_error(*gqgmc);

    sleep(1);
  } // end for loop

  // Turn off CPS reporting
  cout << "turning off CPS" << endl;
  gqgmc->turnOffCPS();
  if (gqgmc->getErrorCode() != eNoProblem)
    Display_error(*gqgmc);

  cout << "get history" << endl;
  mypause();
  // Read history
  const
  uint16_t  histsize(0x100);
  uint32_t dataSaveAddress;
  uint32_t backup =16;
  dataSaveAddress = gqgmc->getDataSaveAddress();
  if (dataSaveAddress < 16) backup = 0;
  const uint32_t  address(dataSaveAddress - backup);
  uint8_t * pHistoryData;
  pHistoryData = gqgmc->getHistoryData(address,histsize);

  for(uint16_t i=0; i<histsize; i++)
  {
    cout << hex << uint16_t(pHistoryData[i]) << " ";
    if (((i+1)%16) == 0) cout << endl;
  }

  cout << "test writing configuration data" << endl;
  cout << "Manually set the save data to counts per hour" << endl;
  mypause();
  gqgmc->getConfigurationData(); // Need to update for user's change

  enum saveDataType_t  saveDataType0;
  saveDataType0 = gqgmc->getSaveDataType();
  cout << "saveDataType= " << dec << uint16_t(saveDataType0) << endl;

  uint32_t  dataSaveAddress99;
  dataSaveAddress99 = gqgmc->getDataSaveAddress();
  cout << "dataSaveAddress= " << hex << dataSaveAddress99 << endl;

  // set save data type
  cout << "set saveDatatype= " << GQLLC::eCPM << endl;
  mypause();
  gqgmc->setSaveDataType(GQLLC::eCPM);

  enum saveDataType_t  saveDataType1;
  saveDataType1 = gqgmc->getSaveDataType();
  cout << "saveDataType= " << dec << uint16_t(saveDataType1) << endl;

  cout << "reset DataSaveAddress" << endl;
  mypause();
  gqgmc->resetDataSaveAddress();
  dataSaveAddress99 = gqgmc->getDataSaveAddress();
  cout << "dataSaveAddress= " << hex << dataSaveAddress99 << endl;

  // update configuration - this takes a long time to complete
  cout << "update configuration- " << "this will take about a minute" << endl;
  mypause();
  gqgmc->updateConfigurationData();

  // Read configuration data, to get a print out of config data,
  // you need to enable the debug code in getConfiguration().
  cout << "get configuration" << endl;
  mypause();
  gqgmc->getConfigurationData();

  enum saveDataType_t  saveDataType9;
  saveDataType9 = gqgmc->getSaveDataType();
  cout << "saveDataType= " << dec << uint16_t(saveDataType9) << endl;

  cout << "get dataSaveAddress" << endl;
  uint32_t  dataSaveAddress2;
  dataSaveAddress2 = gqgmc->getDataSaveAddress();
  cout << "dataSaveAddress= " << hex << dataSaveAddress2 << endl;

  // Read history
  cout << "get history" << endl;
  mypause();
  pHistoryData = gqgmc->getHistoryData(address,histsize);

  for(uint16_t i=0; i<histsize; i++)
  {
    cout << hex << uint16_t(pHistoryData[i]) << " ";
    if (((i+1)%16) == 0) cout << endl;
  }

  cout << "turn off GQ GMC" << endl;
  mypause();
  // Turn off GMC-300 power
  gqgmc->turnOffPower();

  // Close USB port
  gqgmc->closeUSB();

  delete gqgmc;

  return 0;
} // end main()
