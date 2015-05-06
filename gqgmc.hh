// **************************************************************************
// File: gqgmc.hh
//
// Author:    Phil Gillaspy
//
// Last Modified: 04/26/2012
//
// Description:
//    Declare the class and its methods describing the capabilities
//    of the GQ Electronics LLC Geiger-Muller Counter (GQ GMC).
//    This code covers the GMC-300 and later models.
//
// INTRODUCTION:
//
// Detailed design documentation is embedded in the source code files.
// The intent is to combine source code and documentation such that a
// separate design document is unnecessary. The author's philosophy is that
// the source code should be considered as part of the design documentation.
// This philosophy was developed over many years of experience wherein the
// design document replicated so much information that was in the source
// code, that doing so generated needless opportunity for inconsistency.
// It is widely recognized that replicating information only increases
// the chances for human error and undue maintenance hardship.
//
// Since the design documentation will be embedded in the source code,
// the organization is constrained to the C/C++ compilation requirements
// that there exist a separate include and source body files. However,
// the author's philosophy is that sofware engineers find the separation
// of information between two files to be an annoyance. It would be much
// preferred that all information were located at the point where the
// software engineer needs to read the code to understand its operation,
// in other words, the source body file (.c,.cc). However, the C/C++
// convention of separate header and body files precludes that idealism.
// It will be realized that various C/C++ constructs and objects appear
// in both the header and source body files. So the following convention
// is followed as to where to embed the documentation: the documentation
// should appear where the object is defined, not where it declared.
// For example, an enumeration which must be defined as publicly
// visible, will be defined only in the header file and consequently
// should be documented in the header file. As another example, a
// class method is declared in the header file, but defined in the source
// file. Therefore the bulk of the documentation for the method should
// be placed in the source file.
//
// The documentation presented here will not cover any nuclear physics
// technical aspects of the GQ Geiger-Muller counter.
//
// GQ GEIGER-MULLER FEATURES
//
// The discussion of the GQ LLC geiger counter is drawn principally
// from the the GMC-300 model with firmware revision 2.15 and later.
// The GMC-300 records high energy particles which when passing through
// geiger-muller tube ionize the gas and create a transient current.
// Each transient pulsed current is considered a 'count'.
// The basic measured form of the radiation is reported in terms of
// counts per second, counts per minute, or the counts per minute
// averaged over an hour. The GMC-300 is also capable of recording
// the aforementioned measures in a history buffer of 64K bytes.
// The GMC-300 provides a USB interface which, however, is physically
// and logically implemented as a traditional RS-232 serial interface.
// The GMC-300 provides a variety of commands to collect data and
// set operational parameters. The reader is referred to the GQ
// Electronics LLC GMC-200 User Manual for a more thorough
// discussion of the its features.
//
//
// SOFTWARE LICENSE STATEMENT
//
// This software contained in the gqgmc.hh and gqgmc.cc files are
// licensed to the public domain under terms of the GNU GPL
// reproduced here.
// Copyright (C) 2012  Phil Gillaspy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Additionally, the author, Phil Gillaspy, requires that no copyright
// notice nor acknowledgement to the author is necessary for any
// derivative software using any or all part of this software.
//
// DEVELOPMENT AND BUILD OVERVIEW
//
// The code as written is Linux dependent, however, it should not
// be word-size dependent. So far it has only been compiled for
// 64-bit word size on Linux using GNU g++ 4.6.1. The Eclipse
// IDE was used for software development but this is not a
// necessity. A Makefile is provided so that it is perfectly
// acceptable to compile from the command line using
// "make all", "make bin", or "make lib".  The makefile creates
// a library containing the gqgmc class and its methods. This was
// done so third party usage could simply link to the library.
// Along with the makefile there are three associated files,
// Defines.mk, Targets.mk, and Patterns.mk. Defines.mk defines
// the root path of the development directory and various
// compile macros. Targets.mk defines the makefile targets and
// Patterns.mk defines makefile compilation dependencies.
// The makefile expects a specific directory structure. Given the
// the root directory of the source, there should be the following
// subdirectories existing; libs, bin, obj. The libGQGMC.a is
// located in the libs directory. Object files are placed in the
// obj directory. A sample main.c is compiled into a executable,
// gqgmc, and placed in the bin directory. The main.c contains
// sample test code to excercise the various capabilities of
// the GQ GMC geiger counter.
//
//
// DESIGN OVERVIEW
//
// This code provides an interface to GQ Electronics LLC GMC-300
// geiger counter. Therefore the functionality implemented by this
// software consists of providing access to the capabilities
// provided by the geiger counter. These capabilities are
// elucidated in the interface document titled GQ-GMC-ICD.odt.
// Generally, the public methods defined for the gqgmc class
// are matched one for one with the capabilities of the
// GMC-300. Therefore, the user is referred to the interface
// document to ascertain what functionality can be expected of
// this software. The software does make an attempt to abstract
// or otherwise mask the details of the communications mechanism.
// Some capabilities of the GMC-300 are not implemented for the
// reason that they are not meaningful to accessing the radiation
// measurements through a remote computer, or in other words,
// the capability is only useful if the user carries the geiger
// counter in hand. For example, only two of the approximately
// 50 configuration parameters are provided with get and set methods.
//
//
// INCLUDE FILE DOCUMENTATION
//
// This include for C++ string handling
#include <string>

