#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_TUI



#include "ss_tui.h"


#include "ss_uart.h"
#include "ss_rtos.h"
#include "ss_error.h"



static uint8_t _iface = 0;

typedef struct {
    uint16_t row;
    uint16_t col;
} ss_tui_pos_t;


typedef struct {
    ss_tui_etype_t type;
    uint8_t        slide_id;
    uint8_t        update_needed; /* 0 = dirty (needs redraw), 1 = clean */
    ss_tui_pos_t   pos;
} _elem_base_t;


typedef struct {
    _elem_base_t base;
    char text[SS_TUI_TEXT_MAX_LEN];
} _elem_text_t;

typedef struct {
    _elem_base_t base;
    ss_tui_pos_t end;
} _elem_line_t;


typedef struct {
    _elem_base_t base;
    char     key[SS_TUI_KEY_MAX_LEN];
    float    value;
    char     value_str[SS_TUI_VALUE_STR_MAX_LEN];
    uint8_t  is_string;
    int      parent_box_id;
} _elem_kv_t;


typedef struct {
    _elem_base_t base;
    char     name[SS_TUI_NAME_MAX_LEN];
    uint16_t inner_width;                    
    uint8_t  max_key_len;                    
    int      kv_ids[SS_TUI_MAX_KV_PER_BOX];
    uint8_t  kv_count;
} _elem_box_t;


/* Input widget states */
#define _INPUT_IDLE    0u
#define _INPUT_FOCUSED 1u
#define _INPUT_EDITING 2u

typedef struct {
    _elem_base_t base;
    char         buf[SS_TUI_INPUT_MAX_LEN];
    char         name[SS_TUI_NAME_MAX_LEN];
    uint8_t      len;
    uint16_t     width;
    uint8_t      state;      /* _INPUT_IDLE / _INPUT_FOCUSED / _INPUT_EDITING */
    uint8_t      activated;  /* 1 = im Tab-Zyklus der Slide */
} _elem_input_t;


typedef union {
    _elem_base_t  base;
    _elem_text_t  text;
    _elem_line_t  line;
    _elem_kv_t    kv;
    _elem_box_t   box;
    _elem_input_t input;
} _slot_data_t;

typedef struct {
    uint8_t      in_use;
    _slot_data_t d;
} _slot_t;


static _slot_t  _pool[SS_TUI_MAX_ELEMENTS];
static uint8_t  _g_font_color = SS_TUI_FG_WHITE;
static uint8_t  _g_bg_color   = SS_TUI_BG_BLUE;
static uint8_t  _g_line_color = SS_TUI_FG_RED;
static int      _focused_input_id = SS_TUI_INVALID_ID;


static int _alloc_slot(void)
{
    for (int i = 0; i < SS_TUI_MAX_ELEMENTS; i++) {
        if (!_pool[i].in_use) {
            _pool[i].in_use = 1;
            return i + 1;
        }
    }
    return SS_TUI_INVALID_ID;
}


static _slot_data_t *_get_slot(int id)
{
    if (id < 1 || id > SS_TUI_MAX_ELEMENTS) return 0;
    if (!_pool[id - 1].in_use)              return 0;
    return &_pool[id - 1].d;
}


static uint8_t _slen(const char *s)
{
    uint8_t n = 0;
    while (s[n]) n++;
    return n;
}

static void _scopy(char *dst, const char *src, uint8_t max)
{
    uint8_t i = 0;
    while (src[i] && i < (uint8_t)(max - 1u)) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}


static void _send(const uint8_t *data, uint32_t len)
{
    ss_uart_send(_iface, (uint8_t *)data, len);
}



static void _send_str(const char *s)
{
    uint32_t len = 0;
    while (s[len]) len++;
    _send((const uint8_t *)s, len);
}

static void _send_char(char c)
{
    _send((const uint8_t *)&c, 1);
}


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

    uint8_t len = i;
    for (uint8_t j = 0; j < len; j++) {
        buf[j] = tmp[len - 1 - j];
    }
    return len;
}


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



