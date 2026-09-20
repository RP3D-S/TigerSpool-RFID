#include "screen_home.h"
#include "fonts.h"
#include "icons.h"
#include "theme.h"
#include "lvgl_port.h"
#include "frame.h"
#include "../i18n.h"
#include "i18n.h"
#include <lvgl.h>
#include <Arduino.h>

namespace {

lv_obj_t* s_screen   = nullptr;
lv_obj_t* s_list     = nullptr;
lv_obj_t* s_account  = nullptr;
// The header is built once, with the screen, and only the list below it is
// rebuilt - so the title has to be told when the language changes, or a
// device switched to Chinese keeps the title of the language it booted in.
lv_obj_t* s_title    = nullptr;
lv_obj_t* s_wifi     = nullptr;
// The chevron back to the first screen. It belongs to the printer list only,
// so it is built once with the header and hidden while the choice is showing -
// a screen with nowhere to go back to must not offer the way.
lv_obj_t* s_back     = nullptr;
lv_obj_t* s_gear     = nullptr;
lv_obj_t* s_gearIcon = nullptr;

// Which of the two faces the list currently holds: 1 the choice, 2 the printer
// list. Both faces draw into the same container and each keeps its own redraw
// signature, so without this a face whose signature had not changed since the
// last time it was shown would decline to rebuild - and the other face's rows
// would stay on screen.
int       s_face     = 0;

// The level lives in icons::wifiLevelFromRssi. The portal's picker uses the
// same thresholds; see `bars()` in net/portal_page.h.
bool      s_active   = false;
volatile int s_tapped   = -1;
volatile bool s_settings = false;
volatile bool      s_pick     = false;
volatile bool      s_goPrinters = false;
volatile bool      s_goReader   = false;
volatile bool      s_homeBack   = false;

void onRow(lv_event_t* e)      { s_tapped   = (int)(intptr_t)lv_event_get_user_data(e); }
void onSettings(lv_event_t*)   { s_settings = true; }
void onGoPrinters(lv_event_t*) { s_goPrinters = true; }
void onGoReader(lv_event_t*)   { s_goReader   = true; }
void onHomeBack(lv_event_t*)   { s_homeBack   = true; }

// A status dot: 9 px, and colour is the only thing that changes. Green means
// the printer answered on its control port recently; grey means it did not.
lv_obj_t* makeDot(lv_obj_t* parent, uint32_t colour) {
    lv_obj_t* d = lv_obj_create(parent);
    lv_obj_remove_style_all(d);
    lv_obj_set_size(d, 9, 9);
    lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(d, lv_color_hex(colour), 0);
    lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
    return d;
}

void buildScreen() {
    s_screen = lv_obj_create(nullptr);
    lv_obj_add_style(s_screen, theme::screenStyle(), 0);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);

