#include "battery.h"
#include "config.h"
#include <Preferences.h>

namespace {

// Every number here was measured on the bench, because the board's schematic
// is the only documentation and it does not say what the divider is.
const uint32_t SAMPLE_MS  = 1000;   // often enough for a screen, idle otherwise
const int      READS      = 8;      // averaged: a single ADC read wanders
// Whether there is a cell at all is NOT measured. The owner says so.
//
// Two indirect signals were tried on real hardware and both lied.
//
// The LEVEL cannot answer it: the pin reads the battery rail, and on USB that
// rail is held up by the charger whether a cell is plugged in or not - at
// 4.27 V on one of our boards and 4.05 V on another with its cell unplugged.
// Any threshold between those fails on one board or the other, and 4.05 V is
// also exactly where a working cell sits.
//
// The RIPPLE cannot either, though it looked convincing for a while: the
// charger is a switcher, its output ripples, and a healthy cell is an enormous
// capacitor across that output which swallows the ripple. But a tired cell does
// not. Measured on ONE board, in both directions: a nearly flat pack at 3.31 V
// rippled 6.0 mV and was read as "no cell", and the same board with the pack
// REMOVED sat at 4.05 V rippling 1.7 mV and was read as "a cell". The other
// board, empty, sits at 4.27 V rippling 9 mV. No threshold separates those.
//
// Nor is there a pin to ask: the charger's STAT output drives a red LED and
// goes nowhere near the processor (ETA6098, pin 9, in the board's schematic),
// and the battery connector is two wires, VBAT and ground.
//
// So the question is put to the user, once, in Settings - see battery.h - and
// everything below follows that answer. What IS still measured, and is honest,
// is the voltage on the rail and the shape of its curve over time.

// Charging is read from the SHAPE of the curve, not from its level, because
// the level says nothing: the same cell read 3.94 V on its own and 4.04 V a
// second after the cable went in. Measured on the bench: on the cell the
// voltage falls about 15 mV a minute with the screen lit, and on the charger
// it is flat to within a millivolt or two. So: falling means the cell is
// carrying the device, anything else means something is holding it up.
//
// Two mechanisms, because the two events have different shapes.
//
// A cable going in or out is a STEP, and it is compared against the PREVIOUS
// reading - one second - not against a window.
//
// Recorded on Benoit's board over a dozen plug-and-unplug cycles: plugged in
// reads 1298-1303 mV at the pin, unplugged 1256-1261, so the step is about 40
// and the drift between two readings is one. A six-second window looked safer
// and was wrong: plugging and unplugging inside it compared two readings at
// the SAME level, saw no step, and left the state as it was - so the level
// kept the charger's 0.13 V taken off it after the cable was already out, and
// the screen showed 23% for a cell reading 44%.
// Five at the pin, fifteen at the cell. It was eight, which is comfortably
// clear of the millivolt two readings wander by - but the step shrinks with
// the charge current, and Benoit caught it at 0.06 V at the cell, twenty at
// the pin, late in a charge. Lower still would start catching noise; when the
// step falls under this the three-minute trend is what notices the cable.
const int      STEP_MV    = 5;
// The drift is the other shape: a few millivolts a minute either way, which
// needs a window long enough to rise above the noise and short enough to
// answer while somebody is still looking. Three minutes, measured on this
// board: eight averaged reads wander by a millivolt or two, a cell charging at
// 3.8 V climbs about 2.4 mV a minute at the pin and one running the device
// falls about 5. A minute was not enough for the charge - the row still said
// "On battery" with the cable in. This window is also what settles the state
// after a boot, where there is no step to catch because the cable was already
// there.
const uint32_t SLOW_MS    = 180000;
// Asymmetric, and that asymmetry is the point. A cell running the device falls
// about 5 mV a minute at the pin, so fifteen over this window; a cell being
// charged climbs two or three a minute early on and FLATTENS to nothing as the
// charger tapers. Judging both at 3 mV called a nearly full cell on a charger
// "on battery", because flat and falling look alike at that size. Rising by 3
// still means charging; only a fall of 6 - which no charger produces and every
// discharge exceeds - is allowed to say the cable is out.
const int      SLOW_MV_UP   = 3;
const int      SLOW_MV_DOWN = 6;
// Where the rail sits when a charger is holding it. Used only for the first
// reading, when there is no history to read yet.
const float    CHARGER_V  = 4.00f;
// What the charger adds to the cell's own voltage while it is charging - and
// it is NOT a constant. It is the charge current through the cell's internal
// resistance, so it shrinks as the charger tapers towards a full cell: fixed
// at 0.13 V it over-corrected late in a charge and the level read 64% with the
// cable in against 75% with it out, for the same cell a second apart.
//
// So the device measures it, on the only occasion it can be measured: the
// moment a cable goes in or out, where the step in the reading IS the offset.
//
// Until it has seen one - a fresh device, or the first minutes after a boot -
// it uses a prior that follows the same physics: the offset is the charge
// current through the cell's resistance, and that current is large on an empty
// cell and near nothing on a full one. So it is scaled between the two ends of
// the cell's range. A flat 43 mV was the first attempt and it was wrong in the
// case that matters - a nearly full cell on a charger read 60% against the 74%
// it reads a second after the cable moves.
const float    OFFSET_AT_EMPTY_MV = 43.0f;   // 0.13 V at the cell, measured
const float    OFFSET_AT_FULL_MV  = 3.0f;    // the charger has all but stopped
const float    OFFSET_EMPTY_V     = 3.50f;
const float    OFFSET_FULL_V      = 4.20f;
// And the bounds it is kept inside: a step outside them is not a cable.
const float    OFFSET_MIN_MV = 5.0f;
const float    OFFSET_MAX_MV = 120.0f;
// The slope is measured on the VOLTAGE, not on the level, and that is what
// makes an estimate possible in a minute instead of six.
//
// A percent is a coarse unit: charging, this cell gains one point every 70
// seconds, so a window short enough to be useful measures rounding. The pin
// reads in millivolts and moves 2-3 of them a minute, well above the one
// millivolt two readings wander by - so the voltage gives a usable slope in a
// minute, and the curve turns it into percent.
const uint32_t SLOPE_MS   = 120000;
// Under this the cell is resting, not moving, and no estimate is made. It is
// low on purpose: towards the end of a charge the charger tapers and the cell
// climbs half a millivolt a minute at the pin, which is still a real charge
// and still worth an estimate - at 0.4 the line simply disappeared for the
// last third of the charge, which is the third somebody is watching.
const float    SLOPE_MIN_MV = 0.15f;
// And the estimate is capped. Below the threshold the arithmetic divides by
// almost nothing and answers in days; a cap says "a long time" without
// pretending to know how long.
const int      MAX_MINUTES  = 600;

// The estimate has to answer IMMEDIATELY, and the slope cannot: it needs two
// minutes of readings before it means anything. So the first answer comes from
// a nominal model - the cell's rated capacity against the current that goes in
// or out - and the measurement takes over as soon as it has one, which is what
// makes the number settle over the first few minutes instead of appearing out
// of nowhere. A person plugging a cable in wants to know roughly how long,
// now; "no answer for two minutes" reads as broken.
//
// The cell is the 1000 mAh LiPo on the bench units. The two currents are of
// the right order for this board rather than measured to the milliamp: a
// charger that gives about half an amp, a device that takes about two hundred
// milliamps with its screen lit. They set the first answer only.
const int      CAPACITY_MAH = 1000;
const int      CHARGE_MA    = 300;   // what reaches the cell while it charges
const int      DRAIN_MA     = 200;   // what the device takes off it

static int nominalMinutes(int pct, bool charging) {
    if (pct < 0) return -1;
    const int mah = charging ? (100 - pct) * CAPACITY_MAH / 100
                             : pct * CAPACITY_MAH / 100;
    const int ma  = charging ? CHARGE_MA : DRAIN_MA;
    const int m   = mah * 60 / ma;
    return m > MAX_MINUTES ? MAX_MINUTES : m;
}

int   s_mv      = 0;
float s_v       = 0.0f;
bool  s_charging = false;
int   s_prevMv  = 0;        // the reading before this one, for the step
float s_offsetMv = 0.0f;    // the charger's share, once measured
bool  s_offsetKnown = false;
// ...and kept across reboots. It is learned at the moment a cable moves, which
// may be hours apart, and a device that forgets it starts every boot applying
// a default that is wrong for a nearly full cell - which shows up as the level
// reading fifteen points low until somebody happens to unplug something.
float s_offsetSaved = 0.0f;

// The stillness test, and its verdict.

bool  s_declared = false;

void saveDeclared() {
    Preferences k;
    if (!k.begin("tigerspool", false)) return;
    k.putUChar("bdecl", s_declared ? 1 : 0);
    k.end();
}


// The prior, for a device that has not seen a cable move yet.
float priorOffsetMv(float v) {
    if (v <= OFFSET_EMPTY_V) return OFFSET_AT_EMPTY_MV;
    if (v >= OFFSET_FULL_V)  return OFFSET_AT_FULL_MV;
    const float t = (v - OFFSET_EMPTY_V) / (OFFSET_FULL_V - OFFSET_EMPTY_V);
    return OFFSET_AT_EMPTY_MV + (OFFSET_AT_FULL_MV - OFFSET_AT_EMPTY_MV) * t;
}

// What to take off the reading right now: what was measured if anything was,
// and the prior until then.
float offsetNowMv(float v) {
    return s_offsetKnown ? s_offsetMv : priorOffsetMv(v);
}

void saveOffset() {
    // Only when it has really moved: NVS has a finite number of writes, and
    // this value changes by a millivolt all day long.
    if (!s_offsetKnown) return;
    if (s_offsetMv > s_offsetSaved - 2.0f && s_offsetMv < s_offsetSaved + 2.0f) return;
    Preferences k;
    if (!k.begin("tigerspool", false)) return;
    k.putUShort("boffmv", (uint16_t)(s_offsetMv + 0.5f));
    k.end();
    s_offsetSaved = s_offsetMv;
}

// A piecewise fit of the discharge curve of a single lithium-polymer cell at
// rest. Linear from voltage to percent is wrong at both ends - the flat middle
// of the curve would read as half the pack gone in ten minutes - so the curve
// is walked in segments, by percent() and by the slope that feeds the estimate.
struct Point { float v; int pct; };
const Point CURVE[] = {
    { 3.30f,   0 }, { 3.60f,  15 }, { 3.75f,  40 },
    { 3.95f,  70 }, { 4.10f,  90 }, { 4.20f, 100 },
};
int   s_slowMv  = 0;
uint32_t s_slowAt = 0;
int   s_slopeMv  = -1;      // the reading the slope is measured from
uint32_t s_slopeAt = 0;
float s_slope    = 0.0f;    // mV per minute at the pin, signed, smoothed
// Whether s_slope has been measured at all. It cannot be told from the value:
// a window over a cell rising a millivolt a minute often comes back as exactly
// zero, and treating that as "nothing measured yet" left the estimate at "no
// answer" indefinitely - which is what Benoit saw with the cable in.
bool  s_haveSlope = false;
bool  s_slopeUp  = false;   // whether it was measured while charging
uint32_t s_next = 0;

void sample() {
    uint32_t sum = 0;
    int lo = 100000, hi = 0;
    for (int i = 0; i < READS; i++) {
        const int r = (int)analogReadMilliVolts(BATTERY_ADC_PIN);
        sum += (uint32_t)r;
        if (r < lo) lo = r;
        if (r > hi) hi = r;
    }
    s_mv = (int)(sum / READS);
    s_v  = (float)s_mv * BATTERY_DIVIDER / 1000.0f;

    // The step, against the reading before this one. Decided here, before the
    // level is asked for, so the state and the voltage it is applied to are
    // always the same reading.
    if (s_prevMv) {
        const int step = s_mv - s_prevMv;
        if (step >= STEP_MV)       s_charging = true;     // a cable went in
        else if (step <= -STEP_MV) s_charging = false;    // and out
        // The size of that step is the charger's contribution, right now, at
        // this state of charge. Half of the old value so one noisy sample
        // cannot throw it, and inside bounds so a brown-out or a reconnecting
        // radio is not mistaken for a cable.
        const float mag = (float)(step < 0 ? -step : step);
        if (mag >= STEP_MV && mag >= OFFSET_MIN_MV && mag <= OFFSET_MAX_MV) {
            s_offsetMv = s_offsetKnown ? (s_offsetMv * 0.5f + mag * 0.5f) : mag;
            s_offsetKnown = true;
            saveOffset();
        }
    }
    s_prevMv = s_mv;

    const uint32_t now = millis();
    if (now - s_slowAt >= SLOW_MS) {
        const int delta = s_mv - s_slowMv;
        if (delta <= -SLOW_MV_DOWN)   s_charging = false;  // living off the cell
        else if (delta >= SLOW_MV_UP) s_charging = true;
        s_slowMv = s_mv;
        s_slowAt = now;
    }
}

}  // namespace

