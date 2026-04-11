#include "ss_config.h"

#if COMPILE_SS_TUI

#ifndef _SS_TUI_H_
#define _SS_TUI_H_

#include <stdint.h>

/* ─────────────────────────────────────────────────────────────
 * TUI – Tiny Terminal UI Library
 *
 * Kein printf, kein stdio, kein stdlib.
 * Einzige externe Abhängigkeit:
 *   SS_FEEDBACK ss_uart_send(uint8_t interface,
 *                            uint8_t *value,
 *                            uint32_t len);
 * ───────────────────────────────────────────────────────────── */

/* ── Foreground colours ── */
#define SS_TUI_FG_DEFAULT   39
#define SS_TUI_FG_BLACK     30
#define SS_TUI_FG_RED       31
#define SS_TUI_FG_GREEN     32
#define SS_TUI_FG_YELLOW    33
#define SS_TUI_FG_BLUE      34
#define SS_TUI_FG_MAGENTA   35
#define SS_TUI_FG_CYAN      36
#define SS_TUI_FG_WHITE     37
#define SS_TUI_FG_BBLACK    90
#define SS_TUI_FG_BRED      91
#define SS_TUI_FG_BGREEN    92
#define SS_TUI_FG_BYELLOW   93
#define SS_TUI_FG_BBLUE     94
#define SS_TUI_FG_BMAGENTA  95
#define SS_TUI_FG_BCYAN     96
#define SS_TUI_FG_BWHITE    97

/* ── Background colours ── */
#define SS_TUI_BG_DEFAULT   49
#define SS_TUI_BG_BLACK     40
#define SS_TUI_BG_RED       41
#define SS_TUI_BG_GREEN     42
#define SS_TUI_BG_YELLOW    43
#define SS_TUI_BG_BLUE      44
#define SS_TUI_BG_MAGENTA   45
#define SS_TUI_BG_CYAN      46
#define SS_TUI_BG_WHITE     47
#define SS_TUI_BG_BBLACK   100
#define SS_TUI_BG_BRED     101
#define SS_TUI_BG_BGREEN   102
#define SS_TUI_BG_BYELLOW  103
#define SS_TUI_BG_BBLUE    104
#define SS_TUI_BG_BMAGENTA 105
#define SS_TUI_BG_BCYAN    106
#define SS_TUI_BG_BWHITE   107

/* ── Text attributes ── */
#define SS_TUI_ATTR_RESET     0
#define SS_TUI_ATTR_BOLD      1
#define SS_TUI_ATTR_DIM       2
#define SS_TUI_ATTR_UNDERLINE 4
#define SS_TUI_ATTR_BLINK     5
#define SS_TUI_ATTR_REVERSE   7

/* ════════════════════════════════════════════
 * Widget system – pool configuration
 * Override any of these in ss_config.h before
 * including this header.
 * ════════════════════════════════════════════ */
#ifndef SS_TUI_MAX_ELEMENTS
#  define SS_TUI_MAX_ELEMENTS     32   /* total slots in the element pool   */
#endif
#ifndef SS_TUI_MAX_KV_PER_BOX
#  define SS_TUI_MAX_KV_PER_BOX    8   /* key-value rows per text box       */
#endif
#ifndef SS_TUI_TEXT_MAX_LEN
#  define SS_TUI_TEXT_MAX_LEN     32   /* max chars for a text element      */
#endif
#ifndef SS_TUI_KEY_MAX_LEN
#  define SS_TUI_KEY_MAX_LEN      16   /* max chars for a key label         */
#endif
#ifndef SS_TUI_NAME_MAX_LEN
#  define SS_TUI_NAME_MAX_LEN     24   /* max chars for a text-box title    */
#endif

