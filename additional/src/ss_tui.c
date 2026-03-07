#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_TUI



#include "ss_tui.h"

/* ─────────────────────────────────────────────────────────────
 * Passe diese Zeile an dein Projekt an.
 * Das Header muss SS_FEEDBACK und ss_uart_send deklarieren.
 * ───────────────────────────────────────────────────────────── */
#include "ss_uart.h"
#include "ss_rtos.h"

/* ════════════════════════════════════════════════════════════
 * Interne Hilfsfunktionen
 * ════════════════════════════════════════════════════════════ */

static uint8_t _iface = 0;

/* Rohe Bytes senden */
static void _send(const uint8_t *data, uint32_t len)
{
    ss_uart_send(_iface, (uint8_t *)data, len);
    ss_rtos_delay_ms(1);
}

/* Null-terminierten String senden */
static void _send_str(const char *s)
{
    uint32_t len = 0;
    while (s[len]) len++;
    _send((const uint8_t *)s, len);
}

/* Einzelnes Zeichen senden */
static void _send_char(char c)
{
    _send((const uint8_t *)&c, 1);
}

/* ── Hilfsfunktion: uint32_t → ASCII-Ziffern in Puffer schreiben.
 *    Gibt Anzahl geschriebener Zeichen zurück.
 *    buf muss mindestens 10 Bytes groß sein.                    */