// This include allows use of Linux predefined types
#include <stdint.h>

#ifndef gqgmc_hh_
#define gqgmc_hh_

// Define a name space for the GQ LLC geiger counter product line
namespace GQLLC
{

  // CONFIGURATION PARAMETER ENUMERATION
  //
  // Declare a globally visible enumeration to be used for calling the
  // writeConfigurationData() method. The value of the enumeration is
  // equal to the offset of the parameter in the configuration data
  // structure. This is then used as the first parameter in the call to
  // writeConfigurationData(). See writeConfigurationData() method in
  // gqgmc.cc.
  enum cfg_param_t
  {
    ePowerOnOff                    = 0,
    eAlarmOnOff                    = 1,
    eSpeakerOnOff                  = 2,
    eGraphicModeOnOff              = 3,
    eBacklightTimeoutSeconds       = 4,
    eIdleTitleDisplayMode          = 5,
    eAlarmCPMValue                 = 6,
    eCalibrationCPM0               = 8,
    eCalibrationSvUc0              = 10,
    eCalibrationCPM1               = 14,
    eCalibrationSvUc1              = 16,
    eCalibrationCPM2               = 20,
    eCalibrationSvUc2              = 22,
    eIdleDisplayMode               = 26,
    eAlarmValueuSvUc               = 27,
    eAlarmType                     = 31,
    eSaveDataType                  = 32,
    eSwivelDisplay                 = 33,
    eZoom                          = 34,
    eDataSaveAddress               = 38,
    eDataReadAddress               = 41,
    eNPowerSavingMode              = 44,
    eNSensitivityMode              = 45,
    eNCounterDelay                 = 46,
    eNVoltageOffset                = 48,
    eMaxCPM                        = 49,
    eNSensitivityAutoModeThreshold = 51,
    eSaveDate                      = 52,
    eSaveTime                      = 55,
    eMaxBytes                      = 58
  }; // end enum cfg_param_t

