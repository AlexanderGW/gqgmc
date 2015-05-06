// **************************************************************************
// File: gqgmc.cc
//
// Author:    Phil Gillaspy
//
// Last Modified: 04/26/2012
//
// Synopsis:
//   Define the class and its methods describing the capabilities
//   of the GQ Electronics LLC Geiger-Muller Counter (GQ GMC).
//   this code applies to model 300 and later geiger counters.
//
// CONTINUATION OF DOCUMENTATION FROM gqgmc.hh
//
//
// C++ includes
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ios>
using namespace std;

// These are the C stdio includes needed for configuring the serial port.
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

// These are GQ GMC project specific includes
#include "gqgmc.hh"
using namespace GQLLC;

// LOCAL CONSTANTS
//
// GQ GMC COMMANDS
/*
 Here we declare local constants which are the ASCII strings for the
 various commands to the GQ GMC. These are not declared as data
 members of the GQGMC class because there is just no need to do so.
 The user never need know the actual string commands because there
 is a dedicated public method for each command.

 All commands start with '<'. That is followed by ASCII text,
 followed by ">>". Commands which take binary data as parameters
 must be dynamically constructed within the command method.
*/

static const string  get_serial_cmd        = "<GETSERIAL>>";
static const string  get_version_cmd       = "<GETVER>>";
static const string  get_voltage_cmd       = "<GETVOLT>>";
static const string  get_cpm_cmd           = "<GETCPM>>";
static const string  get_cps_cmd           = "<GETCPS>>";
static const string  get_cfg_cmd           = "<GETCFG>>";
static const string  erase_cfg_cmd         = "<ECFG>>";
static const string  update_cfg_cmd        = "<CFGUPDATE>>";
static const string  turn_on_cps_cmd       = "<HEARTBEAT1>>";
static const string  turn_off_cps_cmd      = "<HEARTBEAT0>>";
static const string  turn_off_pwr_cmd      = "<POWEROFF>>";
// The get_history data command has to be formed dynamically since
// the additional data are parameters of the command.
// This is also true of the write configuration data command.
// The send software key command also needs a extra data parameter
// and so it needs to be formed dynamically.
// The set time and set date commands also use a hexadecimal parameter
// and so need to be formed dynamically.

// FIRMWARE CONSTANT
// Older firmware is any revision prior to 2.23. Not all commands will
// work on older firmware because some commands were not supported or
// the command was changed.
static
float
const          kNew_Firmware = 2.23f;

// Size of the NVM configuration data on the GQ GMC
static
uint32_t
const          kNVMSize = 256;

// C STYLE SERIAL PORT CONFIGURATION
// The termios structure is needed to configure the USB/serial port
// for baudrate and raw line discipline.
static struct
termios        settings;

// debug code
// LOCAL UTILITIES
// These Hex.... routines are nice utilities to convert a raw
// binary byte to an ASCII digit, ie, 1 digit string as hex data.
// The principle use is to print out the data for debugging.
// These were plagarized and adapted from code on the internet.

struct HexCharStruct
{
  unsigned char c;
  HexCharStruct(unsigned char _c) : c(_c) { }
};

static
std::ostream& operator<<(std::ostream& o, const HexCharStruct& hs)
{
  o.width(2);
  o.fill('0');
  return (o << std::hex << (int)hs.c);
}

static
HexCharStruct Hex(unsigned char _c)
{
  return HexCharStruct(_c);
}


// GQGMC CLASS CONSTRUCTOR
//
// Constructor implementation needs to initialize the private
// variables, allocate memory for data history, and determine
// endianess of processor.
GQGMC::GQGMC()
{
  // CPS is false until proven true
  mCPS_is_on             = false;
  // Allocate history_data on heap
  mHistory_data          = new uint8_t[kHistory_Data_Maxsize];
  // Determine endianess of host CPU
  mBig_endian            = isBigEndian();
} // end GQGMC constructor


// GQGMC CLASS DESTRUCTOR
// Destructor implemented inline in class declaration (see gqgmc.hh)

// SUPPORTING PUBLIC METHODS
//
// Method to call to open USB port. We are using classic C style IO
// because the C++ IO streams did not have enough flexibility to
// control serial port.  I could not set the baudrate using iostream.
// Apparently, C++ IO is fixated on files and not hardware devices.
// Input usb_device_name is the name of the USB to serial device
// driver name.
void
GQGMC::openUSB(string usb_device_name)
{
  // Assign name of USB device in private data
  mUSB_device = usb_device_name;  // for example, /dev/usb/ttyUSB0

  // Open the USB port using a USB to serial converter device driver.
  // We are using the C stdio open only because it allows the low
  // level control necessary to set line discpline and baudrate.
  // The line discpline is raw, that is, no character translation,
  // no handshaking, no nothing. The return data from the GQ GMC
  // are always raw binary (well, some of the raw binary might be
  // ASCII). Output to the GQ GMC is always ASCII with
  // some commands taking on binary data parameters.

  cout << mUSB_device.c_str() << endl;

  // Open usb serial port for reading and writing.
  mUSB_serial = open(mUSB_device.c_str(), O_RDWR);

  // If port opened successfully, then proceed to set line discipline.
  if (mUSB_serial != -1)
  {
    mError_code = eNoProblem;

    // Since USB serial is opened successfully, set baud rate to 57600
    // and other settings to force raw binary data exchange.
    fcntl(mUSB_serial, F_SETFL, 0);

    memset(&settings,0,sizeof(settings));
    settings.c_cflag     = CS8 | CREAD | CLOCAL;
    settings.c_iflag     = 0; // setting line discpline to raw binary
    settings.c_oflag     = 0;
    settings.c_lflag     = 0;
    settings.c_cc[VMIN]  = 0; // This combo of VMIN & VTIME means to wait
    settings.c_cc[VTIME] = 5; // a maximum of 0.5 seconds for each
                              // requested or expected byte of data.
    cfsetispeed(&settings, B57600);
    cfsetospeed(&settings, B57600);
    tcsetattr(mUSB_serial, TCSANOW, &settings);

    // Now that the port is successfuly opened, we secretly (unknown to
    // the user) interrogate the GMC-300 to determine the firmware revision.
    // For older firmware, a warning is issued to the user. Older firmware
    // will not support all commands.
    string vers = getVersion();
    if (mRead_status == true)
    {
      // Parse version string to extract firmware revision. The revision
      // is a f4.1 formatted string in the last four characters of string.
      stringstream firmware_rev;
      firmware_rev << vers[10] << vers[11] << vers[12] << vers[13];

      // Convert revision string to float in order to compare
      firmware_rev >> mFirmware_revision;

      // There may be a change to commands caused by change to
      // firmware. We only need to compare if revision is old or new.
      if (mFirmware_revision < kNew_Firmware)
        mError_code = eOlder_firmware;

      // Get a fresh copy of the GQ GMC's NVM configuration data.
      getConfigurationData();

    } // end if (error_code == eNoProblem)
    // else error_code will have been set by getVersion()
  }
  else // usb not opened successfully
  {
    mError_code = eUSB_open_failed;
  } // end  if (usb_serial != -1)

  // It is the responsibility of the caller to test the error_code.
  return;
} // end openUSB()