// The cell's own voltage: what is read, less the charger's share while it is
// charging. Everything that means "how full is it" goes through here.
static float restVolts() {
    if (!s_charging) return s_v;
    const float off = offsetNowMv(s_v);
    return (float)(s_mv - (int)(off + 0.5f)) * BATTERY_DIVIDER / 1000.0f;
}


void battery::begin() {
    analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);   // the full 0-3.1 V span
    {
        Preferences k;
        if (k.begin("tigerspool", true)) {
            const uint16_t v  = k.getUShort("boffmv", 0);
            // What the user said about the battery, last time they said it.
            s_declared = k.getUChar("bdecl", 0) == 1;
            k.end();
            if (v >= (uint16_t)OFFSET_MIN_MV && v <= (uint16_t)OFFSET_MAX_MV) {
                s_offsetMv = s_offsetSaved = (float)v;
                s_offsetKnown = true;
            }
        }
    }
    sample();
    s_prevMv = s_mv;
    s_slowMv = s_mv;
    s_slowAt = millis();
    // Nothing to compare against yet, so the level answers for the first half
    // minute: a board that boots on USB is charging, one that boots on its
    // cell is not.
    s_charging = s_v >= CHARGER_V;
    s_next = millis() + SAMPLE_MS;
    // No verdict on this line: presence takes a second reading to confirm, and
    // the first one always said "no battery" on a board that has one.
    Serial.printf("[batt] first read: %d mV at the pin, %.2f V at the cell, "
                  "charger offset %d mV%s\n",
                  s_mv, s_v, (int)(offsetNowMv(s_v) + 0.5f),
                  s_offsetKnown ? "" : " (not measured yet)");
}

