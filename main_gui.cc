// **************************************************************************
// File: main_gui.cc
//
// Author:    Phil Gillaspy
//
// Description: GUI demonstration program for GQ GMC
//              (geiger-muller counter).
//
// **************************************************************************
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <QApplication>
#include <QtGui>
#include <QPixmap>
#include <QMessageBox>
#include <QDateTime>

#include "plotter.hh"
#include "gqgmc.hh"
using namespace GQLLC;

static Plotter * pPlotter;

// Instantiate the GQGMC object on the heap
static GQGMC * gqgmc;


int numPoints = 120;
int kHalfway  = numPoints/2;
QVector<QPointF> points0;

void getCPM(int i)
{
  // When the plotter reaches the end, shift top half CPM data
  // back to the beginning, then start new data at halfway point.
  // So last hour is always displayed and new hour starts
  // at midpoint of graph.
  if (pPlotter->bScroll)
  {
    for(int j=0;j<kHalfway;j++)
      points0[j].setY(points0[kHalfway+j].y());

    points0.resize(kHalfway);
    i = kHalfway;
  }
  uint16_t cpm;
  cpm = gqgmc->getCPM();
//  cpm = uint(qrand()) % 100; // fake it for debug
  points0.append(QPointF(i, cpm)); // x==i, y==cpm
  pPlotter->setCurveData(0, points0);

  return;
}

// Utility to show message to user.
void Display_message(string msg)
{
  int msgok = QMessageBox::warning(pPlotter, "NOTICE", msg.c_str());
  if (msgok);
  return;
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
main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  Plotter plotter;
  pPlotter = &plotter;

  // open the USB port using a USB to serial converter device driver
  // Using UDEV rule file 51-gqgmc.rules to create symlink to /dev/gqgmc.
  string  usb_device = "/dev/gqgmc";

  if (argc > 1)
    usb_device = argv[1];
  else
  {
    stringstream msg;
    msg << "Usage: gqgmc <usb-port-device-name>" << endl;
    msg << "Example: gqgmc /dev/ttyUSB0" << endl;
    Display_message(msg.str());
    return 0;
  }

  // Instantiate the GQGMC object on the heap
  gqgmc = new GQGMC;

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
    return 0;
  }

  PlotSettings settings;
  settings.minX = 0;
  settings.maxX = 120;
  settings.minY = 0.0;
  settings.maxY = 100.0;

  plotter.setPlotSettings(settings);
  plotter.setWindowTitle("GQ GMC Data Logger");
  plotter.show();

  plotter.startSample();

  return app.exec();
}