// Method to close USB port is pretty trivial. Implementation is
// to just close port.
void
GQGMC::closeUSB()
{
  close(mUSB_serial);
  return;
} // end closeUSB()


// clearUSB is public method to clear the read (input) buffer of
// the serial port. This method exists due to vagaries of the
// turn_on_cps_cmd. Since there is no protocol for the returned
// data (ie, no start, no stop, no ack, no nak), the synchronization
// of the getAutoCPS() method with the GQ GMC is flakey. Consequently,
// when the turn_off_cps_cmd happens, there may be left over
// data in the USB input buffer. So this method simply reads the
// USB input buffer until it is empty.
void
GQGMC::clearUSB()
{
  uint32_t  rcvd(0);
  char      inp;

  // Assume that there isn't that much left over from any previous
  // read. In other words, we couldn't ever get that far off.
  // So read only 10 bytes.
  const
  uint16_t  kMaxtries(10);

  // Read returned data until input buffer is empty as indicated by
  // rcvd = 0 or not. If rcvd = 0 then the buffer is empty and that is
  // what we want.
  for(uint16_t i=0; i<kMaxtries; i++)
  {
    rcvd = read(mUSB_serial, &inp, 1);
    if (rcvd == 0) break;
  } // end for

  // Not good, there are still more characters in input buffer,
  // so declare error. The calling routine could try again.
  if (rcvd > 0)
    mError_code = eClear_USB;

  return;
}// end clearUSB()

// Public method getErrorCode() to check error code of GQGMC class.
// Implemented inline in class declaration (see gqgmc.hh)

// Public method to get a text description of the error code. Intended
// to be used by external code to display this text to user. External
// code would know the error code from the most recent call to a method.
// If the error code is non-zero, then call this routine to obtain a
// textual description of the error.
string
GQGMC::getErrorText(gmc_error_t err)
{
  stringstream err_msg; // error message returned in this string

  switch(err)
  {
    case eNoProblem:  // The user should not call with this error code
      err_msg << "";
      break;          // because they are expected to test the code first.

    case eUSB_open_failed:
      err_msg << "The USB port did not open successfully." << endl;
      break;

    case eOlder_firmware:
      err_msg << "Your GQ GMC has older firmware." << endl
              << "Some commands may not work." << endl;
      break;

    case eGet_version:
      err_msg << "The command to read the version number of the firmware "
              << "failed." << endl;
      break;

    case eGet_serial_number:
      err_msg << "The command to read the serial number failed." << endl;
      break;

    case eGet_CPM:
      err_msg << "The command to read the counts per minute failed." << endl;
      break;

    case eGet_CPS:
      err_msg << "The command to read the counts per second failed." << endl;
      break;

    case eGet_AutoCPS:
      err_msg << "The command to read auto counts per second failed." << endl;
      break;

    case eGet_CFG:
      err_msg << "The command to get configuration data failed." << endl;
      break;

    case eErase_CFG:
      err_msg << "The command to erase configuration data failed." << endl;
      break;

    case eUpdate_CFG:
      err_msg << "The command to update configuration data failed." << endl;
      break;

    case eClear_USB:
      err_msg << "Failed to clear USB input buffer. You should power "
              << "cycle GQ GMC." << endl;
      break;

    case eGet_battery_voltage:
      err_msg << "The command to read the battery voltage failed." << endl;
      break;

    case eGet_history_data:
      err_msg << "The command to read the history data failed." << endl;
      break;
    case eGet_history_data_length:
      err_msg << "The requested data length of the history command cannot "
              << "exceed " << dec << kHistory_Data_Maxsize << " bytes." << endl;
      break;
    case eGet_history_data_address:
      err_msg << "The address of the history command cannot exceed "
              << dec << kHistory_Addr_Maxsize << " bytes." << endl;
      break;
    case eGet_history_data_overrun:
      err_msg << "The history data length added to the address cannot exceed "
              << dec << kHistory_Addr_Maxsize << " bytes." << endl;
      break;

    case eSet_Year:
      err_msg << "The set year command failed." << endl;
      break;

    case eSet_Month:
      err_msg << "The set month command failed." << endl;
      break;

    case eSet_Day:
      err_msg << "The set day command failed." << endl;
      break;

    case eSet_Hour:
      err_msg << "The set hour command failed." << endl;
      break;

    case eSet_Minute:
      err_msg << "The set minute command failed." << endl;
      break;

    case eSet_Second:
      err_msg << "The set second command failed." << endl;
      break;

    default:          // this should never happen since user should have
      break;          // obtained a valid code using getErrorCode().
  } // end switch(err)
  return err_msg.str();
} // end get_error_info()


// PUBLIC METHODS PROVIDING ACCESS TO GQ GMC CAPABILITIES

// Public method to get GQ geiger counter model name and firmware revision.
// Example of returned data is "GMC-300Re2.11". Note that this is the only
// command that has all ASCII as the return data. The command string is
// get_version_cmd (see GQ  GMC COMMANDS above).
string
GQGMC::getVersion()
{
  // The GMC returns 14 characters as the version in an alphanumeric string.
  const
  uint32_t versize = 14;
  char     version[versize+1]; // add one for null, '\0'

  // Issue the command to get the version and read the returned data
  communicate(get_version_cmd, version, versize);

  // If reading data failed, fake returned data and setup error code,
  if (mRead_status == false)
  {
    stringstream ss;
    ss << "invalidinvalid"; // how convenient, exactly 14 characters
    ss >> version;
    mError_code = eGet_version;
  }
  // else the version is returned as an ASCII string in version.

  return string(version);
} // end getVersion()

