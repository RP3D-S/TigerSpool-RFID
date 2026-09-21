#pragma once
#include <Arduino.h>

// The battery, on the boards that have one.
//
// The Waveshare ESP32-S3-Touch-LCD-2 carries a charger and a battery connector
// that most units are shipped without: a user solders or plugs a cell in
// afterwards. So nothing here is allowed to assume a battery exists, and the
// device must read exactly the same with or without one.
//
// The cell sits behind a divider on an ADC pin, and the pin is the only thing
// that answers the question. There is no charge-status line documented on this
// board, so "charging" is not something this module claims to know.
namespace battery {

// Starts sampling. Safe on a board with no cell: the pin is read the same way
// and simply comes back near zero.
void begin();

// Call from the main loop. Cheap: it samples on its own schedule.
void loop();

// Is there a cell on the connector? The USER says so - see declare().
//
// It is not detected any more, because it cannot be. The board has no sense
// line for it, and both indirect signals were measured wrong on real hardware:
// the rail with an empty connector sits anywhere from 4.05 to 4.27 V depending
// on the board, and the charger's ripple - which a healthy cell damps - is not
// damped by a tired one. One of our two boards reported a battery it did not
// have; the other denied the pack it was running on. A guess that confident is
// worse than a question.
bool present();

// What the user declared, kept in NVS. `present()` is this, and nothing else.
bool declared();
void declare(bool yes);

// The last reading, in volts, and the raw ADC millivolts behind it. The raw
// value is what a bench measurement is compared against; the volts are what a
// screen shows.
float volts();
int   millivolts();

// True while the rail is being held up or pushed up by the charger.
//
// The board has no charge-status line, so this is read from the voltage over
// time: a cell being charged climbs or sits at the charger's own level, a cell
// running the device falls. It takes half a minute of readings to change its
// mind, which is the price of not having the pin the hardware does not have.
bool charging();

// A percentage from the voltage, 0-100, or -1 while there is nothing to read.
// It is a curve fit for a single lithium cell at rest, not a fuel gauge: it
// reads low under load and high just off the charger, and it is the best a
// board with no coulomb counter can offer.
int percent();

// How long is left, in minutes: to empty on the cell, to full on the charger.
// -1 until the level has moved enough to measure a slope, which takes a few
// minutes from a cold start - an estimate made from one reading would be a
// number invented to fill a row.
//
// It is the observed rate, not a datasheet figure, so it answers for what the
// device is doing now: a lit screen empties the cell faster than a sleeping
// one, and the estimate follows within a few minutes of the change.
int minutesLeft();

}  // namespace battery