void battery::loop() {
    if ((int32_t)(millis() - s_next) < 0) return;
    s_next = millis() + SAMPLE_MS;
    sample();

    // The slope, on the voltage at the pin.
    const uint32_t now = millis();
    if (!s_declared) { s_slopeMv = -1; s_slope = 0.0f; s_haveSlope = false; return; }
    if (s_slopeMv < 0 || s_slopeUp != s_charging) {
        // A fresh start, and after every change of direction: the rate at
        // which a cell fills says nothing about how long it will then last,
        // and the 40 mV step of a cable is not a slope at all.
        s_slopeMv  = s_mv;
        s_slopeAt  = now;
        s_slopeUp  = s_charging;
        s_slope    = 0.0f;
        s_haveSlope = false;
        return;
    }
    if (now - s_slopeAt < SLOPE_MS) return;
    const float perMin = (float)(s_mv - s_slopeMv) * 60000.0f / (float)(now - s_slopeAt);
    // Smoothed hard, because the measurement is coarse: a minute of charging
    // moves the pin by two or three millivolts, so one window in five reads a
    // millivolt high or low and the remaining time swung from 87 minutes to
    // 145 between two of them. Four fifths of the old value keeps the estimate
    // steady; the new reading still moves it within a few minutes of a real
    // change.
    s_slope    = s_haveSlope ? (s_slope * 0.8f + perMin * 0.2f) : perMin;
    s_haveSlope = true;
    s_slopeMv  = s_mv;
    s_slopeAt  = now;
}