// Public method to get serial number. The serial number is returned as 7
// binary bytes. For example, if the GQ GMC returns  00 30 00 E3 4A 35 1A,
// total 7 bytes in raw binary, then the serial number is "003000E34A351A".
// Notice how each nibble is effectively a single ASCII hexadecimal digit.
// The command string is get_serial_cmd (see GQ GMC COMMANDS above).
string
GQGMC::getSerialNumber()
{
  const
  uint32_t    sernumsize = 7;             // 7 bytes of returned data
  char        serial_number[sernumsize+1];// returned data
  uint8_t     nibble[2*sernumsize];       // each nibble of returned data

  stringstream  ss;

  // Issue the command to get serial number and read returned data.
  communicate(get_serial_cmd, serial_number, sernumsize);

  // If the read of the returned data succeeded, convert raw data to string.
  if (mRead_status == true)
  {
    // Convert each nibble to an 8-bit int because each nibble
   	// represents a separate serial number digit.
    for(uint32_t i=0; i<sernumsize; i++)
    {
      nibble[(i*2)]   = (serial_number[i] & 0xF0) >> 4;
      nibble[(i*2)+1] = (serial_number[i] & 0x0F);
      // Convert byte array to string of hex ASCII char. Do not use
      // Hex utility because that forces each arg to have 2 char field.
      ss << hex << uint16_t(nibble[i*2]) << uint16_t(nibble[(i*2)+1]);
    }
  }
  else // for read failure, set byte array to null and set error code
  {
    for(uint32_t i=0; i<sernumsize; i++) nibble[i] = '\0';
    ss << nibble;
    mError_code = eGet_serial_number;
  }

  return ss.str();
} // end getSerialNumber()

// Public method to get the count per minute value. Command string is
// get_cpm_cmd (local constants above). The return data is two bytes
// of binary data. Typically, the background radiation is 10 to 30
// counts per minute. The command is get_cpm_cmd (see GQ GMC COMMANDS
// above).
uint16_t
GQGMC::getCPM()
{
  const
  uint32_t cpmsize = 2;          // 2 bytes of returned data
  char     cpm_char[cpmsize+1];  // returned data
  uint16_t cpm_int = 0;          // this will be the returned value

  // Issue command to get CPM and read returned data
  communicate(get_cpm_cmd, cpm_char, cpmsize);

  // if read of returned data succeeded, convert raw data to integer
  if (mRead_status == true)
  {
  	// 1st byte is MSB, but note that upper two bits are reserved bits.
    // Note that shifting and bitmasking performed in uP register, so
    // endianess is irrevelant.
    cpm_int |= ((uint16_t(cpm_char[0]) << 8) & 0x3f00);
    cpm_int |=  (uint16_t(cpm_char[1]) & 0x00ff);
  }
  else  // else failure will return cpm = 0 and set error code
  {
    mError_code = eGet_CPM;
  }

  return cpm_int;
} // end getCPM()


// Public method to get the count per second value. Command string is
// get_cps_cmd (local constants above). The return data is two bytes
// of binary data. Typically, the background radiation is 0 to 3
// counts per second. The command is get_cps_cmd (see GQ GMC COMMANDS
// above).
uint16_t
GQGMC::getCPS()
{
  const
  uint32_t cpssize = 2;          // 2 bytes of returned data
  char     cps_char[cpssize+1];  // returned data
  uint16_t cps_int = 0;          // this will be the returned value

  // Issue command to get CPM and read returned data
  communicate(get_cps_cmd, cps_char, cpssize);

  // If read of returned data succeeded, convert raw data to integer
  if (mRead_status == true)
  {
    // 1st byte is MSB, but note that upper two bits are reserved bits.
    // Note that shifting and bitmasking performed in uP register, so
    // endianess is irrevelant.
    cps_int |= ((uint16_t(cps_char[0]) << 8) & 0x3f00);
    cps_int |=  (uint16_t(cps_char[1]) & 0x00ff);
  }
  else  // else failure will return cps = 0 and set error code.
  {
    mError_code = eGet_CPS;
  }

  return cps_int;
} // end getCPS()

// Public method to read voltage value of battery. The GQ GMC returns
// a single byte whose integer value converted to a float divided by 10
// equals the battery voltage. For example, 0x60 = 96 converts to 9.6 Volts.
// The ideal value is 9.8 volts. So practically speaking, we should not
// expect to see a value higher than 100. The command is get_voltage_cmd
// (see GQ GMC COMMANDS above). If the voltage falls below 7.5V, GQ LLC
// says the geiger counter cannot be expected to operate properly.
float
GQGMC::getBatteryVoltage()
{
  const
  uint32_t     voltsize = 1;             // one byte of returned data
  char         voltage_char[voltsize+1]; // returned data
  float        voltage(0.0f);            // will be the returned value

  // Issue command to get battery voltage and read returned data.
  communicate(get_voltage_cmd, voltage_char, voltsize);

  // If read of returned data succeeded, convert raw data to float,
  if (mRead_status == true)
  {
    int32_t  voltage_int = int16_t(voltage_char[0]);
    voltage = float(voltage_int) / 10.0;
  }
  else  // else for failure, voltage is zero and set error code.
  {
    mError_code = eGet_battery_voltage;
  }

  return  voltage;
} // end getBatteryVoltage()