void ss_tui_init(uint8_t uart_interface)
{
    _iface = uart_interface;

    /* Reset widget pool and restore default colours */
    for (int i = 0; i < SS_TUI_MAX_ELEMENTS; i++) {
        _pool[i].in_use = 0;
    }
    _g_font_color     = SS_TUI_FG_WHITE;
    _g_bg_color       = SS_TUI_BG_BLUE;
    _g_line_color     = SS_TUI_FG_RED;
    _focused_input_id = SS_TUI_INVALID_ID;
    ss_tui_cursor_hide();
}



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
    _send_str("\033[H\033[2J");
    ss_uart_flush(_iface);
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

    if (value < 0) {
        _send_char('-');
        value = -value;
    }


    uint32_t divisor = 1;
    for (uint8_t i = 0; i < decimals; i++) divisor *= 10;

    uint32_t integer_part  = (uint32_t)value / divisor;
    uint32_t fraction_part = (uint32_t)value % divisor;


    ss_tui_put_uint(integer_part);

    if (decimals > 0) {
        _send_char('.');

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


    do {
        buf[i++] = hex_chars[value & 0xF];
        value >>= 4;
    } while (value > 0);


    while (i < min_digits && i < 8) {
        buf[i++] = '0';
    }

    for (int8_t j = (int8_t)i - 1; j >= 0; j--) {
        _send_char(buf[j]);
    }
}

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