  // CONFIGURATION PARAMETER DATA BYTE COUNT
  //
  // Declare an globally visible enumeration to be used for calling the
  // writeConfigurationData() method. The value of the enumeration is equal
  // to the number of bytes of data passed to the writeConfigurationData()
  // method. For each enumeration element, enum cfg_param_t, there is a
  // matching enum cfg_bytecnt_t entry.
  enum cfg_bytecnt_t
  {
    ePowerOnOff_bytecnt                    = 1,
    eAlarmOnOff_bytecnt                    = 1,
    eSpeakerOnOff_bytecnt                  = 1,
    eGraphicModeOnOff_bytecnt              = 1,
    eBacklightTimeoutSeconds_bytecnt       = 1,
    eIdleTitleDisplayMode_bytecnt          = 1,
    eAlarmCPMValue_bytecnt                 = 2,
    eCalibrationCPM0_bytecnt               = 2,
    eCalibrationSvUc0_bytecnt              = 4,
    eCalibrationCPM1_bytecnt               = 2,
    eCalibrationSvUc1_bytecnt              = 4,
    eCalibrationCPM2_bytecnt               = 2,
    eCalibrationSvUc2_bytecnt              = 4,
    eIdleDisplayMode_bytecnt               = 1,
    eAlarmValueuSvUc_bytecnt               = 4,
    eAlarmType_bytecnt                     = 1,
    eSaveDataType_bytecnt                  = 1,
    eSwivelDisplay_bytecnt                 = 1,
    eZoom_bytecnt                          = 4,
    eDataSaveAddress_bytecnt               = 3,
    eDataReadAddress_bytecnt               = 3,
    eNPowerSavingMode_bytecnt              = 1,
    eNSensitivityMode_bytecnt              = 1,
    eNCounterDelay_bytecnt                 = 2,
    eNVoltageOffset_bytecnt                = 1,
    eMaxCPM_bytecnt                        = 2,
    eNSensitivityAutoModeThreshold_bytecnt = 1,
    eSaveDate_bytecnt                      = 3,
    eSaveTime_bytecnt                      = 3,
    eMaxBytes_bytecnt                      = 1
  }; // end enum cfg_bytecnt_t

  // SAVE DATA TYPE ENUMERATON
  // Define a globally visible enumeration which defines the
  // type of data that will be logged in the history buffer.
  // This enumeration is to be used in the get and set method
  // for the eSaveDataType configuration parameter. See
  // getSaveDataType() and setSaveDataType() methods in gqgmc.cc.
  enum saveDataType_t
  {
      eSaveOff = 0, // data logging is off
      eCPS     = 1, // counts per second
      eCPM     = 2, // counts per minute
      eCPH     = 3, // CPM averaged per hour
      eMaxSaveDataType = 4
  };

  // SOFTWARE KEY ENUMERATION
  //
  // Define a globally visible enumeration for use in calling the
  // sendKey (send software key) command. The software key command
  // emulates the same menu keys available on the front panel of
  // the GMC-300. The user uses this enumeration as the parameter
  // to the sendKey() call. See sendKey() method in gqgmc.cc.
  enum softkey_t
  {
    // The GQGMC User Manual numbers the software keys 1 through 4,
    // but the actual value is ASCII 0 through 3.
    // We provide one enum named following the User Manual convention.
    eKey1 = '0', eKey2 = '1', eKey3 = '2', eKey4 = '3',
    // The user can use a second, more intuitive, enumeration:
    // key 1 is the left arrow
    // key 2 is the up arrow
    // key 3 is the down arrow
    // key 4 is the enter/menu key
    eLeftArrow = '0', eUpArrow = '1', eDownArrow = '2', eEnter = '3'
  };

  // GMC ERROR CODES
  //
  // Define an enumeration of all possible error conditions. This needs
  // to be globally visible so external routines can use the error code
  // to access error text with the idea that the display of the error
  // condition and its associated text message to the end user can
  // be separated from this class. The idea is that this is a low level
  // driver which should be not contain high level GUI functionality.
  enum gmc_error_t
  {
    eNoProblem, eUSB_open_failed, eOlder_firmware, eGet_version,
    eGet_serial_number, eGet_CPM, eGet_CPS, eGet_AutoCPS, eGet_CFG,
    eErase_CFG, eUpdate_CFG, eWrite_CFG, eClear_USB,
    eGet_battery_voltage, eGet_history_data,
    eGet_history_data_length, eGet_history_data_address,
    eGet_history_data_overrun, eSet_Year, eSet_Month, eSet_Day,
    eSet_Hour, eSet_Minute, eSet_Second,
    eLast_error_code
  };