// Public method to get history data from GQ GMC's history buffer.
// The user requests the data given the offset into the GQ GMC's 64k
// byte buffer and the length (amount) of data. The data logging can be
// set to record either CPM or CPS and the data can be recorded either
// every second, minute, or hour. The length parameter is the number
// of bytes to be read, but note that the history data itself is
// intermixed with special tag data indicating either (1) date/timestamp,
// (2) uint16 data, or (3) ASCII tag. So the number of bytes read is not
// the number of data samples. You don't know the number of data
// samples until you have parsed the history data returned.
//
// The date/timestamp string is 55AA00YYMMDDHHMMSS55AADD where
// 55AA is start of sequence marker,
// 00 is the enum code that the date/timestamp follows,
// YY is year, MM is month,
// HH is 24 hour time, MM is minutes, SS is seconds,
// 55AA is end of sequence marker,
// DD is indicator of saved data type where
// 0 = off (history is off),
// 1 = CPS every second,
// 2 = CPM every minute,
// 3 = CPM recorded once per hour.
//
// The uint16 data (two bytes) string is 55AA01DHDL where
// 55AA is start of sequence marker,
// 01 is the enum code that two byte data follows,
// DH is the MSB, DL is the LSB.
// There is no 55AA end of sequence marker.
// This particular special tag represents a data sample whose value
// exceeded 255 and thus needed two bytes. A history buffer of two byte
// data samples would look something like 55AA02a355AA027555AA01b8 etc.
// If the radiation is continuously high, then all data will be > 255
// and the number of data samples in the history buffer will greatly
// diminished (decreased by factor of 2).
//
// The ASCII tag string is 55AA02LLCCCCCCCC.... where
// 55AA is start of sequence marker,
// 02 is the enum code indicating that the tag ASCII string follows,
// LL is the number of ASCII characters in the tag,
// CC is an ASCII character, there being LL number of ASCII characters.
// There is no 55AA end of sequence marker.
//
// The history data itself is either CPS or CPM. If CPS, the data byte
// is typically a sequence of 00, 01's, 02's, and occassionally a 03.
// If CPM, the data is typically anywhere from 0x0a (10 decimal)
// up to 0x1e (30 decimal). But remember that any time the count per
// time exceeds 255, then the special 55AADHDL sequence kicks in.
//
// The history data continues until either the date/timestamp or label
// tag special sequences are inserted into the history buffer.
//
// If the user requests an area of the history buffer which has no data as
// yet recorded, then the start of the history buffer is used, in other
// words, it is as if the user requested address zero.
//
// If the user requests a small area of the history buffer, then chances
// are high that the data retrieved will have no date/timestamp embedded.
// So it is impossible to know when the data was recorded. The 64K history
// buffer is divided into 4K blocks and it is guaranteed that there will
// be a date/timestamp somewhere within each 4K block. For these reasons,
// the user should request no less than 4K bytes on 4K byte boundary.
// Note that 4K bytes is also the maximum request allowed. So for all
// practical purposes, all get history commands should be 4K bytes.
//
// The following is typical history buffer log of counts per second
// (leading 0 is suppressed). Note that the data/timestamp is
// embedded about half way through (2012, April 1, 17:10).
// 1 0 1 1 0 0 0 1 0 1 0 0 0 0 2 0 0 0 0 0 0 2 0 0 0 2 0 0 0 0 0 0
// 0 1 0 0 0 0 0 0 1 0 0 2 0 0 1 0 1 0 2 0 0 0 1 2 1 0 0 0 1 0 1 1
// 1 1 0 0 0 0 0 0 1 1 0 1 0 0 0 1 0 4 1 0 0 0 0 1 0 0 2 2 0 2 0 0
// 0 2 1 1 0 0 1 1 0 0 1 1 2 0 0 3 2 0 1 0 0 0 2 0 0 2 0 0 0 0 0 0
// 0 0 1 2 0 0 0 55 aa 0 c 4 1 11 1f a 55 aa 1 1 0 0 0 0 0 0 1 0 0 0 1 1
// 0 1 0 0 0 0 0 0 0 0 0 0 0 1 0 1 1 1 1 1 0 0 0 1 0 1 0 0 0 0 0 1
// 1 0 0 0 0 0 0 0 1 0 0 1 0 2 1 0 1 2 1 1 0 1 1 0 0 0 0 0 0 0 0 0
// 0 0 1 0 0 0 0 0 0 2 0 0 0 0 0 1 1 3 0 0 2 0 0 0 0 0 0 1 1 0 1 0
//
// The following is a typical history buffer log of counts per
// minute (leading 0 is suppressed). Note that there are two
// date/timestamps. The first indicates the current logging is
// set to counts per second, but the second changes that to
// counts per minute. Also notice, there is a single two byte
// sample (55AA021b). The ff data indicates an area of the
// history buffer that has no data.
// 0 0 0 0 0 0 1 0 0 0 1 1 0 0 0 0
// 1 3 0 0 0 0 0 1 1 0 0 2 0 0 0 0
// 0 0 55 aa 0 c 4 2 11 e 34 55 aa 1 55 aa
// 0 c 4 2 11 e 35 55 aa 2 1b 16 12 18 18 16
// 14 13 18 13 18 24 a6 ff ff ff ff ff ff ff ff ff
// ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
//
// Given that the desired amount of history buffer has been
// retrieved, parsing of the data can proceed by working from the
// beginning to end or vica versa. Either way, parsing would be
// looking for the embedded date/timestamp in order to fix the
// point in time when the data logging begins. The first data
// sample after the date/timestamp is therefore the beginning of
// a new recording. Proceeding forward, each successive byte is
// the next count per second (CPS) or count per minute (CPM) data
// sample. However, caution must be taken to check for the
// 55AA sequence which could be either a new date/timestamp or a
// new label, or a two byte data sample. In the case of a
// date/timestamp, the parsing must check for a change in the
// sampling mode, in other words, does the logging change from CPS
// to CPM or vica versa.
//
// The address of the first data sample of the newest log run
// is maintained in the configuration data as the parameters,
// dataSaveAddress0, dataSaveAddress1, dataSaveAddress3. This address
// represents the offset from the beginning of the history buffer.
// The first data sample will always be preceeded by date/timestamp.
// So this date/timestamp should be read in order to orient
// the data log. Any time the sampling mode is changed, the
// dataSaveAddress0/1/2 is updated, otherwise it remains the same
// until the logging wraps around to the same address.
// Incidentally, the configuration data also maintains the parameter,
// saveDataType, which contains the sampling mode.
//
// If you intend to use the GQ GMC to log radiation data for
// any time longer than a few hours, it is not recommended to
// use the built-in logging capability of the GQ GMC. Instead,
// do the logging yourself by simply calling getCPM() method,
// letting the host computer do the storage and time/date tracking.
uint8_t * const
GQGMC::getHistoryData(uint32_t address, uint32_t length)
{
  // Initialize history data to zeroes. The max allowed request is
  // kHistory_Data_Maxsize = 4K bytes. Even if the user requests less
  // than kHistory_Data_Maxsize, we zero out the whole array.
  for(uint32_t i=0;i<kHistory_Data_Maxsize;i++)
    mHistory_data[i] = 0;

  // Check the validity of the input arguments, return if invalid.
  mError_code = eNoProblem;
  // 1st check length
  if (length > kHistory_Data_Maxsize)
    mError_code = eGet_history_data_length;
  // check address
  if (address > kHistory_Addr_Maxsize)
    mError_code = eGet_history_data_address;
  // check address plus length
  if ((address + length) > kHistory_Addr_Maxsize)
    mError_code = eGet_history_data_overrun;
  // Trap error here, return immediately if there is an error
  if (mError_code != eNoProblem)
    return &mHistory_data[address];

  // Since the request is within boundaries, formulate history command.
  string get_history_data_cmd = "<SPIR";

  // Pack address and data length into get_history_data_cmd with big
  // endian byte order.

  // Transmit byte order as MSB first. Note we are using a 32-bit int,
  // but the MSB will always be 0 since address is max 24 bits in size.

  // pack address
  get_history_data_cmd += uint8_t((address >> 16) & 0x000000ff); // msb
  get_history_data_cmd += uint8_t((address >>  8) & 0x000000ff);
  get_history_data_cmd += uint8_t((address >>  0) & 0x000000ff); // lsb

  // pack length
  get_history_data_cmd += uint8_t((length >> 8) & 0x00ff);  // lsb
  get_history_data_cmd += uint8_t((length >> 0) & 0x00ff);  // msb

  // And of course, command finishes with ">>"
  get_history_data_cmd += ">>";

  // Issue command to get history data and read returned data
  communicate(get_history_data_cmd, (char *)&mHistory_data[0], length);

  // If read of returned data failed, set error code.
  if (mRead_status == false)
  {
    mError_code = eGet_history_data;
  }

  // Note that the returned data is a byte array that has to be parsed
  // further in order to extract the actual history data.
  return &mHistory_data[0];
} // end getHistoryData()

