#define USE_PRIVATE
#include "ss_config.h"

#if COMPILE_SS_TUI



#include "ss_tui.h"


#include "ss_uart.h"
#include "ss_rtos.h"



static uint8_t _iface = 0;




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
    char  key[SS_TUI_KEY_MAX_LEN];
    float value;
    int   parent_box_id;
} _elem_kv_t;


typedef struct {
    _elem_base_t base;
    char     name[SS_TUI_NAME_MAX_LEN];
    uint16_t inner_width;                    
    uint8_t  max_key_len;                    
    int      kv_ids[SS_TUI_MAX_KV_PER_BOX];
    uint8_t  kv_count;
} _elem_box_t;


typedef union {
    _elem_base_t base; 
    _elem_text_t text;
    _elem_line_t line;
    _elem_kv_t   kv;
    _elem_box_t  box;
} _slot_data_t;

typedef struct {
    uint8_t      in_use;
    _slot_data_t d;
} _slot_t;


static _slot_t  _pool[SS_TUI_MAX_ELEMENTS];
static uint8_t  _g_font_color = SS_TUI_FG_WHITE;
static uint8_t  _g_bg_color   = SS_TUI_BG_BLUE;
static uint8_t  _g_line_color = SS_TUI_FG_RED;


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
    _g_font_color = SS_TUI_FG_WHITE;
    _g_bg_color   = SS_TUI_BG_BLUE;
    _g_line_color = SS_TUI_FG_RED;
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
    _put_float_value(kv->value);
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



void ss_tui_set_font_color(uint8_t fg)       { _g_font_color = fg; }
void ss_tui_set_background_color(uint8_t bg) { _g_bg_color   = bg; }
void ss_tui_set_line_color(uint8_t fg)       { _g_line_color = fg; }



int ss_tui_text_box_create(uint8_t slide_id,
                           ss_tui_pos_t pos,
                           const char *name)
{
    int id = _alloc_slot();
    if (id == SS_TUI_INVALID_ID) return SS_TUI_INVALID_ID;

    _elem_box_t *b        = &_pool[id - 1].d.box;
    b->base.type          = SS_TUI_ETYPE_TEXT_BOX;
    b->base.slide_id      = slide_id;
    b->base.update_needed = 0;
    b->base.pos           = pos;
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

int ss_tui_text_create(uint8_t slide_id,
                       ss_tui_pos_t pos,
                       const char *text)
{
    int id = _alloc_slot();
    if (id == SS_TUI_INVALID_ID) return SS_TUI_INVALID_ID;

    _elem_text_t *e        = &_pool[id - 1].d.text;
    e->base.type           = SS_TUI_ETYPE_TEXT;
    e->base.slide_id       = slide_id;
    e->base.update_needed  = 0;
    e->base.pos            = pos;
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
                       ss_tui_pos_t start,
                       ss_tui_pos_t end)
{
    int id = _alloc_slot();
    if (id == SS_TUI_INVALID_ID) return SS_TUI_INVALID_ID;

    _elem_line_t *e        = &_pool[id - 1].d.line;
    e->base.type           = SS_TUI_ETYPE_LINE;
    e->base.slide_id       = slide_id;
    e->base.update_needed  = 0;
    e->base.pos            = start;
    e->end                 = end;
    return id;
}


void ss_tui_update(uint8_t slide_id)
{

    for (int i = 0; i < SS_TUI_MAX_ELEMENTS; i++) {
        if (!_pool[i].in_use)              continue;
        _slot_data_t *s = &_pool[i].d;
        if (s->base.slide_id != slide_id)  continue;
        if (s->base.update_needed)         continue; /* already clean */

        switch (s->base.type) {
            case SS_TUI_ETYPE_TEXT:     _draw_text_elem(&s->text); break;
            
            case SS_TUI_ETYPE_LINE:     _draw_line_elem(&s->line); break;
            case SS_TUI_ETYPE_KV:       _draw_kv_elem(&s->kv);    break;
            case SS_TUI_ETYPE_TEXT_BOX: _draw_box_elem(&s->box);   break;
            default: break;
        }
        s->base.update_needed = 1;
    }
    ss_uart_flush(_iface);
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

#endif