/* ── Box-drawing characters (UTF-8) ── */
#define SS_TUI_BOX_TL    "\xe2\x94\x8c"   /* ┌ */
#define SS_TUI_BOX_TR    "\xe2\x94\x90"   /* ┐ */
#define SS_TUI_BOX_BL    "\xe2\x94\x94"   /* └ */
#define SS_TUI_BOX_BR    "\xe2\x94\x98"   /* ┘ */
#define SS_TUI_BOX_H     "\xe2\x94\x80"   /* ─ */
#define SS_TUI_BOX_V     "\xe2\x94\x82"   /* │ */
#define SS_TUI_BOX_ML    "\xe2\x94\x9c"   /* ├ */
#define SS_TUI_BOX_MR    "\xe2\x94\xa4"   /* ┤ */
#define SS_TUI_BOX_MT    "\xe2\x94\xac"   /* ┬ */
#define SS_TUI_BOX_MB    "\xe2\x94\xb4"   /* ┴ */
#define SS_TUI_BOX_CROSS "\xe2\x94\xbc"   /* ┼ */


/* ════════════════════════════════════════════
 * Initialisierung
 * ════════════════════════════════════════════ */

/* Muss einmal aufgerufen werden, bevor irgendeine andere
 * ss_tui_*-Funktion genutzt wird.                          */
void ss_tui_init(uint8_t uart_interface);


/* ════════════════════════════════════════════
 * Low-level Primitives
 * ════════════════════════════════════════════ */

void ss_tui_goto(uint16_t row, uint16_t col);   /* Cursor bewegen (1-basiert)  */
void ss_tui_fg(uint8_t fg_colour);              /* Vordergrundfarbe setzen      */
void ss_tui_bg(uint8_t bg_colour);              /* Hintergrundfarbe setzen      */
void ss_tui_color(uint8_t fg, uint8_t bg);      /* Beide Farben auf einmal      */
void ss_tui_attr(uint8_t attribute);            /* Textattribut setzen          */
void ss_tui_reset(void);                        /* Alle Attribute zurücksetzen  */
void ss_tui_clear(void);                        /* Bildschirm löschen           */
void ss_tui_clear_eol(void);                    /* Bis Zeilenende löschen       */
void ss_tui_cursor_hide(void);
void ss_tui_cursor_show(void);


/* ════════════════════════════════════════════
 * Zeichen & String Ausgabe
 * ════════════════════════════════════════════ */

void ss_tui_putc(char c);                   /* Zeichen an aktueller Position   */
void ss_tui_puts(const char *str);          /* String  an aktueller Position   */

/* Nur Position */
void ss_tui_putc_at(uint16_t row, uint16_t col, char c);
void ss_tui_puts_at(uint16_t row, uint16_t col, const char *str);

/* Position + Vordergrundfarbe */
void ss_tui_putc_colored(uint16_t row, uint16_t col, uint8_t fg, char c);
void ss_tui_puts_colored(uint16_t row, uint16_t col, uint8_t fg, const char *str);

/* Position + Vordergrund- und Hintergrundfarbe */
void ss_tui_putc_colored_bg(uint16_t row, uint16_t col,
                         uint8_t fg, uint8_t bg, char c);
void ss_tui_puts_colored_bg(uint16_t row, uint16_t col,
                         uint8_t fg, uint8_t bg, const char *str);


/* ════════════════════════════════════════════
 * Zahlen-Ausgabe  (kein printf)
 * ════════════════════════════════════════════ */

/* Vorzeichenloser Integer an aktueller Position */
void ss_tui_put_uint(uint32_t value);

/* Vorzeichenbehafteter Integer an aktueller Position */
void ss_tui_put_int(int32_t value);

/* Fixpunkt:  value = Ganzzahl-Rohwert,  decimals = Nachkommastellen
 * Beispiel:  ss_tui_put_fixed(2350, 2)  -->  "23.50"
 *            ss_tui_put_fixed(-105, 1)  -->  "-10.5"               */
void ss_tui_put_fixed(int32_t value, uint8_t decimals);

/* Hex mit optionaler Mindestbreite (0 = keine Auffüllung) */
void ss_tui_put_hex(uint32_t value, uint8_t min_digits);

/* Kombinierte Helfer: Position + Farbe + Zahl */
void ss_tui_put_uint_colored(uint16_t row, uint16_t col,
                          uint8_t fg, uint32_t value);