// turnOnCPS is public method to enable automatic reporting of the
// count per second (CPS) value. First, I would say don't use this
// command. Since the returned data has no protocol, that is, there
// is no start/stop marker, no identification, no nothing but a
// sequence of bytes, any other command issued while CPS is turned on
// will take extraordinary effort not to confuse its returned data
// with the CPS data. To handle it correctly, you would have to
// wait for the CPS data to be returned, and then issue the command.
// This would be most safely done by creating a new thread to
// read the CPS and using a mutex to signal opportunity to issue a
// separate command. The command is turn_on_cps_cmd
// (see GQ GMC COMMANDS above). The returned data is always two
// bytes so that samples > 255 can be reported (even though unlikely).
void
GQGMC::turnOnCPS()
{
  sendCmd(turn_on_cps_cmd);
  // There is no pass/fail return from GQ GMC
  // Set flag that auto-transmission of CPS is turned on. This is to be
  // changed to a mutex when threading is implemented.
  mCPS_is_on = true;

  return;
} // end turnOnCPS()

// turnOffCPS is the public method to disable automatic reporting
// of count per second value. The command is turn_off_cps_cmd
// (see GQ GMC COMMANDS above). Another complication of turning
// on the auto-CPS, is that turning it off is asynchronous with
// it being reported. So there may a left over data sample which
// has to be cleaned up.
void
GQGMC::turnOffCPS()
{
  sendCmd(turn_off_cps_cmd);
  // turnOffCPS command has no return data, so there is no way
  // to know if the  command succeeds or fails.
  // Set flag of CPS auto-transmission to false
  mCPS_is_on = false;

  // Since turning off CPS is asynchronous with the GQ GMC's transmission
  // of the CPS, we will attempt to clear the input buffer of any left
  // over data.
  clearUSB();

  return;
} // end turnOffCPS()

// getAutoCPS is public method to read CPS. This is the method to
// be called after the user turns on the auto-CPS. If the user has
// not called turnOnCPS(), then this method will issue error
// messages.
uint16_t
GQGMC::getAutoCPS()
{
  const
  uint32_t cpssize = 2;
  char     cps_char[cpssize+1];
  uint16_t cps_int = 0;

  // Issue command to get CPS and read returned data. Note that
  // we are calling readCmdReturn directly, therefore bypassing
  // the communicate method. The user should have already issued
  // turn_on_cps_cmd.
  readCmdReturn(cps_char, cpssize);

  // If read of returned data succeeded, convert raw data to integer,
  if (mRead_status == true)
  {
    // 1st byte is MSB, but note that upper two bits are reserved
    // bits. Assume shift & bitmasking take place in register which
    // is independent of big endian vs little endian architecture.
    cps_int |= ((uint16_t(cps_char[0]) << 8) & 0x3f00);
    cps_int |=  (uint16_t(cps_char[1]) & 0x00ff);
  }
  else  // else failure will return cps = 0 and set error code
  {
    mError_code = eGet_AutoCPS;
  }

  return cps_int;
} // end getAutoCPS()


// turnOffPower public method turns off the GQ GMC-300. The command
// is turn_off_pwr_cmd (see GQ GMC COMMANDS above). This will turn
// off the GQ GMC, so there shouldn't be any other commands issued
// to the GQ GMC following this command.
void
GQGMC::turnOffPower()
{
  sendCmd(turn_off_pwr_cmd);
  // Note that power off cannot fail because the GQ GMC returns nothing.
  return;
} // end turnOffPower()

// CONFIGURATION DATA
//
// Setting the configuration data of the GQ GMC is not a
// straight forward procedure. The following statement is the
// principle reason why: the GQ GMC does not use or keep a RAM copy
// of its non-volatile memory (NVM) (configuration data is in NVM).
// That condition coupled with the fact that the EEPROM
// used by the GQ GMC can only be reprogrammed
// all 256 bytes at a shot means that if you write a byte of
// configuration data and then read back the configuration data,
// you will not see your data changed as expected.
//
// All this means that in order to change the configuration
// parameters and have the GQ GMC change its operations accordingly,
// 1) the host computer must keep its own copy of NVM,
// 2) update its own copy of NVM,
// 3) issue the erase configuration command,
// 4) write all 256 bytes of configuration data at one shot,
// 5) follow immediately with an update configuration command.
//
// When the GQ GMC receives the update configuration command,
// then and only then does it re-configure its operation in
// accordance with the NVM configuration data. Keeping the host
// computer's copy of the NVM accurate and up to date can be
// problematic since behind the back of the host computer,
// the GQ GMC can be changed manually.
//
// The GQGMC software makes a valiant attempt to hide all this
// from the user. First, immediately following the opening of
// the USB port, the software silently reads the configuration
// data from the GQ GMC to obtain its own copy of NVM.
// From that point on, it is assumed that no manual changes
// to the GQ GMC will occur. The GQGMC software then reads/writes
// it own local copy of NVM. When the user issues the Update CFG
// command, the GQGMC software silently
// 1) issues the erase configuraton command,
// 2) writes all 256 bytes of NVM,
// 3) issues the update configuration command.

// The user software may at any time cause a Get configuration
// command in which case, the user must be aware that the GQGMC's
// local copy of NVM will be overwritten.


// getConfigurationData public method reads configuration data. You
// don't request pieces of the configuration data, all 256 bytes
// are returned, although, there are currently only about 60
// bytes used (corresponding to about 50 parameters). The command is
// get_cfg_cmd (see GQ GMC COMMANDS above).
void
GQGMC::getConfigurationData()
{
  // Issue command to get configuration and read returned data.
  communicate(get_cfg_cmd, reinterpret_cast<char *>(&mCFG_Data),
                           sizeof(mCFG_Data));

  // If read of returned data failed, set error code.
  if (mRead_status == false)
  {
    mError_code = eGet_CFG;
  }

  //  debugging code
  /*
  uint8_t * inp = (uint8_t *)&mCFG_Data;
  for(uint32_t i=0; i<64; i++)
  {
    cout << Hex(inp[i]) << "-";
    if (i > 0)
      if (((i+1)%16) == 0) cout << endl;
    if (i > 62)break;
  }
  cout << endl;
  */
  // end debug code

  return;
} // end getConfigurationData()

// The following get methods are provided in order to access the
// configuration data, not all configuration data, but only those
// parameters which are needed to retrieve and parse the history data.

// getSaveDataType is a get method to retrieve the saved data type
// parameter in the configuration data. Note that SaveDataType is
// retrieved from the host computer's local copy of NVM
// configuration data. The return type is an enumeration
// which matches the following possibilities:
// 0 = logging is off,
// 1 = counts per second,
// 2 = counts per minute,
// 3 = CPM averaged per hour.
enum saveDataType_t
GQGMC::getSaveDataType()
{
  return ((enum saveDataType_t)(mCFG_Data.saveDataType));
} // end getSaveDataType()