  // PUBLIC CONSTANTS
  //
  // Publicly available constants which specify the maximum allowed
  // requested length of history data and the maximum address in history
  // buffer, respectively. External users can use this to guard against
  // erroneous requests for history data. Actually, getHistoryData()
  // method has guards also.
  uint32_t const kHistory_Data_Maxsize = 0x1000;  //  4k bytes
  uint32_t const kHistory_Addr_Maxsize = 0x10000; // 64k bytes

  // CLASS DECLARATION
  //
  // The Class declaration - see gqgmc.cc for documentation
  class GQGMC
  {
    // PUBLIC METHODS
    public:

    // Constructor
    GQGMC();

    // Destructor is trivial, so coded inline
    virtual
    ~GQGMC()
    {
      delete[] mHistory_data;
    };

    // SUPPORTING PUBLIC METHODS

    // Method to call to open USB port.
    virtual
    void
    openUSB(string usb_device_name);

    // Method to close USB port
    virtual
    void
    closeUSB(void);

    // Method to clear the read (input) buffer of the USB-serial port.
    // This arguably should be a private method, but I can see the
    // outside possibility that a third party might need this.
    virtual
    void
    clearUSB();

    // Method to call to check any and all error conditions exihibited
    // by the GQGMC class, implementation is trivial so coded inline.
    virtual
    enum gmc_error_t
    getErrorCode()
    {
      return mError_code;
    };

    // Method to get a text description of the error code.
    virtual
    std::string
    getErrorText(gmc_error_t err);

    // PUBLIC METHODS PROVIDING ACCESS TO GQ GMC CAPABILITIES
    // Begin the public methods for issuing commands to the GQ GMC
    // and returning data (if any).

    // Method to get GQ GMC version.
    virtual
    std::string
    getVersion();

    // Method to get serial number
    virtual
    std::string
    getSerialNumber();

    // Method to get the count per minute value.
    virtual
    uint16_t
    getCPM();


    // Method to get the count per second value.
    virtual
    uint16_t
    getCPS();

    // Method to get voltage value of battery.
    virtual
    float
    getBatteryVoltage();

    // Method to get history data from internal flash. Returned
    // pointer points to private data history data buffer.
    virtual
    uint8_t * const
    getHistoryData(uint32_t address, uint32_t length);

    // Method to enable automatic reporting of count per second value.
    virtual
    void
    turnOnCPS();

    // Method to disable automatic reporting of count per second value.
    virtual
    void
    turnOffCPS();

    // Method to read the automatically transmitted CPS value.
    virtual
    uint16_t
    getAutoCPS();

    // Method to turn off the GQ GMC.
    virtual
    void
    turnOffPower();

    // Method to read configuration data.
    virtual
    void
    getConfigurationData();

    // Method to retrieve the data type logged in the history buffer.
    virtual
    enum saveDataType_t
    getSaveDataType();

    // Method to set the data type logged in the history buffer.
    virtual
    void
    setSaveDataType(enum saveDataType_t newSaveDataType);

    // Method to retrieve the address in the history buffer
    // where the logged data begins.
    virtual
    uint32_t
    getDataSaveAddress();

    // Method to set DataSaveAddress to zero, in other words,
    // set the address of data logging to begin in the
    // history buffer at address zero.
    virtual
    void
    resetDataSaveAddress();

    // Method to write configuration data.
    virtual
    void
    writeConfigurationData(enum cfg_param_t        cfgParameter,
                           enum cfg_bytecnt_t      cfgDataCount,
                           uint8_t const * const   cfgData);

    // Method to erase configuration data.
    virtual
    void
    eraseConfigurationData();

    // Method to make latest changes to configuration data take effect.
    // Invoke this method after setting configuration data using
    // writeConfiguration(), setDataSaveAddress(), or setSaveDataType().
    virtual
    void
    updateConfigurationData();