void ss_tui_put_int_colored(uint16_t row, uint16_t col,
                         uint8_t fg, int32_t value);
void ss_tui_put_fixed_colored(uint16_t row, uint16_t col,
                           uint8_t fg, int32_t value, uint8_t decimals);
void ss_tui_put_hex_colored(uint16_t row, uint16_t col,
                         uint8_t fg, uint32_t value, uint8_t min_digits);


/* ════════════════════════════════════════════
 * Box & Tabellen-Zeichnen
 * ════════════════════════════════════════════ */

/* Rechteckigen Rahmen zeichnen.
 *   row, col : obere linke Ecke (1-basiert)
 *   width    : innere Breite  (ohne Rahmen-Spalten)
 *   height   : innere Höhe   (ohne Rahmen-Zeilen)
 *   fg       : Rahmenfarbe
 *   title    : Titeltext in oberer Linie (NULL = kein Titel)     */
void ss_tui_box(uint16_t row, uint16_t col,
             uint16_t width, uint16_t height,
             uint8_t fg, const char *title);

/* Horizontale Trennlinie innerhalb eines Rahmens */
void ss_tui_box_separator(uint16_t row, uint16_t col,
                       uint16_t width, uint8_t fg);


/* ════════════════════════════════════════════
 * Widget System
 * ════════════════════════════════════════════ */

/* ── Position ── */
typedef struct {
    uint16_t row;
    uint16_t col;
} ss_tui_pos_t;

/* ── Element type tag ── */
typedef enum {
    SS_TUI_ETYPE_NONE = 0,
    SS_TUI_ETYPE_TEXT,
    SS_TUI_ETYPE_LINE,
    SS_TUI_ETYPE_KV,
    SS_TUI_ETYPE_TEXT_BOX,
} ss_tui_etype_t;

/* Returned by create functions on failure */
#define SS_TUI_INVALID_ID  (-1)

/* ── Global colour control ──────────────────
 * Defaults: font=WHITE  bg=BLUE  line=RED
 * Applies to all elements when drawn.        */
void ss_tui_set_font_color(uint8_t fg);
void ss_tui_set_background_color(uint8_t bg);
void ss_tui_set_line_color(uint8_t fg);

/* ── Text box ───────────────────────────────
 * A bordered box with a centered title that
 * holds key-value rows.
 *
 *  ss_tui_text_box_create()        → box_id
 *  ss_tui_text_box_add_key_value() → kv_id
 *  ss_tui_text_box_set_value(kv_id, new_float)
 *
 * Width and height are calculated automatically:
 *   inner_width  = max(key_len) + 14
 *   inner_height = number of key-value rows
 *
 * Value display format: " ±XXXXX.XX"  (9 chars)
 *   – 5 integer digits, space-padded
 *   – always 2 decimal places                */
int  ss_tui_text_box_create(uint8_t slide_id,
                            ss_tui_pos_t pos,
                            const char *name);

int  ss_tui_text_box_add_key_value(int box_id,
                                   const char *key,
                                   float value);

void ss_tui_text_box_set_value(int kv_id, float value);

/* ── Standalone text element ─────────────── */
int  ss_tui_text_create(uint8_t slide_id,
                        ss_tui_pos_t pos,
                        const char *text);

void ss_tui_text_set(int text_id, const char *text);

/* ── Standalone line element ─────────────── */
int  ss_tui_line_create(uint8_t slide_id,
                        ss_tui_pos_t start,
                        ss_tui_pos_t end);

/* ── Slide control ───────────────────────────
 * ss_tui_update(slide_id)
 *   Draws every element of that slide whose
 *   update_needed flag is 0 (dirty), then
 *   sets the flag to 1 (clean).
 *
 * ss_tui_erase()
 *   Clears the terminal and marks every element
 *   (all slides) as dirty so that the next
 *   ss_tui_update() redraws everything.        */
void ss_tui_update(uint8_t slide_id);
void ss_tui_erase(void);


#endif
#endif