// setSaveDataType is a set method to reconfigure the data type
// logged in the history buffer. This method is provided as a
// convenience instead of using the writeConfigurationData method
// since changing the saved data type is considered to be
// more commonly used configuration change. The passed argument
// is an enumeration (see definition in gqgmc.hh) whose value
// is to be the new value of the saveDataType configuration
// parameter. Note that only the host computer's local copy
// of NVM is updated.  The user must issue a update configuration
// command to cause the GQGMC to implement the NVM changes.
void
GQGMC::setSaveDataType(enum saveDataType_t newSaveDataType)
{
  uint8_t  saveData = uint8_t(newSaveDataType);

  // Use writeConfigurationData method
  writeConfigurationData(eSaveDataType, eSaveDataType_bytecnt, &saveData);
  // error condition will be handled by writeConfigurationData()

  return;
} // end setSaveDatatype method


// getDataSaveAddress is get method to retrieve the address
// in the history buffer where the logged data begins. See
// getHistoryData method for an explanation of the
// dataSaveAddress in the configuration data structure.
// The returned value is a 32 bit address, although the
// dataSaveAddress maximum value cannot exceed 24 bits worth.
// Note that the DataSaveAddress is retrieved from the host
// computer's local copy of the GQ GMC's NVM configuration data.
uint32_t
GQGMC::getDataSaveAddress()
{
  uint32_t      address(0);

  address |= (uint32_t(mCFG_Data.dataSaveAddress2) << 16);
  address |= (uint32_t(mCFG_Data.dataSaveAddress1) <<  8);
  address |= (uint32_t(mCFG_Data.dataSaveAddress0) <<  0);

  return address;
}  // end getDataSaveAddress()

// The resetDataSaveAddress sets the dataSaveAddress configuration
// parameter to the value of 0x10.  This is provided as a
// convenience instead of using writeConfigurationData() directly
// because setting the start of the history buffer back to zero
// would be a common thing to do. Note that the DataSaveAddress
// is being updated in the host computer's local copy of the
// GQ GMC's NVM configuration data. The user must issue the
// update configuration command to force the GQ GMC to implement
// the NVM configuration data changes.
void
GQGMC::resetDataSaveAddress()
{
  uint32_t     address(0x10); // 0x10 provides room for date/timestamp

  // Use writeConfigurationData() method
  writeConfigurationData(eDataSaveAddress, eDataSaveAddress_bytecnt,
                         (uint8_t *)(&address));
  // error condition handled by writeConfigurationData()

  return;
} // end resetDataSaveAddress()


// writeConfiguratonData is the public method to write configuration
// data. It takes the following parameters.
//
// cfgParameter is actually the offset from the beginning of the
// configuration data of the desired parameter. It was explicitly
// created this way and assigned the offset as its enumeration value so
// that cfgParameter serves both as an enumeration and as the actual
// address of the configuration parameter.
//
// cfgData is a pointer to an array of raw binary for the parameter.
//
// cfgDataCount is the number of bytes of cfgData. Since cfgData is
// passed as pointer, we need to supply the array length separately
// as the cfgDataCount parameter.
//
// Note that this method updates the local copy of the GQ GMC's
// NVM configuration data. As noted previously in the documentation,
// the GQ GMC does not support a direct method of updating its
// NVM configuration data and having it take effect immediately.
//
// This method is not an exact reflection of the native GQ GMC write
// configuration command. The native GQ GMC write configuration
// only writes one byte at a time. So writing multibyte configuration
// parameters would take multiple writes, one per data byte. Instead,
// we abstract the write configuration command so the user does not
// have to know this much detail. We can do this because we
// know a priori the base address of each parameter and how many
// bytes each parameter needs. So this method is intended to
// handle parameters with multibyte data by requiring the user
// to only pass the parameter enumeration, its byte count, and
// value.
//
// Note that changes to the configuration data do not have
// immediate effect since we writing to the local copy of the
// GQ GMC's NVM configuration data. To take effect, the user must
// call the updateConfigurationData() method. Before doing so, all
// changes to the configuration data should be completed so that
// interdependent configuration parameters are updated
// simultaneously.
void
GQGMC::writeConfigurationData(enum cfg_param_t       cfgParameter,
                              enum cfg_bytecnt_t     cfgDataCount,
                              uint8_t const * const  cfgData)
{
  uint8_t * pCfg_Data = (uint8_t *)&mCFG_Data + uint8_t(cfgParameter);
  for(int i=0; i<cfgDataCount; i++)
  {
    // Convert little endian to big endian which GQ GMC wants.
    if (mBig_endian)
      pCfg_Data[i] = cfgData[i];
    else
      pCfg_Data[i] = cfgData[cfgDataCount-1-i];
  } // end for loop

  return;
} // end writeConfigurationData()

// loadConfigurationData private method writes all 256 bytes
// of the configuration data to the GQ GMC. This will take
// over a minute to complete. This is a practice in patience.
// The GQ GMC's write_cfg_cmd only transmits a single byte
// at a time. So it takes 256 transmits and each transmit
// has to wait for a 0xAA return.  The user obviously should
// not update the NVM configuration too often.
void
GQGMC::loadConfigurationData()
{
  const
  uint32_t     retsize = 1;
  char         ret_char[retsize+1];

  // Need a pointer to the local host computer's copy
  // of the NVM configuration data.
  uint8_t *    pCfg_Data = (uint8_t *)&mCFG_Data;

  // Begin formulating the write configuration data command.
  // "AD" is just a place holder for the address byte and data byte
  // that will be dynamically derived and inserted.
  string write_cfg_cmd = "<WCFGAD>>";

  // The parameter and its data have to be dynamically derived.
  // write_cfg_cmd[5] is parameter enumeration (aka address offset)
  // write_cfg_cmd[6] is parameter data

  // Pack address and data into write_cfg_data_cmd with big
  // endian byte order.

  // Address (ie, parameter) is less than 256 since configuration
  // data has a fixed size of 256 bytes, so address is one byte.

  // pack address and data into command
  for(uint16_t i=0; i<kNVMSize; i++)
  {
    // Increment the address for each additional data byte.
    write_cfg_cmd[5] = uint8_t(i);

    // Load data one byte at a time
    write_cfg_cmd[6] = pCfg_Data[i];

/*    // debug code
    if (i < 64)
    {
      for(int i=0; i<5; i++)
        cout << write_cfg_cmd[i];
      cout << Hex(write_cfg_cmd[5]);
      cout << Hex(write_cfg_cmd[6]);
      cout << write_cfg_cmd[7];
      cout << write_cfg_cmd[8];
      cout << endl;
    }
*/    // end debug code

    // Issue command to write configuration data, one byte
    // at a time because that is the native write configuration
    // command of the GQ GMC.
    communicate(write_cfg_cmd, ret_char, retsize);

    // if read of returned data succeeded, convert raw data to float
    if (mRead_status == true)
    {
      // We really don't care about the return value of 0xAA. If the
      // GQ GMC does not recognize the command, it returns nothing and
      // we get a read_status error.
    }
    else  // else for failure, set error code
    {
      mError_code = eWrite_CFG;
      break;                    // break out of for loop
    } // end (read_status == true)
  } // end for loop

  return;
} // loadConfigurationData()