    // Public method to send key (emulate one of the 4 keys on the
    // front panel of the GQ GMC).
    virtual
    void
    sendKey(enum softkey_t key);

    // Public method to set the date month-day-year format. For example,
    // for January 1, 2013 use "013113".
    virtual
    void
    setDate(string date);

    // Public method to set the time of day: hour:minutes:seconds format.
    // For example, for 30 minutes past 1PM use "133000".
    virtual
    void
    setTime(string time);

    // There are no public data members


    private:

    // PRIVATE DATA

    // The device name of the USB port connected to the GQ GMC.
    std::string             mUSB_device;

    // Forced to use C style IO because C++ streams does not support
    // setting line discipline sufficiently on the serial port.
    int                     mUSB_serial;

    // Error flag for indicating failure, see enumeration of error codes
    // declared above enum gmc_error_t.
    enum gmc_error_t        mError_code;

    // The failure of each command results from failing to read the
    // returned data. So we need a separate status for the read failure
    // so it can be tested privately.
    bool                    mRead_status;

    // Special flag indicating that automatic reporting of counts per
    // second is turned off or on (off==false, on==true). This should
    // and is intended to be implemented as a mutex if and when threading
    // is implemented.
    bool                    mCPS_is_on;

    // The USB port uses big endian transfer (ie, MSB transmitted 1st).
    // This flag indicates the endianess of the host CPU. This is set
    // in the constructor by calling isBigEndian() method.
    bool                    mBig_endian;

    // This real number is the GQ GMC's firmware revision. This is needed
    // because the GQ GMC changed the command string for various commands
    // between versions of the firmware. This means that not all commands
    // will work with firmware prior to 2.15.
    float                   mFirmware_revision;

    // Storage for the history data, maximum of 4K bytes to be allocated
    // in constructor. The user can only request a max of 4K at a time.
    uint8_t *               mHistory_data;