    // ---- header: title, status dots, gear -----------------------------------
    lv_obj_t* header = lv_obj_create(s_screen);
    lv_obj_remove_style_all(header);
    lv_obj_add_style(header, theme::headerStyle(), 0);
    lv_obj_set_size(header, theme::SCREEN_W, theme::HEADER_H);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // 56 x 44 of hit area for a 24 px glyph, the same target frame::build
    // gives every other screen.
    s_back = lv_btn_create(header);
    lv_obj_remove_style_all(s_back);
    lv_obj_set_size(s_back, 56, theme::HEADER_H);
    lv_obj_align(s_back, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(s_back, onHomeBack, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_flag(s_back, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t* backGlyph = lv_label_create(s_back);
    lv_label_set_text(backGlyph, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(backGlyph, &font_ui_24, 0);
    lv_obj_set_style_text_color(backGlyph, lv_color_hex(theme::TEXT), 0);
    lv_obj_center(backGlyph);

    lv_obj_t* title = s_title = lv_label_create(header);
    lv_label_set_text(title, i18n::T(S_PRINTER));
    // Truncated, never overlapping: the title shared its line with three icons
    // and nothing stopped a long word from running under them.
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_height(title, lv_font_get_line_height(&font_ui_bold_16));
    // This screen builds its own header rather than using frame::build, so the
    // title's weight has to be set here too - and it has to match, or the home
    // screen is the one place in the product where a title is lighter.
    lv_obj_set_style_text_font(title, &font_ui_bold_16, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(theme::TEXT), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 9, 0);

    // Account, then Wi-Fi, then the gear. Two things this screen depends on
    // and cannot show you otherwise: the printers come from the account, and
    // they are reached over Wi-Fi. The same person glyph the Account row in
    // Settings uses, so the two are recognisably the same subject.
    //
    // Side by side at one height - 21 px of ink each, rows 12 to 32 - with
    // 11 px of black between one icon's ink and the next. The offsets place INK, not boxes:
    // the gear's glyph sits centred in the header's right-hand 52 px (ink
    // from x 203),
    // the wave's ink fills its 30 px box, and the bust's ink ends 3 px inside
    // its 22 px one. Measured on /screen.bmp; the boxes alone would have
    // spaced them 9 and 20 px apart, which is what this replaced.
    s_account = icons::build(header, icons::USER, theme::OK);
    lv_obj_align(s_account, LV_ALIGN_RIGHT_MID, -theme::ICON_HIT_W - 33, 0);

    s_wifi = icons::wifiWave(header);
    lv_obj_align(s_wifi, LV_ALIGN_RIGHT_MID, -theme::ICON_HIT_W + 5, -1);

    // ONE target for all three, and it opens Settings. The account and the
    // Wi-Fi are both rows there, so a finger that lands on the bust or the
    // wave is asking for the same place the gear goes - and three icons that
    // look alike and only one of which answers read as a broken screen.
    //
    // It starts at x 126, 8 px left of the bust's box, which leaves the
    // longest title - the French and the Portuguese one, ending at x 117 -
    // outside it. 114 x 44 px, the whole header height: sizing a target to its icon is
    // how a 2 mm button happens. The bust and the wave are not clickable
    // (icons::piece clears the flag), and this is created after them, so it
    // is the one a tap on either finds.
    static const lv_coord_t ICONS_X = 126;
    const lv_coord_t hitW = theme::SCREEN_W - ICONS_X;
    lv_obj_t* gear = s_gear = lv_btn_create(header);
    lv_obj_remove_style_all(gear);
    lv_obj_set_size(gear, hitW, theme::HEADER_H);
    lv_obj_align(gear, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(gear, onSettings, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* gearIcon = s_gearIcon = lv_label_create(gear);
    lv_label_set_text(gearIcon, LV_SYMBOL_SETTINGS);
    // 24 px, a size up from the row icons. At 20 its ink was exactly as tall as
    // the bust's and the wave's and it still read as the small one: a gear is
    // mostly gaps - six teeth and a hole - where the other two are solid
    // shapes, so matching the numbers undersized it to the eye.
    lv_obj_set_style_text_font(gearIcon, &font_ui_24, 0);
    lv_obj_set_style_text_color(gearIcon, lv_color_hex(theme::TEXT), 0);
    // Where it was when its target was the right-hand 52 px: centred there, one
    // pixel right, 11 px from the wave like the bust.
    lv_obj_align(gearIcon, LV_ALIGN_CENTER, hitW / 2 - theme::ICON_HIT_W / 2 + 1, 0);

    // ---- the list ------------------------------------------------------------
    // A flex column inside a scrollable container: LVGL handles the drag, the
    // momentum and the "a tap that moved is not a tap" rule that had to be
    // written by hand against raw touch coordinates.
    s_list = lv_obj_create(s_screen);
    lv_obj_remove_style_all(s_list);
    lv_obj_set_size(s_list, theme::SCREEN_W, theme::SCREEN_H - theme::HEADER_H);
    lv_obj_align(s_list, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(s_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(s_list, theme::PAD, 0);
    lv_obj_set_style_pad_row(s_list, theme::GAP, 0);
    lv_obj_set_scroll_dir(s_list, LV_DIR_VER);
    theme::scrollbar(s_list);
    lv_obj_set_style_bg_opa(s_list, LV_OPA_TRANSP, 0);
}

// The header wears the same two faces as the list under it.
//
// On the choice, the account and the Wi-Fi are what a person wants to see
// before pressing anything, and there is no back to offer. On the printer list
// there IS a back, and a chevron plus a title plus three icons do not fit on
// 240 px: the French title alone is 108 px wide. So the two status icons are
// the ones that go - they belong to the screen the chevron leads to, which is
// one tap away and is where the device starts.
void headerFace(bool choice) {
    static const lv_coord_t ICONS_X = 126;   // as in buildScreen
    if (choice) {
        lv_obj_add_flag(s_back, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_account, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_wifi, LV_OBJ_FLAG_HIDDEN);
        const lv_coord_t hitW = theme::SCREEN_W - ICONS_X;
        lv_obj_set_size(s_gear, hitW, theme::HEADER_H);
        lv_obj_align(s_gear, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_align(s_gearIcon, LV_ALIGN_CENTER,
                     hitW / 2 - theme::ICON_HIT_W / 2 + 1, 0);
        lv_obj_set_width(s_title, ICONS_X - 9 - 6);
        lv_obj_align(s_title, LV_ALIGN_LEFT_MID, 9, 0);
    } else {
        lv_obj_clear_flag(s_back, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_account, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_wifi, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(s_gear, theme::ICON_HIT_W, theme::HEADER_H);
        lv_obj_align(s_gear, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_align(s_gearIcon, LV_ALIGN_CENTER, 1, 0);
        lv_obj_set_width(s_title, theme::SCREEN_W - theme::ICON_HIT_W - 56 - 6);
        lv_obj_align(s_title, LV_ALIGN_LEFT_MID, 56, 0);
    }
}

}  // namespace

namespace screen_home {

// The two rows of the first screen.
//
// frame::row is the settings row, 48 px: the right shape, one size too small
// for a screen whose whole content is two choices. Built through it and then
// grown to 72, so these stay the same component as every other row in the
// product - same style, same pressed state, same icon column - rather than a
// second kind of row that drifts from it.
static const lv_coord_t MAIN_ROW_H = 72;

static void mainRow(lv_obj_t* parent, icons::Id icon, uint32_t colour,
                    const char* label, const char* hint, lv_event_cb_t cb,
                    bool enabled = true) {
    // A disabled row keeps its chevron off: the chevron is the mark that says
    // "this opens something", and one on a row that opens nothing is a promise
    // the screen does not keep.
    lv_obj_t* r = frame::row(parent, label, nullptr, enabled, enabled ? cb : nullptr,
                             nullptr, icon, colour);
    lv_obj_set_height(r, MAIN_ROW_H);
    if (!enabled) {
        // No pressed state either, for the same reason: a row that lights up
        // under the finger and then does nothing reads as a broken device
        // rather than as a feature that is not ready.
        lv_obj_clear_flag(r, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(r, LV_OPA_40, 0);
    }

    // The label a size up, with its hint under it rather than beside it.
    //
    // Beside it is what a settings row does, and on these two it did not fit:
    // the value is given 74 px of a 240 px row, and what was left truncated
    // "Imprimantes" to "Impriman...". The second line is also the honest shape
    // for what these say - "Read a spool" is a sentence about the row, not a
    // value of it.
    lv_obj_t* l = lv_obj_get_child(r, icon == icons::NONE ? 0 : 1);
    if (!l) return;

    lv_obj_t* col = lv_obj_create(r);
    lv_obj_remove_style_all(col);
    lv_obj_set_height(col, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(col, 1);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
    // An LVGL base object is CLICKABLE the moment it is created, and a click
    // does not bubble to its parent. This one covers most of the row, so
    // leaving the flag set made the row look pressable and do nothing - the
    // press landed on the text column and stopped there.
    lv_obj_clear_flag(col, LV_OBJ_FLAG_CLICKABLE);
    // Before the chevron, which frame::row appended last.
    lv_obj_move_to_index(col, icon == icons::NONE ? 0 : 1);

    lv_obj_set_parent(l, col);
    lv_obj_set_flex_grow(l, 0);
    lv_obj_set_width(l, LV_PCT(100));
    lv_obj_set_style_min_width(l, 0, 0);
    lv_obj_set_style_text_font(l, &font_ui_20, 0);
    if (!enabled) lv_obj_set_style_text_color(l, lv_color_hex(theme::TEXT_DIM), 0);

    if (hint && *hint) {
        lv_obj_t* h = lv_label_create(col);
        lv_label_set_text(h, hint);
        // ONE line, truncated. LV_LABEL_LONG_DOT wraps before it dots, and a
        // translation one word too long for the row grew the label instead of
        // cutting it - the second line pushed the row's own text off centre.
        lv_label_set_long_mode(h, LV_LABEL_LONG_DOT);
        lv_obj_set_height(h, lv_font_get_line_height(&font_ui_12));
        lv_obj_set_width(h, LV_PCT(100));
        lv_obj_set_style_text_font(h, &font_ui_12, 0);
        lv_obj_set_style_text_color(h, lv_color_hex(theme::TEXT_DIM), 0);
        lv_obj_set_style_pad_top(h, 2, 0);
    }
}

void showMain(int printersUp, int printersTotal, bool readerReady,
              int wifiRssi, int account) {
    lvgl_port::Lock lvglGuard;   // LVGL is not reentrant - see lvgl_port.h
    if (!s_screen) buildScreen();

    static uint32_t lastSig = 0;
    static bool     built   = false;
    const uint32_t sig = 0x4D41494Eu ^ (uint32_t)(printersUp * 31 + printersTotal * 7)
                       ^ ((uint32_t)readerReady << 20)
                       ^ ((uint32_t)icons::wifiLevelSmoothed(wifiRssi) << 24)
                       ^ ((uint32_t)(account & 3) << 28)
                       ^ ((uint32_t)i18n::current() << 16);
    if (built && s_active && s_face == 1 && sig == lastSig) return;
    lastSig = sig; built = true; s_face = 1;

    lv_label_set_text(s_title, "TigerSpool");
    headerFace(true);

    // From the top, not centred: this list grows - write is already on it and
    // greyed - and a centred stack moves every row down the screen each time
    // one is added. What is at the top stays where a finger expects it.
    lv_obj_clean(s_list);
    lv_obj_set_flex_align(s_list, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(s_list, LV_OBJ_FLAG_SCROLLABLE);

    char up[48];
    snprintf(up, sizeof(up), "%d/%d %s", printersUp, printersTotal, i18n::T(S_ONLINE));
    mainRow(s_list, icons::PRINTER, printersUp ? theme::OK : theme::TEXT_DIM,
            i18n::T(S_PRINTER),
            printersTotal ? up : i18n::T(S_FIND_PRINTERS), onGoPrinters);
    // The same amber the reader screen gives it. One picture for the reader,
    // one colour: the row and the screen it opens are the same subject.
    mainRow(s_list, icons::NFC, readerReady ? theme::ACCENT : theme::TEXT_DIM,
            i18n::T(S_READ_MODE), i18n::T(S_READ_HINT), onGoReader);
    // Writing a spool from the device itself is not built yet. It is on the
    // screen and greyed rather than absent: what the box will do is part of
    // what it is, and a row that appears later moves the two above it.
    mainRow(s_list, icons::NFC, theme::TEXT_DIM,
            i18n::T(S_WRITE_MODE), i18n::T(S_SOON), nullptr, false);

    icons::tint(s_account, account >= 3 ? theme::OK
                         : account == 0 ? theme::DANGER : theme::WARN);
    icons::setSignal(s_wifi, icons::wifiLevelSmoothed(wifiRssi), wifiRssi != 0);

    if (!s_active) { lv_scr_load(s_screen); s_active = true; }
}

bool takeGoPrinters() { bool v = s_goPrinters; s_goPrinters = false; return v; }
bool takeBack()       { bool v = s_homeBack;   s_homeBack   = false; return v; }
bool takeGoReader()   { bool v = s_goReader;   s_goReader   = false; return v; }

// A cheap signature of everything on screen. show() is called from the main
// loop, and rebuilding the list on every one of those calls destroys each row
// under the finger that is pressing it - taps never land, the CPU does nothing
// else, and the screen looks frozen while the device is perfectly healthy.
static uint32_t signature(const PrinterCfg* printers, int count,
                          int selected, const uint8_t* state, bool syncing,
                          int wifiRssi, int account) {
    // The language is in it: the list's own words - no printers yet, all
    // hidden - are translated too.
    uint32_t h = 2166136261u ^ (uint32_t)selected ^ ((uint32_t)syncing << 16)
               ^ ((uint32_t)icons::wifiLevelSmoothed(wifiRssi) << 24)
               ^ ((uint32_t)account << 12) ^ ((uint32_t)i18n::current() << 20);
    for (int i = 0; i < count; i++) {
        h = h * 16777619u ^ (uint32_t)printers[i].type;
        h = h * 16777619u ^ (uint32_t)printers[i].visible;
        h = h * 16777619u ^ (uint32_t)(state ? state[i] : 0);
        for (const char* p = printers[i].name.c_str(); *p; p++)
            h = h * 16777619u ^ (uint8_t)*p;
    }
    return h;
}

void show(const PrinterCfg* printers, int count,
          int selected, const uint8_t* state, bool syncing, int wifiRssi,
          int account) {
    lvgl_port::Lock lvglGuard;   // LVGL is not reentrant - see lvgl_port.h
    if (!s_screen) buildScreen();

    static uint32_t lastSig = 0;
    static bool     everBuilt = false;
    uint32_t sig = signature(printers, count, selected, state, syncing, wifiRssi, account);
    if (everBuilt && s_active && s_face == 2 && sig == lastSig) return;
    lastSig = sig; everBuilt = true; s_face = 2;

    // Set on every rebuild rather than only when the language changes: the
    // other face writes the product name here, so a test against the language
    // alone would leave "TigerSpool" over the printer list.
    lv_label_set_text(s_title, i18n::T(S_PRINTER));
    headerFace(false);

    lv_obj_set_flex_align(s_list, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(s_list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clean(s_list);
    int shown = 0, configured = 0;
    for (int i = 0; i < count; i++) {
        if (printers[i].type == PT_NONE) continue;
        configured++;
        // Hidden in Settings -> Printers. This screen used to filter on type
        // alone and showed everything regardless, which made the picker look
        // like it did nothing.
        if (!printers[i].visible) continue;
        shown++;

        lv_obj_t* row = lv_btn_create(s_list);
        lv_obj_remove_style_all(row);
        lv_obj_add_style(row, theme::rowStyle(), 0);
        lv_obj_add_style(row, theme::rowPressedStyle(), LV_STATE_PRESSED);
        lv_obj_set_size(row, LV_PCT(100), theme::ROW_H);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_event_cb(row, onRow, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        if (i == selected)
            lv_obj_set_style_outline_width(row, 2, 0),
            lv_obj_set_style_outline_color(row, lv_color_hex(theme::ACCENT), 0);

        lv_obj_t* name = lv_label_create(row);
        lv_label_set_text(name, printers[i].name.c_str());
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
        lv_obj_set_flex_grow(name, 1);
        // The printer's name in the weight a settings row uses. It is the same
        // job: the one word on a 48 px line that a person reads before pressing
        // it, at arm's length.
        lv_obj_set_style_text_font(name, &font_ui_bold_16, 0);

        const uint8_t d = state ? state[i] : DOT_OFF;
        makeDot(row, d == DOT_UP     ? theme::OK
                   : d == DOT_TRYING ? theme::BUSY
                                     : theme::DANGER);
    }

    if (!shown) {
        // Nothing but the way out of it.
        //
        // Two situations look identical on an empty list - the account has no
        // printers, or they are all hidden - and both are answered on the
        // same screen, so saying which one it is buys nothing: it was a line
        // of text ("All printers hidden. Settings > Printers") that a person
        // had to translate into taps. The button does the translating.
        lv_obj_t* pad = lv_obj_create(s_list);
        lv_obj_remove_style_all(pad);
        lv_obj_set_size(pad, 1, theme::GAP);

        lv_obj_t* b = frame::button(s_list, i18n::T(S_SELECT_PRINTERS), 0,
                                    []() { s_pick = true; });
        // Two lines, and the height to hold them: the label names what it
        // opens, and in French, Portuguese and Italian that is over twenty
        // characters - on one line it ran out of both ends of the button.
        lv_obj_set_height(b, theme::BUTTON_H + 24);
        lv_obj_t* l = lv_obj_get_child(b, 0);
        lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(l, LV_PCT(85));
        lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(l);
    }

    // Green reachable, orange signed in but unreachable - which on this device
    // means the internet is down, and is the case worth seeing - red not
    // signed in at all.
    // Red no account to be connected to, blue linked and working on it, orange
    // linked but the last exchange failed - the one a user can act on - green
    // the last exchange succeeded. Green is a claim about the exchange, not
    // about holding a token: a device whose network died stops claiming to be
    // fine instead of waiting half an hour for its token to expire.
    static const uint32_t ACCT_COLOUR[4] = {
        theme::DANGER, theme::BUSY, theme::WARN, theme::OK };
    icons::tint(s_account, ACCT_COLOUR[account < 0 ? 0 : (account > 3 ? 3 : account)]);

    // Length, not colour. The glyph used to go red, orange, then green as the
    // signal improved, which made a perfectly usable -70 dBm look like a fault
    // - orange means "something needs your attention" everywhere else on this
    // device, and a slightly weaker signal does not. The rule this settles, and
    // the TigerScale has been following it all along: colour carries a STATE
    // (green connected, red no network), length carries a QUANTITY. Nobody has
    // to wonder whether a yellow means "middling" or "look out".
    icons::setSignal(s_wifi, icons::wifiLevelSmoothed(wifiRssi), wifiRssi != 0);


    if (!s_active) {
        lv_scr_load(s_screen);
        lv_obj_invalidate(s_screen);   // full repaint: a legacy screen drew last
        s_active = true;
    }
}

bool active() { return s_active; }
void leave()  { s_active = false; }

int  takeTappedPrinter() { int v = s_tapped; s_tapped = -1; return v; }
bool takePickTap()       { bool v = s_pick; s_pick = false; return v; }
bool takeSettingsTap()   { bool v = s_settings; s_settings = false; return v; }

}  // namespace screen_home