// eraseConfigurationData public method erases the configuration data
// in its entirety. The configuration returns to its factory default
// setting. It might have been better to call this the reset
// configuration data command. The command is erase_cfg_cmd
// (see GQ GMC COMMANDS above).
void
GQGMC::eraseConfigurationData()
{
  const
  uint32_t     retsize = 1;
  char         ret_char[retsize+1];

  // Issue command to erase NVM configuration.
  communicate(erase_cfg_cmd, ret_char, retsize);

  // If read of returned data succeeded, convert raw data to float,
  if (mRead_status == true)
  {
    // We really don't care about the return value of 0xAA. If the
    // GQ GMC does not recognize the command, it returns nothing and
    // we get a read_status error.
  }
  else  // else for failure, set the error code.
  {
    mError_code = eErase_CFG;
  }

  return;
} // end eraseConfigurationData()


// The updateConfigurationdata public method is called to make changes
// to configuration data take effect. All other methods to modify
// the configuration data do not cause the GQ GMC to immediately
// change operation. This would not be desireable since various
// changes to the configuration may be interdependent and so
// we would want the changes to take effect simultaneously to
// insure proper operation. There are no arguments to call.
// The command is update_cfg_cmd (see GQ GMC COMMANDS above).
// The user who calls this method may want to pop-up a window
// stating that this will take about one minute. That is about
// how long it will take to write all 256 bytes to the GQ GMC.
// This method calls eraseConfigurationData() and
// loadConfigurationData() as part of the procedure needed to
// force the GQ GMC to implement operational changes per the
// new NVM configuration data.
void
GQGMC::updateConfigurationData()
{
  const
  uint32_t     retsize = 1;
  char         ret_char[retsize+1];

    // 1st, we have to erase configuration data
  // cout << erase_cfg_cmd << endl; // debug
  eraseConfigurationData();
  // 2nd, write all 256 bytes of NVM to GQ GMC
  // cout << "load cfg" << endl; // debug
  loadConfigurationData();

  // cout << update_cfg_cmd << endl; // debug
  // Issue command to update NVM and force GQ GMC to change
  // operation in accordance to new configuration data.
  communicate(update_cfg_cmd, ret_char, retsize);

  // If read of returned data succeeded, convert raw data to float,
  if (mRead_status == true)
  {
    // We really don't care about the return value of 0xAA. If the
    // GQ GMC does not recognize the command, it returns nothing and
    // we get a read_status error.
  }
  else  // else for failure, set the error code.
  {
    mError_code = eUpdate_CFG;
  }

  return;
} // end updateConfigurationData()


// sendKey is the public method to emulate any one of the 4 keys on
// the front panel of the GQ GMC. The front panel has a 'left arrow',
// 'up arrow, 'down arrow', and 'enter' keys. These are used to
// navigate through the GQ GMC's menu system. In principle, the
// menu system can be used to set virtually any and all of the
// configuration data (although this is not recommended for
// such configuration data as the calibration values).
// So instead of the writeConfigurationData method,
// the proper sequence of sending the keys would do the
// same thing. The command is derived dynamically, but the actual
// command is "<KEY0>>" or "<KEY1>>" or "<KEY2>>" or "<KEY3>>".
// So for the purpose that the user need not know the actual
// command string, the softkey_t enumeration is created and used
// as the passed argument to the method. See the enum softkey_t
// declaration in gqgmc.hh for more discussion.
//
// For successive sendKey calls there is a trick to know when
// using the sendKey method. The trick is you can't
// transmit sendKey too fast and you can't do it too slow.
// Another thing is that the Save Data option menu starts with
// the current data type and then cycles through the other options.
// So you have to know what is the current data type and
// then send the Enter key the proper number of times to cycle
// to the desired option. When moving through the menu we use
// 0.5 seconds, but when moving through a pop up options menu
// we have to move faster and so should use 0.25 seconds
// between sendKey in that context.
void
GQGMC::sendKey(enum softkey_t key)
{
  char   inp[1]; // This will not be used, just needed as dummy arg

  // Begin formulating the send key command.
  string keycmd = "<KEY";

  // Append key number which is an enumeration equal to the
  // ASCII value of the key, ie, '0', '1', '2', or '3'.
  keycmd += uint8_t(key);
  // Append ">>"
  keycmd += ">>";

  communicate(keycmd, inp, 0);  // no return data
  // Since the sendkey command returns no data there is no way to
  // test success of communication.

  // Debug code
/*
  for(int i=0; i<7; i++)
    cout << Hex(keycmd[i]);
  cout << endl;
*/
  return;
} // end sendKey()


// setDate is the public method to set the date. The date is passed as
// an ASCII string with the format of <month><day><year>, for example,
// 112312 is November (11th month) 12, 2012.  The year is specified
// as the last two digits of the century since presumably the date is
// is being set to the current date. In reality, the GQ GMC has a separate
// command for setting each of the month, day, and year.
void
GQGMC::setDate(string date)
{
  const
  uint32_t     retsize = 1;
  char         ret_char[retsize+1];

  // The date is broken up into three separate commands one each for
  // month, day, and year as supported by the GQ GMC.

  // Set the month, <SETDATEMMXX>> where XX = month byte.
  {
    // uint16_t is necessary since ss >> uint8_t does not work.
    uint16_t      month=0;
    string        setMonthCmd;
    stringstream  ss;
    ss << date[0] << date[1];
    ss >> month;
    //cout << "month = " << Hex(uint8_t(month)) << endl;
    setMonthCmd  = "<SETDATEMM";
    setMonthCmd += uint8_t(month);
    setMonthCmd += ">>";
    communicate(setMonthCmd, ret_char, retsize);
  }

  // Set the day, <SETDATEDDXX>> where XX = day byte.
  {
    uint16_t      day=0;
    string        setDayCmd;
    stringstream  ss;
    ss << date[2] << date[3];
    ss >> day;
    //cout << "day = " << Hex(uint8_t(day)) << endl;
    setDayCmd  = "<SETDATEDD";
    setDayCmd += uint8_t(day);
    setDayCmd += ">>";
    communicate(setDayCmd, ret_char, retsize);
  }

  // Set the year, <SETDATEYYXX>> where XX = year byte.
  {
    uint16_t      year=0;
    string        setYearCmd;
    stringstream  ss;
    ss << date[4] << date[5];
    ss >> year;
    //cout << "year = " << Hex(uint8_t(year)) << endl;
    setYearCmd  = "<SETDATEYY";
    setYearCmd += uint8_t(year);
    setYearCmd += ">>";
    communicate(setYearCmd, ret_char, retsize);
  }

  return;
} // end set Date()