void ss_tui_box(uint16_t row, uint16_t col,
             uint16_t width, uint16_t height,
             uint8_t fg, const char *title)
{
    ss_tui_fg(fg);


    ss_tui_goto(row, col);
    _send_str(SS_TUI_BOX_TL);

    if (title && title[0] != '\0') {
        uint16_t tlen = 0;
        while (title[tlen]) tlen++;


        _send_str(SS_TUI_BOX_H);
        _send_char(' ');
        _send_str(title);
        _send_char(' ');


        uint16_t used = tlen + 3;
        for (uint16_t i = used; i < width; i++) _send_str(SS_TUI_BOX_H);
    } else {
        for (uint16_t i = 0; i < width; i++) _send_str(SS_TUI_BOX_H);
    }
    _send_str(SS_TUI_BOX_TR);


    for (uint16_t r = 1; r <= height; r++) {
        ss_tui_goto(row + r, col);
        _send_str(SS_TUI_BOX_V);
        ss_tui_goto(row + r, col + width + 1);
        _send_str(SS_TUI_BOX_V);
    }


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


static void _put_float_value(float value)
{
    if (value < 0.0f) {
        _send_char('-');
        value = -value;
    } else {
        _send_char(' ');
    }

    if (value > 99999.99f) value = 99999.99f;


    uint32_t scaled   = (uint32_t)(value * 100.0f + 0.5f);
    uint32_t int_part = scaled / 100u;
    uint32_t dec_part = scaled % 100u;


    char ibuf[5];
    uint8_t ilen = _uint_to_str(int_part, ibuf);
    for (uint8_t i = ilen; i < 5; i++) _send_char(' ');
    _send((const uint8_t *)ibuf, ilen);

    _send_char('.');
    if (dec_part < 10) _send_char('0');
    char dbuf[3];
    uint8_t dlen = _uint_to_str(dec_part, dbuf);
    _send((const uint8_t *)dbuf, dlen);
}


static void _put_key_padded(const char *key, uint8_t max_key_len)
{
    uint8_t len = _slen(key);
    _send((const uint8_t *)key, len);
    for (uint8_t i = len; i < max_key_len; i++) _send_char(' ');
}


static void _draw_text_elem(const _elem_text_t *e)
{
    ss_tui_goto(e->base.pos.row, e->base.pos.col);
    ss_tui_color(_g_font_color, _g_bg_color);
    _send_str(e->text);
    ss_tui_reset();
}

static void _draw_line_elem(const _elem_line_t *e)
{
    uint16_t r1 = e->base.pos.row, c1 = e->base.pos.col;
    uint16_t r2 = e->end.row,      c2 = e->end.col;

    ss_tui_fg(_g_line_color);

    if (r1 == r2) {
        /* Horizontal line */
        ss_tui_goto(r1, (c1 < c2 ? c1 : c2));
        uint16_t len = (uint16_t)((c2 >= c1 ? c2 - c1 : c1 - c2) + 1u);
        for (uint16_t i = 0; i < len; i++) _send_str(SS_TUI_BOX_H);
    } else if (c1 == c2) {
        /* Vertical line */
        uint16_t r_start = (r1 < r2 ? r1 : r2);
        uint16_t len = (uint16_t)((r2 >= r1 ? r2 - r1 : r1 - r2) + 1u);
        for (uint16_t i = 0; i < len; i++) {
            ss_tui_goto(r_start + i, c1);
            _send_str(SS_TUI_BOX_V);
        }
    }

    ss_tui_reset();
}

static void _draw_kv_elem(const _elem_kv_t *kv)
{
    /* Retrieve parent box to get the current max_key_len */
    uint8_t max_kl = _slen(kv->key);
    const _slot_data_t *bs = _get_slot(kv->parent_box_id);
    if (bs && bs->base.type == SS_TUI_ETYPE_TEXT_BOX) {
        max_kl = bs->box.max_key_len;
    }

    ss_tui_goto(kv->base.pos.row, kv->base.pos.col);
    ss_tui_color(_g_font_color, _g_bg_color);
    _send_char(' ');
    _put_key_padded(kv->key, max_kl);
    _send_str(" : ");
    if (kv->is_string) {
        uint8_t vlen = _slen(kv->value_str);
        _send((const uint8_t *)kv->value_str, vlen);
        for (uint8_t i = vlen; i < 9u; i++) _send_char(' ');
    } else {
        _put_float_value(kv->value);
    }
    _send_char(' ');
    ss_tui_reset();
}

static void _draw_box_elem(const _elem_box_t *box)
{
    uint16_t row = box->base.pos.row;
    uint16_t col = box->base.pos.col;
    uint16_t w   = box->inner_width;
    uint8_t  h   = box->kv_count;

    

    ss_tui_fg(_g_line_color);

    ss_tui_goto(row, col);
    _send_str(SS_TUI_BOX_TL);

    uint8_t nlen = _slen(box->name);
    if (nlen > 0 && (uint16_t)(nlen + 4u) <= w) {

        uint16_t dash_total  = w - (uint16_t)nlen - 2u;
        uint16_t left_dashes = dash_total / 2u;
        uint16_t right_dashes = dash_total - left_dashes;
        for (uint16_t i = 0; i < left_dashes;  i++) _send_str(SS_TUI_BOX_H);
        _send_char(' ');
        _send_str(box->name);
        _send_char(' ');
        for (uint16_t i = 0; i < right_dashes; i++) _send_str(SS_TUI_BOX_H);
    } else {
        for (uint16_t i = 0; i < w; i++) _send_str(SS_TUI_BOX_H);
    }
    _send_str(SS_TUI_BOX_TR);

    for (uint8_t r = 1; r <= h; r++) {
        ss_tui_goto(row + r, col);
        _send_str(SS_TUI_BOX_V);
        ss_tui_goto(row + r, col + w + 1u);
        _send_str(SS_TUI_BOX_V);
    }

    ss_tui_goto(row + h + 1u, col);
    _send_str(SS_TUI_BOX_BL);
    for (uint16_t i = 0; i < w; i++) _send_str(SS_TUI_BOX_H);
    _send_str(SS_TUI_BOX_BR);

    ss_tui_reset();
}



/* ── Input: Tab-Zyklus (nur innerhalb derselben Slide) ─────────── */

static void _cycle_focus(void)
{
    if (_focused_input_id == SS_TUI_INVALID_ID) return;
    _slot_data_t *cur = _get_slot(_focused_input_id);
    if (!cur || cur->base.type != SS_TUI_ETYPE_INPUT) return;

    uint8_t slide_id    = cur->base.slide_id;
    int     next_id     = SS_TUI_INVALID_ID;
    int     first_id    = SS_TUI_INVALID_ID;
    uint8_t found_cur   = 0;

    for (int i = 0; i < SS_TUI_MAX_ELEMENTS; i++) {
        if (!_pool[i].in_use)                            continue;
        if (_pool[i].d.base.type != SS_TUI_ETYPE_INPUT)  continue;
        if (!_pool[i].d.input.activated)                 continue;
        if (_pool[i].d.base.slide_id != slide_id)        continue;
        int id = i + 1;
        if (first_id == SS_TUI_INVALID_ID) first_id = id;
        if (found_cur && next_id == SS_TUI_INVALID_ID) next_id = id;
        if (id == _focused_input_id) found_cur = 1;
    }

    if (next_id == SS_TUI_INVALID_ID) next_id = first_id; /* wrap */
    if (next_id == SS_TUI_INVALID_ID || next_id == _focused_input_id) return;

    /* Altes Widget defokussieren */
    if (cur->input.state == _INPUT_EDITING) ss_tui_cursor_hide();
    cur->input.state        = _INPUT_IDLE;
    cur->base.update_needed = 0;

    /* Neues Widget fokussieren */
    _slot_data_t *nxt = _get_slot(next_id);
    if (nxt) {
        nxt->input.state        = _INPUT_FOCUSED;
        nxt->base.update_needed = 0;
    }
    _focused_input_id = next_id;
}


/* ── Input: Zeichnen ──────────────────────── */

static void _draw_input_elem(const _elem_input_t *e)
{
    uint16_t row     = e->base.pos.row;
    uint16_t col     = e->base.pos.col;
    uint16_t w       = e->width;
    uint8_t  active  = (e->state != _INPUT_IDLE);
    uint8_t  editing = (e->state == _INPUT_EDITING);
    uint8_t  bcolor  = active ? SS_TUI_FG_YELLOW : _g_line_color;

    /* Oberer Rahmen mit Name links, '*' ganz rechts nur im Edit-Modus */
    ss_tui_fg(bcolor);
    ss_tui_goto(row, col);
    _send_str(SS_TUI_BOX_TL);

    uint8_t nlen = _slen(e->name);
    if (nlen > 0u && (uint16_t)(nlen + 4u) <= w) {
        /* ─ Name ─ ... ─ */
        _send_str(SS_TUI_BOX_H);
        _send_char(' ');
        _send_str(e->name);
        _send_char(' ');
        uint16_t extra = w - (uint16_t)nlen - 4u; /* verbleibende Dashes vor letztem */
        for (uint16_t i = 0; i < extra; i++) _send_str(SS_TUI_BOX_H);
    } else {
        for (uint16_t i = 0; i < w - 1u; i++) _send_str(SS_TUI_BOX_H);
    }
    if (editing) _send_char('*');
    else         _send_str(SS_TUI_BOX_H);
    _send_str(SS_TUI_BOX_TR);

    /* Inhaltszeile */
    ss_tui_goto(row + 1u, col);
    ss_tui_fg(bcolor);
    _send_str(SS_TUI_BOX_V);
    ss_tui_color(_g_font_color, _g_bg_color);
    _send_char(' ');

    /* Text: bei Überlänge die letzten (w-1) Zeichen anzeigen */
    uint16_t text_area   = w > 1u ? (uint16_t)(w - 1u) : 0u;
    uint16_t start       = e->len > text_area ? (uint16_t)(e->len - text_area) : 0u;
    uint16_t display_len = (uint16_t)(e->len - start);
    for (uint16_t i = 0; i < display_len; i++) _send_char(e->buf[start + i]);
    for (uint16_t i = display_len; i < text_area; i++) _send_char(' ');

    ss_tui_fg(bcolor);
    _send_str(SS_TUI_BOX_V);

    /* Unterer Rahmen */
    ss_tui_goto(row + 2u, col);
    _send_str(SS_TUI_BOX_BL);
    for (uint16_t i = 0; i < w; i++) _send_str(SS_TUI_BOX_H);
    _send_str(SS_TUI_BOX_BR);

    ss_tui_reset();
}


/* ── Parser-Helfer (kein stdlib) ──────────── */

static bool _parse_int(const char *buf, uint8_t len, int32_t *out)
{
    if (len == 0u) SS_ERROR("invalid input");

    uint8_t  i    = 0;
    int32_t  sign = 1;

    if      (buf[i] == '-') { sign = -1; i++; }
    else if (buf[i] == '+') {            i++; }

    if (i == len) SS_ERROR("invalid input"); /* nur Vorzeichen */

    int32_t result = 0;
    while (i < len) {
        if (buf[i] < '0' || buf[i] > '9') SS_ERROR("invalid input");
        result = result * 10 + (int32_t)(buf[i] - '0');
        i++;
    }

    *out = sign * result;
    return true;
}

static bool _parse_float(const char *buf, uint8_t len, float *out)
{
    if (len == 0u) SS_ERROR("invalid input");

    uint8_t i        = 0;
    float   sign     = 1.0f;

    if      (buf[i] == '-') { sign = -1.0f; i++; }
    else if (buf[i] == '+') {               i++; }

    if (i == len) SS_ERROR("invalid input");

    /* Ganzzahliger Anteil (max. 5 Stellen) */
    float   result     = 0.0f;
    uint8_t int_digits = 0;
    while (i < len && buf[i] != '.') {
        if (buf[i] < '0' || buf[i] > '9') SS_ERROR("invalid input");
        result = result * 10.0f + (float)(buf[i] - '0');
        int_digits++;
        i++;
    }
    if (int_digits == 0u) SS_ERROR("invalid input");

    /* Nachkommastellen (max. 2) */
    if (i < len && buf[i] == '.') {
        i++;
        float   factor    = 0.1f;
        uint8_t dec_count = 0u;
        while (i < len && dec_count < 2u) {
            if (buf[i] < '0' || buf[i] > '9') SS_ERROR("invalid input");
            result   += (float)(buf[i] - '0') * factor;
            factor   *= 0.1f;
            dec_count++;
            i++;
        }
        if (i < len) SS_ERROR("invalid input"); /* mehr als 2 Dezimalstellen */
    }

    if (i != len) SS_ERROR("invalid input"); /* unbekannte Zeichen am Ende */

    *out = sign * result;
    return true;
}


void ss_tui_set_font_color(uint8_t fg)       { _g_font_color = fg; }
void ss_tui_set_background_color(uint8_t bg) { _g_bg_color   = bg; }
void ss_tui_set_line_color(uint8_t fg)       { _g_line_color = fg; }



int ss_tui_text_box_create(uint8_t slide_id,
                           uint16_t x, uint16_t y,
                           const char *name)
{
    int id = _alloc_slot();
    if (id == SS_TUI_INVALID_ID) return SS_TUI_INVALID_ID;

    _elem_box_t *b        = &_pool[id - 1].d.box;
    b->base.type          = SS_TUI_ETYPE_TEXT_BOX;
    b->base.slide_id      = slide_id;
    b->base.update_needed = 0;
    b->base.pos.col       = x;
    b->base.pos.row       = y;
    _scopy(b->name, name, SS_TUI_NAME_MAX_LEN);
    b->kv_count    = 0;
    b->max_key_len = 0;


    uint8_t  nlen    = _slen(name);
    uint16_t title_w = (uint16_t)nlen + 4u;
    b->inner_width   = (title_w > 14u) ? title_w : 14u;

    return id;
}

int ss_tui_text_box_add_key_value(int box_id,
                                  const char *key,
                                  float value)
{
    _slot_data_t *bs = _get_slot(box_id);
    if (!bs || bs->base.type != SS_TUI_ETYPE_TEXT_BOX) return SS_TUI_INVALID_ID;

    _elem_box_t *box = &bs->box;
    if (box->kv_count >= SS_TUI_MAX_KV_PER_BOX)        return SS_TUI_INVALID_ID;

    int kv_id = _alloc_slot();
    if (kv_id == SS_TUI_INVALID_ID)                     return SS_TUI_INVALID_ID;


    _elem_kv_t *kv        = &_pool[kv_id - 1].d.kv;
    kv->base.type          = SS_TUI_ETYPE_KV;
    kv->base.slide_id      = box->base.slide_id;
    kv->base.update_needed = 0;

    kv->base.pos.row = box->base.pos.row + 1u + box->kv_count;
    kv->base.pos.col = box->base.pos.col + 1u;
    _scopy(kv->key, key, SS_TUI_KEY_MAX_LEN);
    kv->value         = value;
    kv->parent_box_id = box_id;

    box->kv_ids[box->kv_count++] = kv_id;

    uint8_t klen = _slen(key);
    if (klen > box->max_key_len) {
        box->max_key_len = klen;


        uint16_t content_w = (uint16_t)klen + 14u;
        uint8_t  nlen      = _slen(box->name);
        uint16_t title_w   = (uint16_t)nlen + 4u;
        box->inner_width   = (content_w > title_w) ? content_w : title_w;

        for (uint8_t i = 0; i < box->kv_count - 1u; i++) {
            _slot_data_t *s = _get_slot(box->kv_ids[i]);
            if (s) s->base.update_needed = 0;
        }
    }

    box->base.update_needed = 0;

    return kv_id;
}

void ss_tui_text_box_set_value(int kv_id, float value)
{
    _slot_data_t *s = _get_slot(kv_id);
    if (!s || s->base.type != SS_TUI_ETYPE_KV) return;
    s->kv.value           = value;
    s->base.update_needed = 0;
}

int ss_tui_text_box_add_key_string(int box_id, const char *key, const char *value)
{
    int kv_id = ss_tui_text_box_add_key_value(box_id, key, 0.0f);
    if (kv_id == SS_TUI_INVALID_ID) return SS_TUI_INVALID_ID;

    _slot_data_t *s = _get_slot(kv_id);
    s->kv.is_string = 1;
    _scopy(s->kv.value_str, value, SS_TUI_VALUE_STR_MAX_LEN);
    return kv_id;
}

void ss_tui_text_box_set_string(int kv_id, const char *str)
{
    _slot_data_t *s = _get_slot(kv_id);
    if (!s || s->base.type != SS_TUI_ETYPE_KV) return;
    _scopy(s->kv.value_str, str, SS_TUI_VALUE_STR_MAX_LEN);
    s->base.update_needed = 0;
}

int ss_tui_text_create(uint8_t slide_id,
                       uint16_t x, uint16_t y,
                       const char *text)
{
    int id = _alloc_slot();
    if (id == SS_TUI_INVALID_ID) return SS_TUI_INVALID_ID;

    _elem_text_t *e        = &_pool[id - 1].d.text;
    e->base.type           = SS_TUI_ETYPE_TEXT;
    e->base.slide_id       = slide_id;
    e->base.update_needed  = 0;
    e->base.pos.col        = x;
    e->base.pos.row        = y;
    _scopy(e->text, text, SS_TUI_TEXT_MAX_LEN);
    return id;
}

void ss_tui_text_set(int text_id, const char *text)
{
    _slot_data_t *s = _get_slot(text_id);
    if (!s || s->base.type != SS_TUI_ETYPE_TEXT) return;
    _scopy(s->text.text, text, SS_TUI_TEXT_MAX_LEN);
    s->base.update_needed = 0;
}


int ss_tui_line_create(uint8_t slide_id,
                       uint16_t x1, uint16_t y1,
                       uint16_t x2, uint16_t y2)
{
    int id = _alloc_slot();
    if (id == SS_TUI_INVALID_ID) return SS_TUI_INVALID_ID;

    _elem_line_t *e        = &_pool[id - 1].d.line;
    e->base.type           = SS_TUI_ETYPE_LINE;
    e->base.slide_id       = slide_id;
    e->base.update_needed  = 0;
    e->base.pos.col        = x1;
    e->base.pos.row        = y1;
    e->end.col             = x2;
    e->end.row             = y2;
    return id;
}


void ss_tui_start(uint8_t slide_id)
{
    ss_tui_clear();
    ss_tui_update(slide_id);
}

void ss_tui_update(uint8_t slide_id)
{
    static uint8_t _update_count = 0;
    if (++_update_count >= 20u) {
        _update_count = 0;
        ss_tui_erase();
    }

    for (int i = 0; i < SS_TUI_MAX_ELEMENTS; i++) {
        if (!_pool[i].in_use)              continue;
        _slot_data_t *s = &_pool[i].d;
        if (s->base.slide_id != slide_id)  continue;
        if (s->base.update_needed)         continue; /* already clean */

        switch (s->base.type) {
            case SS_TUI_ETYPE_TEXT:     _draw_text_elem(&s->text);   break;
            case SS_TUI_ETYPE_LINE:     _draw_line_elem(&s->line);   break;
            case SS_TUI_ETYPE_KV:       _draw_kv_elem(&s->kv);       break;
            case SS_TUI_ETYPE_TEXT_BOX: _draw_box_elem(&s->box);     break;
            case SS_TUI_ETYPE_INPUT:    _draw_input_elem(&s->input); break;
            default: break;
        }
        s->base.update_needed = 1;
    }

    /* Cursor positionieren wenn ein Input-Widget editiert wird */
    if (_focused_input_id != SS_TUI_INVALID_ID) {
        _slot_data_t *cs = _get_slot(_focused_input_id);
        if (cs && cs->base.type == SS_TUI_ETYPE_INPUT &&
            cs->input.state == _INPUT_EDITING) {
            const _elem_input_t *inp = &cs->input;
            uint16_t text_area   = inp->width > 1u ? (uint16_t)(inp->width - 1u) : 0u;
            uint16_t display_len = inp->len < text_area ? (uint16_t)inp->len : text_area;
            ss_tui_cursor_show();
            ss_tui_goto(inp->base.pos.row + 1u,
                        inp->base.pos.col + 2u + display_len);
        } else {
            ss_tui_cursor_hide();
        }
    } else {
        ss_tui_cursor_hide();
    }

    ss_uart_flush(_iface);
}

void ss_tui_slide_free(uint8_t slide_id)
{
    for (int i = 0; i < SS_TUI_MAX_ELEMENTS; i++) {
        if (_pool[i].in_use && _pool[i].d.base.slide_id == slide_id) {
            if ((i + 1) == _focused_input_id) {
                _focused_input_id = SS_TUI_INVALID_ID;
                ss_tui_cursor_hide();
            }
            _pool[i].in_use = 0;
        }
    }
}

void ss_tui_erase(void)
{
    ss_tui_clear(); /* ESC[2J + ESC[H */

    for (int i = 0; i < SS_TUI_MAX_ELEMENTS; i++) {
        if (_pool[i].in_use) {
            _pool[i].d.base.update_needed = 0;
        }
    }
}


/* ════════════════════════════════════════════
 * Input Widget
 * ════════════════════════════════════════════ */

int ss_tui_input_create(uint8_t slide_id, uint16_t x, uint16_t y,
                        uint16_t width, const char *name, bool activate)
{
    if (width < 3u) width = 3u;

    int id = _alloc_slot();
    if (id == SS_TUI_INVALID_ID) return SS_TUI_INVALID_ID;

    _elem_input_t *e      = &_pool[id - 1].d.input;
    e->base.type          = SS_TUI_ETYPE_INPUT;
    e->base.slide_id      = slide_id;
    e->base.update_needed = 0;
    e->base.pos.col       = x;
    e->base.pos.row       = y;
    e->width              = width;
    e->len                = 0;
    e->buf[0]             = '\0';
    e->state              = _INPUT_IDLE;
    e->activated          = 0;
    _scopy(e->name, name ? name : "", SS_TUI_NAME_MAX_LEN);

    if (activate) ss_tui_input_activate(id);
    return id;
}

void ss_tui_input_activate(int id)
{
    _slot_data_t *s = _get_slot(id);
    if (!s || s->base.type != SS_TUI_ETYPE_INPUT) return;

    /* Falls ein Widget von einer anderen Slide fokussiert ist → ablösen */
    if (_focused_input_id != SS_TUI_INVALID_ID && _focused_input_id != id) {
        _slot_data_t *old = _get_slot(_focused_input_id);
        if (old && old->base.type == SS_TUI_ETYPE_INPUT &&
            old->base.slide_id != s->base.slide_id) {
            if (old->input.state == _INPUT_EDITING) ss_tui_cursor_hide();
            old->input.activated    = 0;
            old->input.state        = _INPUT_IDLE;
            old->base.update_needed = 0;
            _focused_input_id       = SS_TUI_INVALID_ID;
        }
    }

    s->input.activated    = 1;
    s->base.update_needed = 0;

    /* Fokus nur setzen wenn noch kein Widget dieser Slide fokussiert ist */
    if (_focused_input_id == SS_TUI_INVALID_ID) {
        s->input.state    = _INPUT_FOCUSED;
        _focused_input_id = id;
    }
}

void ss_tui_input_deactivate(int id)
{
    _slot_data_t *s = _get_slot(id);
    if (!s || s->base.type != SS_TUI_ETYPE_INPUT) return;

    if (s->input.state == _INPUT_EDITING) ss_tui_cursor_hide();
    s->input.activated    = 0;
    s->input.state        = _INPUT_IDLE;
    s->base.update_needed = 0;

    if (_focused_input_id != id) return;

    /* Fokus auf nächstes aktiviertes Widget derselben Slide übertragen */
    _focused_input_id = SS_TUI_INVALID_ID;
    for (int i = 0; i < SS_TUI_MAX_ELEMENTS; i++) {
        if (!_pool[i].in_use)                            continue;
        if (_pool[i].d.base.type != SS_TUI_ETYPE_INPUT)  continue;
        if (!_pool[i].d.input.activated)                 continue;
        if (_pool[i].d.base.slide_id != s->base.slide_id) continue;
        _pool[i].d.input.state        = _INPUT_FOCUSED;
        _pool[i].d.base.update_needed = 0;
        _focused_input_id             = i + 1;
        break;
    }
}

void ss_tui_input_feed(uint8_t byte)
{
    /* TAB: innerhalb der Slide zum nächsten Widget springen */
    if (byte == 0x09u) {
        if (_focused_input_id != SS_TUI_INVALID_ID) {
            _slot_data_t *s = _get_slot(_focused_input_id);
            if (!s || s->input.state != _INPUT_EDITING) _cycle_focus();
        }
        return;
    }

    if (_focused_input_id == SS_TUI_INVALID_ID) return;
    _slot_data_t *s = _get_slot(_focused_input_id);
    if (!s || s->base.type != SS_TUI_ETYPE_INPUT) return;
    _elem_input_t *inp = &s->input;

    /* ENTER: Edit-Modus umschalten */
    if (byte == 0x0Du || byte == 0x0Au) {
        if (inp->state == _INPUT_FOCUSED) {
            inp->state = _INPUT_EDITING;
            ss_tui_cursor_show();
        } else if (inp->state == _INPUT_EDITING) {
            inp->state = _INPUT_FOCUSED;
            ss_tui_cursor_hide();
        }
        s->base.update_needed = 0;
        return;
    }

    if (inp->state != _INPUT_EDITING) return;

    /* BACKSPACE (0x7F und 0x08) */
    if (byte == 0x7Fu || byte == 0x08u) {
        if (inp->len > 0u) {
            inp->len--;
            inp->buf[inp->len] = '\0';
            s->base.update_needed = 0;
        }
        return;
    }

    /* Druckbare Zeichen */
    if (byte >= 0x20u && byte <= 0x7Eu) {
        if (inp->len < (uint8_t)(SS_TUI_INPUT_MAX_LEN - 1)) {
            inp->buf[inp->len++] = (char)byte;
            inp->buf[inp->len]   = '\0';
            s->base.update_needed = 0;
        }
        return;
    }
}


bool ss_tui_input_get_str(int id, char *buf, uint8_t max_len)
{
    _slot_data_t *s = _get_slot(id);
    if (!s || s->base.type != SS_TUI_ETYPE_INPUT) SS_ERROR("invalid input");
    if (s->input.state == _INPUT_EDITING)          SS_ERROR("invalid input");
    _scopy(buf, s->input.buf, max_len);
    return true;
}

bool ss_tui_input_get_int(int id, int32_t *value)
{
    _slot_data_t *s = _get_slot(id);
    if (!s || s->base.type != SS_TUI_ETYPE_INPUT) SS_ERROR("invalid input");
    if (s->input.state == _INPUT_EDITING)          SS_ERROR("invalid input");
    return _parse_int(s->input.buf, s->input.len, value);
}

bool ss_tui_input_get_float(int id, float *value)
{
    _slot_data_t *s = _get_slot(id);
    if (!s || s->base.type != SS_TUI_ETYPE_INPUT) SS_ERROR("invalid input");
    if (s->input.state == _INPUT_EDITING)          SS_ERROR("invalid input");
    return _parse_float(s->input.buf, s->input.len, value);
}

#endif