// How many percent the level moves for one volt at the cell, here, on this
// part of the curve. The curve is not a line - the flat middle of a lithium
// cell covers thirty points in two hundred millivolts, the ends far fewer per
// millivolt - so a slope in millivolts only becomes a slope in percent once it
// is read against the segment the cell is actually on.
static float pctPerVolt(float v) {
    for (int i = 1; i < (int)(sizeof(CURVE) / sizeof(CURVE[0])); i++) {
        if (v <= CURVE[i].v || i == (int)(sizeof(CURVE) / sizeof(CURVE[0])) - 1)
            return (float)(CURVE[i].pct - CURVE[i-1].pct) / (CURVE[i].v - CURVE[i-1].v);
    }
    return 0.0f;
}

int battery::minutesLeft() {
    if (!s_declared) return -1;
    const int pct = battery::percent();
    if (pct < 0) return -1;
    if (pct >= 100 && s_charging) return 0;
    // Nothing measured yet - the first two minutes, and any moment the cell is
    // barely moving - so the model answers.
    if (!s_haveSlope) return nominalMinutes(pct, s_charging);
    const float v = restVolts();
    // mV a minute at the pin -> percent a minute at the cell.
    const float pctPerMin = s_slope * BATTERY_DIVIDER / 1000.0f * pctPerVolt(v);
    if (s_charging) {
        if (s_slope < SLOPE_MIN_MV || pctPerMin <= 0.0f)
            return nominalMinutes(pct, true);
        const int m = (int)((100 - pct) / pctPerMin + 0.5f);
        return m > MAX_MINUTES ? MAX_MINUTES : m;
    }
    if (-s_slope < SLOPE_MIN_MV || pctPerMin >= 0.0f)
        return nominalMinutes(pct, false);
    const int m = (int)(pct / -pctPerMin + 0.5f);
    return m > MAX_MINUTES ? MAX_MINUTES : m;
}

// The user's word, and nothing else - see battery.h for why there is no
// detection left to consult.
bool  battery::present()    { return s_declared; }
bool  battery::declared()   { return s_declared; }
void  battery::declare(bool yes) {
    if (yes == s_declared) return;
    s_declared = yes;
    saveDeclared();
    Serial.printf("[batt] the user says there is %s\n",
                  yes ? "a battery" : "no battery");
}
bool  battery::charging()   { return s_charging; }
float battery::volts()      { return s_v; }
int   battery::millivolts() { return s_mv; }

int battery::percent() {
    if (!s_declared) return -1;
    const float v = restVolts();
    const int n = sizeof(CURVE) / sizeof(CURVE[0]);
    if (v <= CURVE[0].v)     return 0;
    if (v >= CURVE[n-1].v)   return 100;
    for (int i = 1; i < n; i++) {
        if (v <= CURVE[i].v) {
            const float span = CURVE[i].v - CURVE[i-1].v;
            const float in   = v - CURVE[i-1].v;
            return CURVE[i-1].pct +
                   (int)((CURVE[i].pct - CURVE[i-1].pct) * in / span + 0.5f);
        }
    }
    return 100;
}