    // Declare a structure for storage of the configuration data.
    // This is a replica of the GQ GMC's internal configuration data.
    // This is referred to as the host computer's local copy of the
    // GQ GMC's NVM configuration data elsewhere in the documentation.
    // The getConfigurationData method will deposit the GQ GMC's NVM data
    // into this structure. All configuration data is the binary value.
    // Booleans are just 0 or 1. But remember that the byte order is
    // big endian for multibyte data. So multibyte data may require
    // reversing the byte order for display to a user. Those data
    // whose semantics are understood are documented below, otherwise
    // no explanation means either we have no interest or the parameter
    // is better left to being updated physically using the GQ GMC's
    // front panel keys.
    struct cfg_data_t
    {
      uint8_t powerOnOff;              // byte 0
      uint8_t alarmOnOff;
      uint8_t speakerOnOff;
      uint8_t graphicModeOnOff;
      uint8_t backlightTimeoutSeconds; // byte 4
      uint8_t idleTitleDisplayMode;
      uint8_t alarmCPMValueHiByte;
      uint8_t alarmCPMValueLoByte;
      uint8_t calibrationCPMHiByte_0; // byte 8
      uint8_t calibrationCPMLoByte_0;
      uint8_t calibrationSvUcByte3_0;
      uint8_t calibrationSvUcByte2_0;
      uint8_t calibrationSvUcByte1_0; // byte 12
      uint8_t calibrationSvUcByte0_0;
      uint8_t calibrationCPMHiByte_1;
      uint8_t calibrationCPMLoByte_1;
      uint8_t calibrationSvUcByte3_1; // byte 16
      uint8_t calibrationSvUcByte2_1;
      uint8_t calibrationSvUcByte1_1;
      uint8_t calibrationSvUcByte0_1;
      uint8_t calibrationCPMHiByte_2; // byte 20
      uint8_t calibrationCPMLoByte_2;
      uint8_t calibrationSvUcByte3_2;
      uint8_t calibrationSvUcByte2_2;
      uint8_t calibrationSvUcByte1_2; // byte 24
      uint8_t calibrationSvUcByte0_2;
      uint8_t idleDisplayMode;
      uint8_t alarmValueuSvUcByte3;
      uint8_t alarmValueuSvUcByte2;   // byte 28
      uint8_t alarmValueuSvUcByte1;
      uint8_t alarmValueuSvUcByte0;
      uint8_t alarmType;
      // saveDataType specifies both the interval of data logging where
      // 0 = data logging is off, 1 = once per second, 2 = once per minute,
      // 3 = once per hour and the type of data saved where 0 = don't care,
      // 1 = counts/second, 2 = counts/minute, 3 = CPM averaged over hour.
      // Whenever this is changed, the GQ GMC inserts a date/timestamp
      // into the history data buffer.
      uint8_t saveDataType;           // byte 32
      uint8_t swivelDisplay;
      uint8_t zoomByte3;
      uint8_t zoomByte2;
      uint8_t zoomByte1;              // byte 36
      uint8_t zoomByte0;
      // dataSaveAddress represents the address of the first sample following
      // the insertion of a date/timestamp or label tag into the data buffer.
      // Periodically, a label or date/timestamp will be put into the buffer
      // without a change made to dataSaveAddress. So you always have to be
      // on the lookout for 55AA sequence when parsing data buffer. But you
      // have to do that anyway because you might encounter double byte data.
      uint8_t dataSaveAddress2;
      uint8_t dataSaveAddress1;
      uint8_t dataSaveAddress0;       // byte 40
      // dataReadAddress semantics is unknown. As far as I have seen,
      // its value is always zero.
      uint8_t dataReadAddress2;
      uint8_t dataReadAddress1;
      uint8_t dataReadAddress0;
      uint8_t nPowerSavingMode;       // byte 44
      uint8_t nSensitivityMode;
      uint8_t nCounterDelayHiByte;
      uint8_t nCounterDelayLoByte;
      uint8_t nVoltageOffset;         // byte 48
      uint8_t maxCPMHiByte;
      uint8_t maxCPMLoByte;
      uint8_t nSensitivityAutoModeThreshold;
      // saveDateTimeStamp is the date/timestamp of the logging run,
      // all data following up to the next date/timestamp are marked
      // in time by this date/timestamp, where
      uint8_t saveDateTimeStampByte5; // = year (last two digits) // byte 52
      uint8_t saveDateTimeStampByte4; // = month
      uint8_t saveDateTimeStampByte3; // = day
      uint8_t saveDateTimeStampByte2; // = hour
      uint8_t saveDateTimeStampByte1; // = minute // byte 56
      uint8_t saveDateTimeStampByte0; // = second
      // maxBytes is always 0xff
      uint8_t maxBytes;
      uint8_t spare[197]; // add spare to total 256 bytes
    } mCFG_Data;

    // PRIVATE METHODS


    // Method to load configuration data which writes all 256 bytes
    // of the configuration data to the GQ GMC in sequence. This is
    // called by updateConfigurationData().
    virtual
    void
    loadConfigurationData();

    // Because of the returned byte stream from the GQ GMC has no protocol,
    // that is, it is just a sequence of N bytes, we need a specialized
    // read. The write is straight forward, but is combined with the read
    // for reasons of code commonality. So we will call the write followed
    // by read as the 'communicate' method.
    void
    communicate(const std::string cmd, char * retdata, uint32_t retbytes);

    // This is the basic method to transmit the command to the GQ GMC.
    // communicate() calls this to transmit command.
    void
    sendCmd(const std::string cmd);

    // This is the basic method to read the return bytes from the command.
    // communicate() calls this to read the return byte stream of the
    // command.
    void
    readCmdReturn(char * retdata, uint32_t retbytes);

    // This is the method to determine endianess of host CPU. This is to be
    // called by constructor once and once only.
    bool
    isBigEndian(void);

  }; // end class GQGMC

} // end namespace GQLLC

// DOCUMENTATION CONTINUES IN gqgmc.cc
#endif  // gqgmc_hh_
