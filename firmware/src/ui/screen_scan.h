#pragma once
#include "reader.h"

// The three screens of the everyday loop: present the spool, confirm what was
// read, see what the printer actually stored.
namespace screen_scan {

// No status dots: reaching this screen already proves the printer answered and
// the tap came off its slot grid, so a pair of green dots would only repeat
// what the user just did.
// It also stays up while the spool is being written to the printer: the write
// gets no screen, not even a word - it is over before one could be read.
// `caught` says the chip has been read and the write is under way; it takes
// the Cancel button away, because there is nothing left to cancel.
void showScan(const char* slotLabel, const char* errorOrNull, bool caught = false);

// The success screen confirms what was SENT and names the next step: put the
// spool in that slot. It deliberately says nothing about what the slot held
// before, and nothing about the colour the printer settled on - the question
// somebody has at this moment is whether their spool went through, and where
// to go now.
//
// `msLeft` is what is left of the five seconds before it closes itself, drawn
// as a draining bar. A tap anywhere closes it at once. `message` is only read
// on failure.
void showResult(const char* slotLabel, bool ok, const char* message,
                const TagInfo& tag, uint32_t msLeft);

void invalidate();
bool takeCancel();
bool takeDismiss();

}  // namespace screen_scan