static uint8_t _uint_to_str(uint32_t value, char *buf)
{
    if (value == 0) {
        buf[0] = '0';
        return 1;
    }
    char tmp[10];
    uint8_t i = 0;
    while (value > 0) {
        tmp[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    /* Umkehren */
    uint8_t len = i;
    for (uint8_t j = 0; j < len; j++) {
        buf[j] = tmp[len - 1 - j];
    }
    return len;
}

/* ── ESC-Sequenz senden die eine oder zwei Zahlen enthält.
 *
 *   Aufbau:  ESC [ <n1> <suffix>          (two_nums = 0)
 *            ESC [ <n1> ; <n2> <suffix>   (two_nums = 1)     */
static void _esc1(uint8_t n1, char suffix)
{
    char buf[8];
    uint8_t pos = 0;
    buf[pos++] = '\033';
    buf[pos++] = '[';
    uint8_t len = _uint_to_str((uint32_t)n1, buf + pos);
    pos += len;
    buf[pos++] = suffix;
    _send((const uint8_t *)buf, pos);
}

static void _esc2(uint16_t n1, uint16_t n2, char suffix)
{
    char buf[16];
    uint8_t pos = 0;
    buf[pos++] = '\033';
    buf[pos++] = '[';
    uint8_t len = _uint_to_str((uint32_t)n1, buf + pos);
    pos += len;
    buf[pos++] = ';';
    len = _uint_to_str((uint32_t)n2, buf + pos);
    pos += len;
    buf[pos++] = suffix;
    _send((const uint8_t *)buf, pos);
}


/* ════════════════════════════════════════════════════════════
 * Initialisierung
 * ════════════════════════════════════════════════════════════ */

void ss_tui_init(uint8_t uart_interface)
{
    _iface = uart_interface;
}


/* ════════════════════════════════════════════════════════════
 * Low-level Primitives
 * ════════════════════════════════════════════════════════════ */

void ss_tui_goto(uint16_t row, uint16_t col)
{
    _esc2(row, col, 'H');
}

void ss_tui_fg(uint8_t fg_colour)
{
    _esc1(fg_colour, 'm');
}

void ss_tui_bg(uint8_t bg_colour)
{
    _esc1(bg_colour, 'm');
}

void ss_tui_color(uint8_t fg, uint8_t bg)
{
    /* ESC [ <fg> ; <bg> m */
    char buf[16];
    uint8_t pos = 0;
    buf[pos++] = '\033';
    buf[pos++] = '[';
    uint8_t len = _uint_to_str((uint32_t)fg, buf + pos);
    pos += len;
    buf[pos++] = ';';
    len = _uint_to_str((uint32_t)bg, buf + pos);
    pos += len;
    buf[pos++] = 'm';
    _send((const uint8_t *)buf, pos);
}

void ss_tui_attr(uint8_t attribute)
{
    _esc1(attribute, 'm');
}

void ss_tui_reset(void)
{
    _send_str("\033[0m");
}

void ss_tui_clear(void)
{
    _send_str("\033[2J\033[H");
}

void ss_tui_clear_eol(void)
{
    _send_str("\033[K");
}

void ss_tui_cursor_hide(void)
{
    _send_str("\033[?25l");
}

void ss_tui_cursor_show(void)
{
    _send_str("\033[?25h");
}


/* ════════════════════════════════════════════════════════════
 * Zeichen & String Ausgabe
 * ════════════════════════════════════════════════════════════ */

void ss_tui_putc(char c)
{
    _send_char(c);
}

void ss_tui_puts(const char *str)
{
    if (str) _send_str(str);
}

void ss_tui_putc_at(uint16_t row, uint16_t col, char c)
{
    ss_tui_goto(row, col);
    _send_char(c);
}

void ss_tui_puts_at(uint16_t row, uint16_t col, const char *str)
{
    ss_tui_goto(row, col);
    _send_str(str);
}

void ss_tui_putc_colored(uint16_t row, uint16_t col, uint8_t fg, char c)
{
    ss_tui_goto(row, col);
    ss_tui_fg(fg);
    _send_char(c);
    ss_tui_reset();
}

void ss_tui_puts_colored(uint16_t row, uint16_t col, uint8_t fg, const char *str)
{
    ss_tui_goto(row, col);
    ss_tui_fg(fg);
    _send_str(str);
    ss_tui_reset();
}

void ss_tui_putc_colored_bg(uint16_t row, uint16_t col,
                         uint8_t fg, uint8_t bg, char c)
{
    ss_tui_goto(row, col);
    ss_tui_color(fg, bg);
    _send_char(c);
    ss_tui_reset();
}

void ss_tui_puts_colored_bg(uint16_t row, uint16_t col,
                         uint8_t fg, uint8_t bg, const char *str)
{
    ss_tui_goto(row, col);
    ss_tui_color(fg, bg);
    _send_str(str);
    ss_tui_reset();
}


/* ════════════════════════════════════════════════════════════
 * Zahlen-Ausgabe  (kein printf)
 * ════════════════════════════════════════════════════════════ */

void ss_tui_put_uint(uint32_t value)
{
    char buf[10];
    uint8_t len = _uint_to_str(value, buf);
    _send((const uint8_t *)buf, len);
}

void ss_tui_put_int(int32_t value)
{
    if (value < 0) {
        _send_char('-');
        /* Sonderfall INT32_MIN vermeiden */
        uint32_t uval = (value == -2147483647 - 1)
                        ? 2147483648u
                        : (uint32_t)(-value);
        ss_tui_put_uint(uval);
    } else {
        ss_tui_put_uint((uint32_t)value);
    }
}

void ss_tui_put_fixed(int32_t value, uint8_t decimals)
{
    /* Vorzeichen */
    if (value < 0) {
        _send_char('-');
        value = -value;
    }

    /* Divisor berechnen: 10^decimals */
    uint32_t divisor = 1;
    for (uint8_t i = 0; i < decimals; i++) divisor *= 10;

    uint32_t integer_part  = (uint32_t)value / divisor;
    uint32_t fraction_part = (uint32_t)value % divisor;

    /* Ganzzahlanteil */
    ss_tui_put_uint(integer_part);

    if (decimals > 0) {
        _send_char('.');
        /* Führende Nullen im Nachkommaanteil */
        uint32_t leading = divisor / 10;
        while (leading > 1 && fraction_part < leading) {
            _send_char('0');
            leading /= 10;
        }
        ss_tui_put_uint(fraction_part);
    }
}

void ss_tui_put_hex(uint32_t value, uint8_t min_digits)
{
    static const char hex_chars[] = "0123456789ABCDEF";
    char buf[8];
    uint8_t i = 0;

    /* Ziffern von hinten befüllen */
    do {
        buf[i++] = hex_chars[value & 0xF];
        value >>= 4;
    } while (value > 0);

    /* Auffüllen auf min_digits */
    while (i < min_digits && i < 8) {
        buf[i++] = '0';
    }

    /* Umgekehrt ausgeben */
    for (int8_t j = (int8_t)i - 1; j >= 0; j--) {
        _send_char(buf[j]);
    }
}

/* ── Kombinierte Helfer ── */

void ss_tui_put_uint_colored(uint16_t row, uint16_t col,
                          uint8_t fg, uint32_t value)
{
    ss_tui_goto(row, col);
    ss_tui_fg(fg);
    ss_tui_put_uint(value);
    ss_tui_reset();
}

void ss_tui_put_int_colored(uint16_t row, uint16_t col,
                         uint8_t fg, int32_t value)
{
    ss_tui_goto(row, col);
    ss_tui_fg(fg);
    ss_tui_put_int(value);
    ss_tui_reset();
}

void ss_tui_put_fixed_colored(uint16_t row, uint16_t col,
                           uint8_t fg, int32_t value, uint8_t decimals)
{
    ss_tui_goto(row, col);
    ss_tui_fg(fg);
    ss_tui_put_fixed(value, decimals);
    ss_tui_reset();
}

void ss_tui_put_hex_colored(uint16_t row, uint16_t col,
                         uint8_t fg, uint32_t value, uint8_t min_digits)
{
    ss_tui_goto(row, col);
    ss_tui_fg(fg);
    ss_tui_put_hex(value, min_digits);
    ss_tui_reset();
}


/* ════════════════════════════════════════════════════════════
 * Box & Tabellen-Zeichnen
 * ════════════════════════════════════════════════════════════ */

void ss_tui_box(uint16_t row, uint16_t col,
             uint16_t width, uint16_t height,
             uint8_t fg, const char *title)
{
    ss_tui_fg(fg);

    /* ── Obere Linie ── */
    ss_tui_goto(row, col);
    _send_str(SS_TUI_BOX_TL);

    if (title && title[0] != '\0') {
        uint16_t tlen = 0;
        while (title[tlen]) tlen++;

        /* Format:  ─ Titel ─────  */
        _send_str(SS_TUI_BOX_H);
        _send_char(' ');
        _send_str(title);
        _send_char(' ');

        /* Restliche Striche auffüllen */
        uint16_t used = tlen + 3; /* " title " + linke ─ */
        for (uint16_t i = used; i < width; i++) _send_str(SS_TUI_BOX_H);
    } else {
        for (uint16_t i = 0; i < width; i++) _send_str(SS_TUI_BOX_H);
    }
    _send_str(SS_TUI_BOX_TR);

    /* ── Seitenlinien ── */
    for (uint16_t r = 1; r <= height; r++) {
        ss_tui_goto(row + r, col);
        _send_str(SS_TUI_BOX_V);
        ss_tui_goto(row + r, col + width + 1);
        _send_str(SS_TUI_BOX_V);
    }

    /* ── Untere Linie ── */
    ss_tui_goto(row + height + 1, col);
    _send_str(SS_TUI_BOX_BL);
    for (uint16_t i = 0; i < width; i++) _send_str(SS_TUI_BOX_H);
    _send_str(SS_TUI_BOX_BR);

    ss_tui_reset();
}

void ss_tui_box_separator(uint16_t row, uint16_t col,
                       uint16_t width, uint8_t fg)
{
    ss_tui_goto(row, col);
    ss_tui_fg(fg);
    _send_str(SS_TUI_BOX_ML);
    for (uint16_t i = 0; i < width; i++) _send_str(SS_TUI_BOX_H);
    _send_str(SS_TUI_BOX_MR);
    ss_tui_reset();
}


#endif