// setTime is the public method to set the time of day. The time is
// passed as an ASCII string with the format of <hour><minutes><seconds>,
// for example, 142256 is the 14th hour, 22 minutes after the hour,
// 56 seconds after the minute.  The hour is given in 24 hour format
// counting from 0 to 23. In reality, the GQ GMC provides a separate
// command for setting each of the hour, minutes and seconds.
void
GQGMC::setTime(string time)
{
    const
    uint32_t     retsize = 1;
    char         ret_char[retsize+1];

    // The time is broken up into three separate commands one each for
    // hour, minute, and second as supported by the GQ GMC.

    // Set the hour, <SETTIMEHHXX>> where XX = hour byte.
    {
      uint16_t      hour=0;       // stringstream does not convert to uint8_t
      string        setHourCmd;
      stringstream  ss;
      ss << time[0] << time[1];
      ss >> hour;
      //cout << "hours = " << Hex(uint8_t(hour)) << endl;
      setHourCmd  = "<SETTIMEHH";
      setHourCmd += uint8_t(hour);
      setHourCmd += ">>";
      communicate(setHourCmd, ret_char, retsize);
    }

    // Set the minute, <SETTIMEMMXX>> where XX = minute byte.
    {
      uint16_t      minute=0;
      string        setMinuteCmd;
      stringstream  ss;
      ss << time[2] << time[3];
      ss >> minute;
      //cout << "minute = " << Hex(uint8_t(minute)) << endl;
      setMinuteCmd  = "<SETTIMEMM";
      setMinuteCmd += uint8_t(minute);
      setMinuteCmd += ">>";
      communicate(setMinuteCmd, ret_char, retsize);
    }

    // Set the seconds, <SETTIMESSXX>> where XX = second byte.
    {
      uint16_t      second=0;
      string        setSecondCmd;
      stringstream  ss;
      ss << time[4] << time[5];
      ss >> second;
      //cout << "second = " << Hex(uint8_t(second)) << endl;
      setSecondCmd  = "<SETTIMESS";
      setSecondCmd += uint8_t(second);
      setSecondCmd += ">>";
      communicate(setSecondCmd, ret_char, retsize);
    }

  return;
} // end setTime()


// PRIVATE METHODS

// communicate private method is used to write/read data to/from
// the GMC-300. This method is expressedly designed to be called
// by methods which send an ASCII string and expect to receive
// returned data. However for flexibility, if the command string
// is null, no command is transmitted and if the expected number
// of return bytes is zero, no read is performed.
// cmd is the ASCII string command.
// retdata is the repository for the returned data.
// retbytes is the number of bytes of returned data.
void
GQGMC::communicate(const string cmd, char * retdata, uint32_t retbytes)
{
  // Clear the USB port of any left over data from last exchange. Even
  // though we know how many bytes the GQ GMC transmits for each
  // command, experience has shown this is the safe thing to do since
  // there is no protocol for the returned data.
  clearUSB();

  //cout << cmd << endl;
  // 1st, issue the command to the GMC-300, this is always an ASCII
  // string.
  // For flexibility, only transmit if cmdbytes is not 'null'.
  if (cmd.size() > 0) sendCmd(cmd);
  // 2nd, read the return data, for all commands except get version
  // this is always raw binary data.
  // For flexibility, only read if return is not 'null'.
  if (retbytes > 0) readCmdReturn(retdata, retbytes);

  return;
} // end communicate()

// sendCmd is the private method (the basic method) to transmit
// the command to the GMC-300.
// cmd is the ASCII string to send as the command.
void
GQGMC::sendCmd(const string cmd)
{
  // This is a common place to reset the error code since it is always
  // called for any command.
  mError_code = eNoProblem;

  // This is a common place to reset the read status since every read
  // is always preceeded by a write command (except for turn_on_cps!).
  mRead_status = true;

  // Call low level C stdio routine to write to USB port.
  write(mUSB_serial, cmd.c_str(), cmd.size());

  return;
} // end sendCmd()

// readCmdReturn is the private method (the basic method) to read
// the return bytes from the command.
// retdata is the repository for the returned data.
// retbytes is the number of bytes of the returned data.
void
GQGMC::readCmdReturn(char * retdata, uint32_t retbytes)
{
  uint32_t rcvd = 0;           // the number of received bytes
  char * inp    = &retdata[0]; // pointer to returned data char array
                               // start pointer off at beginning of
                               // repository.

  // Assume read will succeed, replicated here only because of the
  // nature of the turn_on_cps command which automatically returns
  // data without a preceeding write.
  mRead_status = true;

  // Now read the returned raw byte string. Do this by reading one byte
  // at a time until the requested number of bytes are attained. However,
  // the serial port has been setup to timeout each read attempt. So if
  // after N calls to read, we haven't yet read all N bytes, declare
  // a failure. The read is done this way to avoid an indefinite blocking
  // situation when 0 bytes are returned by the GQ GMC. The only good thing
  // about this methodology is that the largest possible read is only 4K
  // for the history data. So the read never really takes that much time.
  for(uint32_t i=0; i<retbytes; i++)
  {
    rcvd += read(mUSB_serial, inp, 1);
    inp   = &retdata[rcvd];
    if (rcvd >= retbytes) break;
  } // end for loop

  //  debugging code
  /*
  inp = &retdata[0];
  for(uint32_t i=0; i<retbytes; i++)
  {
    cout << Hex(inp[i]) << "-";
    if (i > 0)
      if (((i+1)%16) == 0) cout << endl;
    if (i > 62)break;
  }
  cout << endl;
  cout << "rcvd = " << rcvd << endl;
  */
  // end debug code

  // Communication is considered a failure if less than the expected
  // number of bytes is returned by the GMC-300.
  if (rcvd < retbytes)
    mRead_status = false;

  return;
} // readCmdReturn()

// isBigEndian is the method to determine endianess of host CPU.
// This is to be called by constructor once and once only. This is
// needed because the GQ GMC wants data transmitted to it in
// big endian and returns data in big endian, but the host computer
// could be little endian. The routine was adapted from code
// found on the internet.
bool
GQGMC::isBigEndian(void)
{
  uint32_t x(0xff);
  bool bigend(reinterpret_cast<unsigned char *>(&x)[0] == 0);
  return bigend;
} // end isBigEndian()

// end file gqgmc.cc
