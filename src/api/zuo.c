// MIT License

// Copyright (c) 2017 Vadim Grigoruk @nesbox // grigoruk@gmail.com

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "core/core.h"

#if 1 // defined(TIC_BUILD_WITH_ZUO)

//#define USE_FOREIGN_POINTER

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zuo.h>

static const char* TicCore = "_TIC80";
static const char* TicCoreType = "_TIC80Type";

typedef struct {
    tic_mem* tic;
    zuo_ext_t* module_ht;
} zuo_scheme;

tic_core* getZuoCore(zuo_scheme* sc)
{
    return zuo_c_pointer(zuo_name_to_value(sc, TicCore));
}

static int zuo_list_length(zuo_scheme* sc, zuo_ext_t* lst)
{
    zuo_ext_t* length = zuo_ext_hash_ref(
        sc->module_ht,
        zuo_ext_symbol("length"),
        zuo_ext_false());

    return zuo_ext_integer_value(zuo_ext_apply(zuo_ext_apply(length, lst), zuo_ext_null()));
}

zuo_ext_t* zuo_print(zuo_scheme* sc, zuo_ext_t* args)
{
    //print(text x=0 y=0 color=15 fixed=false scale=1 smallfont=false) -> width
    tic_mem* tic = sc->tic;
    const char* text = zuo_ext_string_ptr(zuo_ext_list_ref(args, 0));
    const s32 x = zuo_ext_integer_value(zuo_ext_list_ref(args, 1));
    const s32 y = zuo_ext_integer_value(zuo_ext_list_ref(args, 2));
    const u8 color = zuo_ext_integer_value(zuo_ext_list_ref(args, 2));
    const bool fixed = zuo_ext_bool_value(zuo_list_ref(args,4));
    const s32 scale = zuo_ext_integer_value(zuo_ext_list_ref(args, 5));
    const bool alt = zuo_ext_bool_value(zuo_list_ref(args,6));
    const s32 result = tic_api_print(tic, text, x, y, color, fixed, scale, alt);
    return zuo_ext_integer(result);
}
zuo_ext_t* zuo_cls(zuo_scheme* sc, zuo_ext_t* args)
{
    // cls(color=0)
    tic_mem* tic = sc->tic;
    const u8 color = zuo_ext_integer_value(zuo_ext_list_ref(args, 0));
    tic_api_cls(tic, color);
    return zuo_ext_void();
}

zuo_ext_t* zuo_pix(zuo_scheme* sc, zuo_ext_t* args)
{
    // pix(x y color)\npix(x y) -> color
    tic_mem* tic = sc->tic;
    const s32 x = zuo_ext_integer_value(zuo_ext_list_ref(args, 0));
    const s32 y = zuo_ext_integer_value(zuo_ext_list_ref(args, 1));

    const int argn = zuo_list_length(sc, args);
    if (argn == 3)
    {
        const u8 color = zuo_ext_integer_value(zuo_ext_list_ref(args, 2));
        tic_api_pix(tic, x, y, color, false);
        return zuo_ext_void();
    }
    else{
        return zuo_ext_integer(sc, tic_api_pix(tic, x, y, 0, true));
    }
}

zuo_ext_t* zuo_line(zuo_scheme* sc, zuo_ext_t* args)
{
    // line(x0 y0 x1 y1 color)
    tic_mem* tic = sc->tic;
    const s32 x0 = zuo_integer(zuo_car(args));
    const s32 y0 = zuo_integer(zuo_cadr(args));
    const s32 x1 = zuo_integer(zuo_caddr(args));
    const s32 y1 = zuo_integer(zuo_cadddr(args));
    const u8 color = zuo_integer(zuo_list_ref(sc, args, 4));
    tic_api_line(tic, x0, y0, x1, y1, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_rect(zuo_scheme* sc, zuo_ext_t* args)
{
    // rect(x y w h color)
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    const s32 w = zuo_integer(zuo_caddr(args));
    const s32 h = zuo_integer(zuo_cadddr(args));
    const u8 color = zuo_integer(zuo_list_ref(sc, args, 4));
    tic_api_rect(tic, x, y, w, h, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_rectb(zuo_scheme* sc, zuo_ext_t* args)
{
    // rectb(x y w h color)
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    const s32 w = zuo_integer(zuo_caddr(args));
    const s32 h = zuo_integer(zuo_cadddr(args));
    const u8 color = zuo_integer(zuo_list_ref(sc, args, 4));
    tic_api_rectb(tic, x, y, w, h, color);
    return zuo_ext_void();
}

void parseTransparentColorsArg(zuo_scheme* sc, zuo_ext_t* colorkey, u8* out_transparent_colors, u8* out_count)
{
    *out_count = 0;
    if (zuo_is_list(sc, colorkey))
    {
        const s32 arg_color_count = zuo_list_length(sc, colorkey);
        const u8 color_count = arg_color_count < TIC_PALETTE_SIZE ? (u8)arg_color_count : TIC_PALETTE_SIZE;
        for (u8 i=0; i<color_count; ++i)
        {
            zuo_ext_t* c = zuo_list_ref(sc, colorkey, i);
            out_transparent_colors[i] = zuo_is_integer(c) ? zuo_integer(c) : 0;
            ++(*out_count);
        }
    }
    else if (zuo_is_integer(colorkey))
    {
        out_transparent_colors[0] = (u8)zuo_integer(colorkey);
        *out_count = 1;
    }
}

zuo_ext_t* zuo_spr(zuo_scheme* sc, zuo_ext_t* args)
{
    // spr(id x y colorkey=-1 scale=1 flip=0 rotate=0 w=1 h=1)
    const int argn      = zuo_list_length(sc, args);
    tic_mem* tic        = (tic_mem*)getZuoCore(sc);
    const s32 id        = zuo_integer(zuo_car(args));
    const s32 x         = zuo_integer(zuo_cadr(args));
    const s32 y         = zuo_integer(zuo_caddr(args));

    static u8 trans_colors[TIC_PALETTE_SIZE];
    u8 trans_count = 0;
    if (argn > 3)
    {
        zuo_ext_t* colorkey = zuo_cadddr(args);
        parseTransparentColorsArg(sc, colorkey, trans_colors, &trans_count);
    }

    const s32 scale     = argn > 4 ? zuo_integer(zuo_list_ref(sc, args, 4)) : 1;
    const s32 flip      = argn > 5 ? zuo_integer(zuo_list_ref(sc, args, 5)) : 0;
    const s32 rotate    = argn > 6 ? zuo_integer(zuo_list_ref(sc, args, 6)) : 0;
    const s32 w         = argn > 7 ? zuo_integer(zuo_list_ref(sc, args, 7)) : 1;
    const s32 h         = argn > 8 ? zuo_integer(zuo_list_ref(sc, args, 8)) : 1;
    tic_api_spr(tic, id, x, y, w, h, trans_colors, trans_count, scale, (tic_flip)flip, (tic_rotate) rotate);
    return zuo_ext_void();
}
zuo_ext_t* zuo_btn(zuo_scheme* sc, zuo_ext_t* args)
{
    // btn(id) -> pressed
    tic_mem* tic = sc->tic;
    const s32 id = zuo_integer(zuo_car(args));
    
    return zuo_make_boolean(sc, tic_api_btn(tic, id));
}
zuo_ext_t* zuo_btnp(zuo_scheme* sc, zuo_ext_t* args)
{
    // btnp(id hold=-1 period=-1) -> pressed
    tic_mem* tic = sc->tic;
    const s32 id = zuo_integer(zuo_car(args));

    const int argn = zuo_list_length(sc, args);
    const s32 hold = argn > 1 ? zuo_integer(zuo_cadr(args)) : -1;
    const s32 period = argn > 2 ? zuo_integer(zuo_caddr(args)) : -1;
    
    return zuo_make_boolean(sc, tic_api_btnp(tic, id, hold, period));
}

u8 get_note_base(char c) {
    switch (c) {
    case 'C': return 0;
    case 'D': return 2;
    case 'E': return 4;
    case 'F': return 5;
    case 'G': return 7;
    case 'A': return 9;
    case 'B': return 11;
    default:  return 255;
    }
}

u8 get_note_modif(char c) {
    switch (c) {
    case '-': return 0;
    case '#': return 1;
    default:  return 255;
    }
}

u8 get_note_octave(char c) {
    if (c >= '0' && c <= '8')
        return c - '0';
    else
        return 255;
}

zuo_ext_t* zuo_sfx(zuo_scheme* sc, zuo_ext_t* args)
{
    // sfx(id note=-1 duration=-1 channel=0 volume=15 speed=0)
    tic_mem* tic = sc->tic;
    const s32 id = zuo_integer(zuo_car(args));
    
    const int argn = zuo_list_length(sc, args);
    int note = -1;
    int octave = -1;
    if (argn > 1) {
        zuo_ext_t* note_ptr = zuo_cadr(args);
        if (zuo_is_integer(note_ptr)) {
            const s32 raw_note = zuo_integer(note_ptr);
            if (raw_note >= 0 || raw_note <= 95) {
                note = raw_note % 12;
                octave = raw_note / 12;
            }
            /* else { */
            /*     char buffer[100]; */
            /*     snprintf(buffer, 99, "Invalid sfx note given: %d\n", raw_note); */
            /*     tic->data->error(tic->data->data, buffer); */
            /* } */
        } else if (zuo_is_string(note_ptr)) {
            const char* note_str = zuo_string(note_ptr);
            const u8 len = zuo_string_length(note_ptr);
            if (len == 3) {
                const u8 modif = get_note_modif(note_str[1]);
                note = get_note_base(note_str[0]);
                octave = get_note_octave(note_str[2]);
                if (note < 255 || modif < 255 || octave < 255) {
                    note = note + modif;
                } else {
                    note = octave = 255;
                }
            }
            /* if (note == 255 || octave == 255) { */
            /*     char buffer[100]; */
            /*     snprintf(buffer, 99, "Invalid sfx note given: %s\n", note_str); */
            /*     tic->data->error(tic->data->data, buffer); */
            /* } */
        }
    }
    
    const s32 duration = argn > 2 ? zuo_integer(zuo_caddr(args)) : -1;
    const s32 channel = argn > 3 ? zuo_integer(zuo_cadddr(args)) : 0;

    s32 volumes[TIC80_SAMPLE_CHANNELS] = {MAX_VOLUME, MAX_VOLUME};
    if (argn > 4) {
        zuo_ext_t* volume_arg = zuo_list_ref(sc, args, 4);
        if (zuo_is_integer(volume_arg)) {
            volumes[0] = volumes[1] = zuo_integer(volume_arg) & 0xF;
        } else if (zuo_is_list(sc, volume_arg) && zuo_list_length(sc, volume_arg) == 2) {
            volumes[0] = zuo_integer(zuo_car(volume_arg)) & 0xF;
            volumes[1] = zuo_integer(zuo_cadr(volume_arg)) & 0xF;
        }
    }
    const s32 speed = argn > 5 ? zuo_integer(zuo_list_ref(sc, args, 5)) : 0;

    tic_api_sfx(tic, id, note, octave, duration, channel, volumes[0], volumes[1], speed);
    return zuo_ext_void();
}

typedef struct
{
    zuo_scheme* sc;
    zuo_ext_t* callback;
} RemapData;

static void remapCallback(void* data, s32 x, s32 y, RemapResult* result)
{
    RemapData* remap = (RemapData*)data;
    zuo_scheme* sc = remap->sc;

    // (callback index x y) -> (list index flip rotate)
    zuo_ext_t* callbackResult = zuo_call(sc, remap->callback,
                                        zuo_cons(sc, zuo_ext_integer(sc, result->index),
                                                zuo_cons(sc, zuo_ext_integer(sc, x),
                                                        zuo_cons(sc, zuo_ext_integer(sc, y),
                                                                zuo_ext_void()))));

    if (zuo_is_list(sc, callbackResult) && zuo_list_length(sc, callbackResult) == 3)
    {
        result->index = zuo_integer(zuo_car(callbackResult));
        result->flip = (tic_flip)zuo_integer(zuo_cadr(callbackResult));
        result->rotate = (tic_rotate)zuo_integer(zuo_caddr(callbackResult));
    }
}

zuo_ext_t* zuo_map(zuo_scheme* sc, zuo_ext_t* args)
{
    // map(x=0 y=0 w=30 h=17 sx=0 sy=0 colorkey=-1 scale=1 remap=nil)
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    const s32 w = zuo_integer(zuo_caddr(args));
    const s32 h = zuo_integer(zuo_cadddr(args));
    const s32 sx = zuo_integer(zuo_list_ref(sc, args, 4));
    const s32 sy = zuo_integer(zuo_list_ref(sc, args, 5));

    const int argn = zuo_list_length(sc, args);

    static u8 trans_colors[TIC_PALETTE_SIZE];
    u8 trans_count = 0;
    if (argn > 6) {
        zuo_ext_t* colorkey = zuo_list_ref(sc, args, 6);
        parseTransparentColorsArg(sc, colorkey, trans_colors, &trans_count);
    }

    const s32 scale = argn > 7 ? zuo_integer(zuo_list_ref(sc, args, 7)) : 1;

    RemapFunc remap = NULL;
    RemapData data;
    if (argn > 8)
    {
        remap = remapCallback;
        data.sc = sc;
        data.callback = zuo_list_ref(sc, args, 8);
    }
    tic_api_map(tic, x, y, w, h, sx, sy, trans_colors, trans_count, scale, remap, &data);
    return zuo_ext_void();
}
zuo_ext_t* zuo_mget(zuo_scheme* sc, zuo_ext_t* args)
{
    // mget(x y) -> tile_id
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    return zuo_ext_integer(sc, tic_api_mget(tic, x, y));
}
zuo_ext_t* zuo_mset(zuo_scheme* sc, zuo_ext_t* args)
{
    // mset(x y tile_id)
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    const u8 tile_id = zuo_integer(zuo_caddr(args));
    tic_api_mset(tic, x, y, tile_id);
    return zuo_ext_void();
}
zuo_ext_t* zuo_peek(zuo_scheme* sc, zuo_ext_t* args)
{
    // peek(addr bits=8) -> value
    tic_mem* tic = sc->tic;
    const s32 addr = zuo_integer(zuo_car(args));
    const int argn = zuo_list_length(sc, args);
    const s32 bits = argn > 1 ? zuo_integer(zuo_cadr(args)) : 8;
    return zuo_ext_integer(sc, tic_api_peek(tic, addr, bits));
}
zuo_ext_t* zuo_poke(zuo_scheme* sc, zuo_ext_t* args)
{
    // poke(addr value bits=8)
    tic_mem* tic = sc->tic;
    const s32 addr = zuo_integer(zuo_car(args));
    const s32 value = zuo_integer(zuo_cadr(args));
    const int argn = zuo_list_length(sc, args);
    const s32 bits = argn > 2 ? zuo_integer(zuo_caddr(args)) : 8;
    tic_api_poke(tic, addr, value, bits);
    return zuo_ext_void();
}
zuo_ext_t* zuo_peek1(zuo_scheme* sc, zuo_ext_t* args)
{
    // peek1(addr) -> value
    tic_mem* tic = sc->tic;
    const s32 addr = zuo_integer(zuo_car(args));
    return zuo_ext_integer(sc, tic_api_peek1(tic, addr));
}
zuo_ext_t* zuo_poke1(zuo_scheme* sc, zuo_ext_t* args)
{
    // poke1(addr value)
    tic_mem* tic = sc->tic;
    const s32 addr = zuo_integer(zuo_car(args));
    const s32 value = zuo_integer(zuo_cadr(args));
    tic_api_poke1(tic, addr, value);
    return zuo_ext_void();
}
zuo_ext_t* zuo_peek2(zuo_scheme* sc, zuo_ext_t* args)
{
    // peek2(addr) -> value
    tic_mem* tic = sc->tic;
    const s32 addr = zuo_integer(zuo_car(args));
    return zuo_ext_integer(sc, tic_api_peek2(tic, addr));
}
zuo_ext_t* zuo_poke2(zuo_scheme* sc, zuo_ext_t* args)
{
    // poke2(addr value)
    tic_mem* tic = sc->tic;
    const s32 addr = zuo_integer(zuo_car(args));
    const s32 value = zuo_integer(zuo_cadr(args));
    tic_api_poke2(tic, addr, value);
    return zuo_ext_void();
}
zuo_ext_t* zuo_peek4(zuo_scheme* sc, zuo_ext_t* args)
{
    // peek4(addr) -> value
    tic_mem* tic = sc->tic;
    const s32 addr = zuo_integer(zuo_car(args));
    return zuo_ext_integer(sc, tic_api_peek4(tic, addr));
}
zuo_ext_t* zuo_poke4(zuo_scheme* sc, zuo_ext_t* args)
{
    // poke4(addr value)
    tic_mem* tic = sc->tic;
    const s32 addr = zuo_integer(zuo_car(args));
    const s32 value = zuo_integer(zuo_cadr(args));
    tic_api_poke4(tic, addr, value);
    return zuo_ext_void();
}
zuo_ext_t* zuo_memcpy(zuo_scheme* sc, zuo_ext_t* args)
{
    // memcpy(dest source size)
    tic_mem* tic = sc->tic;
    const s32 dest = zuo_integer(zuo_car(args));
    const s32 source = zuo_integer(zuo_cadr(args));
    const s32 size = zuo_integer(zuo_caddr(args));
    tic_api_memcpy(tic, dest, source, size);
    return zuo_ext_void();
}
zuo_ext_t* zuo_memset(zuo_scheme* sc, zuo_ext_t* args)
{
    // memset(dest value size)
    tic_mem* tic = sc->tic;
    const s32 dest = zuo_integer(zuo_car(args));
    const s32 value = zuo_integer(zuo_cadr(args));
    const s32 size = zuo_integer(zuo_caddr(args));
    tic_api_memset(tic, dest, value, size);
    return zuo_ext_void();
}
zuo_ext_t* zuo_trace(zuo_scheme* sc, zuo_ext_t* args)
{
    // trace(message color=15)
    tic_mem* tic = sc->tic;
    const char* msg = zuo_string(zuo_car(args));
    const int argn = zuo_list_length(sc, args);
    const s32 color = argn > 1 ? zuo_integer(zuo_cadr(args)) : 15;
    tic_api_trace(tic, msg, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_pmem(zuo_scheme* sc, zuo_ext_t* args)
{
    // pmem(index value)
    // pmem(index) -> value
    tic_mem* tic = sc->tic;
    const s32 index = zuo_integer(zuo_car(args));
    const int argn = zuo_list_length(sc, args);
    s32 value = 0;
    bool shouldSet = false;
    if (argn > 1)
    {
        value = zuo_integer(zuo_cadr(args));
        shouldSet = true;
    }
    return zuo_ext_integer(sc, (s32)tic_api_pmem(tic, index, value, shouldSet));
}
zuo_ext_t* zuo_time(zuo_scheme* sc, zuo_ext_t* args)
{
    // time() -> ticks
    tic_mem* tic = sc->tic;
    return zuo_make_real(sc, tic_api_time(tic));
}
zuo_ext_t* zuo_tstamp(zuo_scheme* sc, zuo_ext_t* args)
{
    // tstamp() -> timestamp
    tic_mem* tic = sc->tic;
    return zuo_ext_integer(sc, tic_api_tstamp(tic));
}
zuo_ext_t* zuo_exit(zuo_scheme* sc, zuo_ext_t* args)
{
    // exit()
    tic_mem* tic = sc->tic;
    tic_api_exit(tic);
    return zuo_ext_void();
}
zuo_ext_t* zuo_font(zuo_scheme* sc, zuo_ext_t* args)
{
    // font(text x y chromakey char_width char_height fixed=false scale=1 alt=false) -> width
    tic_mem* tic = sc->tic;
    const char* text = zuo_string(zuo_car(args));
    const s32 x = zuo_integer(zuo_cadr(args));
    const s32 y = zuo_integer(zuo_caddr(args));

    static u8 trans_colors[TIC_PALETTE_SIZE];
    u8 trans_count = 0;
    zuo_ext_t* colorkey = zuo_cadddr(args);
    parseTransparentColorsArg(sc, colorkey, trans_colors, &trans_count);

    const s32 w = zuo_integer(zuo_list_ref(sc, args, 4));
    const s32 h = zuo_integer(zuo_list_ref(sc, args, 5));
    const int argn = zuo_list_length(sc, args);
    const s32 fixed = argn > 6 ? zuo_boolean(sc, zuo_list_ref(sc, args, 6)) : false;
    const s32 scale = argn > 7 ? zuo_integer(zuo_list_ref(sc, args, 7)) : 1;
    const s32 alt = argn > 8 ? zuo_boolean(sc, zuo_list_ref(sc, args, 8)) : false;

    return zuo_ext_integer(sc, tic_api_font(tic, text, x, y, trans_colors, trans_count, w, h, fixed, scale, alt));
}
zuo_ext_t* zuo_mouse(zuo_scheme* sc, zuo_ext_t* args)
{
    // mouse() -> x y left middle right scrollx scrolly
    tic_mem* tic = sc->tic;
    const tic_point point = tic_api_mouse(tic);
    const tic80_mouse* mouse = &((tic_core*)tic)->memory.ram->input.mouse;
    
    return
        zuo_cons(sc, zuo_ext_integer(sc, point.x),
                zuo_cons(sc, zuo_ext_integer(sc, point.y),
                        zuo_cons(sc, zuo_ext_integer(sc, mouse->left),
                                zuo_cons(sc, zuo_ext_integer(sc, mouse->middle),
                                        zuo_cons(sc, zuo_ext_integer(sc, mouse->right),
                                                zuo_cons(sc, zuo_ext_integer(sc, mouse->scrollx),
                                                        zuo_cons(sc, zuo_ext_integer(sc, mouse->scrolly),
                                                                zuo_ext_void())))))));
}
zuo_ext_t* zuo_circ(zuo_scheme* sc, zuo_ext_t* args)
{
    // circ(x y radius color)
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    const s32 radius = zuo_integer(zuo_caddr(args));
    const s32 color = zuo_integer(zuo_cadddr(args));
    tic_api_circ(tic, x, y, radius, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_circb(zuo_scheme* sc, zuo_ext_t* args)
{
    // circb(x y radius color)
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    const s32 radius = zuo_integer(zuo_caddr(args));
    const s32 color = zuo_integer(zuo_cadddr(args));
    tic_api_circb(tic, x, y, radius, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_elli(zuo_scheme* sc, zuo_ext_t* args)
{
    // elli(x y a b color)
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    const s32 a = zuo_integer(zuo_caddr(args));
    const s32 b = zuo_integer(zuo_cadddr(args));
    const s32 color = zuo_integer(zuo_list_ref(sc, args, 4));
    tic_api_elli(tic, x, y, a, b, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_ellib(zuo_scheme* sc, zuo_ext_t* args)
{
    // ellib(x y a b color)
    tic_mem* tic = sc->tic;
    const s32 x = zuo_integer(zuo_car(args));
    const s32 y = zuo_integer(zuo_cadr(args));
    const s32 a = zuo_integer(zuo_caddr(args));
    const s32 b = zuo_integer(zuo_cadddr(args));
    const s32 color = zuo_integer(zuo_list_ref(sc, args, 4));
    tic_api_ellib(tic, x, y, a, b, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_tri(zuo_scheme* sc, zuo_ext_t* args)
{
    // tri(x1 y1 x2 y2 x3 y3 color)
    tic_mem* tic = sc->tic;
    const s32 x1 = zuo_integer(zuo_car(args));
    const s32 y1 = zuo_integer(zuo_cadr(args));
    const s32 x2 = zuo_integer(zuo_caddr(args));
    const s32 y2 = zuo_integer(zuo_cadddr(args));
    const s32 x3 = zuo_integer(zuo_list_ref(sc, args, 4));
    const s32 y3 = zuo_integer(zuo_list_ref(sc, args, 5));
    const s32 color = zuo_integer(zuo_list_ref(sc, args, 6));
    tic_api_tri(tic, x1, y1, x2, y2, x3, y3, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_trib(zuo_scheme* sc, zuo_ext_t* args)
{
    // trib(x1 y1 x2 y2 x3 y3 color)
    tic_mem* tic = sc->tic;
    const s32 x1 = zuo_integer(zuo_car(args));
    const s32 y1 = zuo_integer(zuo_cadr(args));
    const s32 x2 = zuo_integer(zuo_caddr(args));
    const s32 y2 = zuo_integer(zuo_cadddr(args));
    const s32 x3 = zuo_integer(zuo_list_ref(sc, args, 4));
    const s32 y3 = zuo_integer(zuo_list_ref(sc, args, 5));
    const s32 color = zuo_integer(zuo_list_ref(sc, args, 6));
    tic_api_trib(tic, x1, y1, x2, y2, x3, y3, color);
    return zuo_ext_void();
}
zuo_ext_t* zuo_ttri(zuo_scheme* sc, zuo_ext_t* args)
{
    // ttri(x1 y1 x2 y2 x3 y3 u1 v1 u2 v2 u3 v3 texsrc=0 chromakey=-1 z1=0 z2=0 z3=0)
    tic_mem* tic = sc->tic;
    const s32 x1 = zuo_integer(zuo_car(args));
    const s32 y1 = zuo_integer(zuo_cadr(args));
    const s32 x2 = zuo_integer(zuo_caddr(args));
    const s32 y2 = zuo_integer(zuo_cadddr(args));
    const s32 x3 = zuo_integer(zuo_list_ref(sc, args, 4));
    const s32 y3 = zuo_integer(zuo_list_ref(sc, args, 5));
    const s32 u1 = zuo_integer(zuo_list_ref(sc, args, 6));
    const s32 v1 = zuo_integer(zuo_list_ref(sc, args, 7));
    const s32 u2 = zuo_integer(zuo_list_ref(sc, args, 8));
    const s32 v2 = zuo_integer(zuo_list_ref(sc, args, 9));
    const s32 u3 = zuo_integer(zuo_list_ref(sc, args, 10));
    const s32 v3 = zuo_integer(zuo_list_ref(sc, args, 11));

    const int argn = zuo_list_length(sc, args);
    const tic_texture_src texsrc = (tic_texture_src)(argn > 12 ? zuo_integer(zuo_list_ref(sc, args, 12)) : 0);
    
    static u8 trans_colors[TIC_PALETTE_SIZE];
    u8 trans_count = 0;

    if (argn > 13)
    {
        zuo_ext_t* colorkey = zuo_list_ref(sc, args, 13);
        parseTransparentColorsArg(sc, colorkey, trans_colors, &trans_count);
    }

    bool depth = argn > 14 ? true : false;
    const s32 z1 = argn > 14 ? zuo_integer(zuo_list_ref(sc, args, 14)) : 0;
    const s32 z2 = argn > 15 ? zuo_integer(zuo_list_ref(sc, args, 15)) : 0;
    const s32 z3 = argn > 16 ? zuo_integer(zuo_list_ref(sc, args, 16)) : 0;

    tic_api_ttri(tic, x1, y1, x2, y2, x3, y3, u1, v1, u2, v2, u3, v3, texsrc, trans_colors, trans_count, z1, z2, z3, depth);
    return zuo_ext_void();
}
zuo_ext_t* zuo_clip(zuo_scheme* sc, zuo_ext_t* args)
{
    // clip(x y width height)
    // clip()
    tic_mem* tic = sc->tic;
    const int argn = zuo_list_length(sc, args);
    if (argn != 4) {
        tic_api_clip(tic, 0, 0, TIC80_WIDTH, TIC80_HEIGHT);
    } else {
        const s32 x = zuo_integer(zuo_car(args));
        const s32 y = zuo_integer(zuo_cadr(args));
        const s32 w = zuo_integer(zuo_caddr(args));
        const s32 h = zuo_integer(zuo_cadddr(args));
        tic_api_clip(tic, x, y, w, h);
    }
    return zuo_ext_void();
}
zuo_ext_t* zuo_music(zuo_scheme* sc, zuo_ext_t* args)
{
    // music(track=-1 frame=-1 row=-1 loop=true sustain=false tempo=-1 speed=-1)
    tic_mem* tic = sc->tic;
    const int argn = zuo_list_length(sc, args);
    const s32 track = argn > 0 ? zuo_integer(zuo_car(args)) : -1;
    const s32 frame = argn > 1 ? zuo_integer(zuo_cadr(args)) : -1;
    const s32 row = argn > 2 ? zuo_integer(zuo_caddr(args)) : -1;
    const bool loop = argn > 3 ? zuo_boolean(sc, zuo_cadddr(args)) : true;
    const bool sustain = argn > 4 ? zuo_boolean(sc, zuo_list_ref(sc, args, 4)) : false;
    const s32 tempo = argn > 5 ? zuo_integer(zuo_list_ref(sc, args, 5)) : -1;
    const s32 speed = argn > 6 ? zuo_integer(zuo_list_ref(sc, args, 6)) : -1;
    tic_api_music(tic, track, frame, row, loop, sustain, tempo, speed);
    return zuo_ext_void();
}
zuo_ext_t* zuo_sync(zuo_scheme* sc, zuo_ext_t* args)
{
    // sync(mask=0 bank=0 tocart=false)
    tic_mem* tic = sc->tic;
    const int argn = zuo_list_length(sc, args);
    const u32 mask = argn > 0 ? (u32)zuo_integer(zuo_car(args)) : 0;
    const s32 bank = argn > 1 ? zuo_integer(zuo_cadr(args)) : 0;
    const bool tocart = argn > 2 ? zuo_boolean(sc, zuo_caddr(args)) : false;
    tic_api_sync(tic, mask, bank, tocart);
    return zuo_ext_void();
}
zuo_ext_t* zuo_vbank(zuo_scheme* sc, zuo_ext_t* args)
{
    // vbank(bank) -> prev
    // vbank() -> prev
    tic_mem* tic = sc->tic;
    const int argn = zuo_list_length(sc, args);

    const s32 prev = ((tic_core*)tic)->state.vbank.id;
    if (argn == 1) {
        const s32 bank = zuo_integer(zuo_car(args));
        tic_api_vbank(tic, bank);
    }
    return zuo_ext_integer(sc, prev);
}
zuo_ext_t* zuo_reset(zuo_scheme* sc, zuo_ext_t* args)
{
    // reset()
    tic_mem* tic = sc->tic;
    tic_api_reset(tic);
    return zuo_ext_void();
}
zuo_ext_t* zuo_key(zuo_scheme* sc, zuo_ext_t* args)
{
    //key(code=-1) -> pressed
    tic_mem* tic = sc->tic;
    const int argn = zuo_list_length(sc, args);
    const tic_key code = argn > 0 ? zuo_integer(zuo_car(args)) : -1;
    return zuo_make_boolean(sc, tic_api_key(tic, code));
}
zuo_ext_t* zuo_keyp(zuo_scheme* sc, zuo_ext_t* args)
{
    // keyp(code=-1 hold=-1 period=-1) -> pressed
    tic_mem* tic = sc->tic;
    const int argn = zuo_list_length(sc, args);
    const tic_key code = argn > 0 ? zuo_integer(zuo_car(args)) : -1;
    const s32 hold = argn > 1 ? zuo_integer(zuo_cadr(args)) : -1;
    const s32 period = argn > 2 ? zuo_integer(zuo_caddr(args)) : -1;
    return zuo_make_boolean(sc, tic_api_keyp(tic, code, hold, period));
}
zuo_ext_t* zuo_fget(zuo_scheme* sc, zuo_ext_t* args)
{
    // fget(sprite_id flag) -> bool
    tic_mem* tic = sc->tic;
    const s32 sprite_id = zuo_integer(zuo_car(args));
    const u8 flag = zuo_integer(zuo_cadr(args));
    return zuo_make_boolean(sc, tic_api_fget(tic, sprite_id, flag));
}
zuo_ext_t* zuo_fset(zuo_scheme* sc, zuo_ext_t* args)
{
    // fset(sprite_id flag bool)
    tic_mem* tic = sc->tic;
    const s32 sprite_id = zuo_integer(zuo_car(args));
    const u8 flag = zuo_integer(zuo_cadr(args));
    const bool val = zuo_boolean(sc, zuo_caddr(args));
    tic_api_fset(tic, sprite_id, flag, val);
    return zuo_ext_void(); 
}


static void closeZuo(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;

    if(core->currentVM)
    {
        zuo_free(core->currentVM);
        core->currentVM = NULL;
    }
}

zuo_ext_t* zuo_error_handler(zuo_scheme* sc, zuo_ext_t* args)
{
    tic_core* tic = getZuoCore(sc);
    if (tic->data) {
        tic->data->error(tic->data->data, zuo_string(zuo_car(args)));
    }
    return zuo_ext_void();
}

static const char* ticFnName = "TIC";

static bool initZuo(tic_mem* tic, const char* code)
{
    tic_core* core = (tic_core*)tic;
    closeZuo(tic);

    zuo_ext_primitive_init();
    
    // add primitives
    zuo_scheme* sc = core->currentVM;

#define API_FUNC_DEF(name, desc, helpstr, count, reqcount, ...) \
    {zuo_ ## name, desc  "\n" helpstr, 1<<count & ((-1>>(32-(reqcount-count)))<<count), "t80::" #name},
    
    static const struct {
        zuo_ext_primitive_t func;
        const char* helpstr;
        uint32_t arity_mask;
        const char* name;
    } ApiItems[] = {TIC_API_LIST(API_FUNC_DEF)};
    
#undef API_FUNC_DEF

    for (s32 i = 0; i < COUNT_OF(ApiItems); i++) 
    {
        zuo_ext_add_primitive(ApiItems[i].func, ApiItems[i].arity_mask, ApiItems[i].name);
    }

    zuo_ext_image_init(NULL);

    zuo_ext_runtime_init(
        zuo_ext_false,
        zuo_ext_empty_hash());

    zuo_ext_t* *ht = zuo_ext_eval_module(zuo_ext_symbol("game"), code, strlen(code));


    // zuo_scheme* sc = core->currentVM = zuo_init();
    // initAPI(core);

    // zuo_define_function(sc, "__TIC_ErrorHandler", zuo_error_handler, 1, 0, 0, NULL);
    // zuo_eval_c_string(sc, "(set! (hook-functions *error-hook*)                    \n\
    //                         (list (lambda (hook)                                 \n\
    //                                 (__TIC_ErrorHandler                          \n\
    //                                   (format #f \"~s: ~a\n--STACKTRACE--\n~a\" ((owlet) 'error-type) (apply format #f (hook 'data)) (stacktrace)))   \n\
    //                                 (set! (hook 'result) #f))))");
    // zuo_eval_c_string(sc, defstructStr);

    // zuo_define_variable(sc, TicCore, zuo_make_c_pointer(sc, core));
    // zuo_load_c_string(sc, code, strlen(code));

    const bool isTicDefined = zuo_is_defined(sc, ticFnName);
    if (!isTicDefined) {
        if (core->data) {
            core->data->error(core->data->data, "TIC function is not defined");
        }
    }

    return true;
}

static void callZuoTick(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;
    zuo_scheme* sc = core->currentVM;

    const bool isTicDefined = zuo_is_defined(sc, ticFnName);
    if (isTicDefined) {
        zuo_call(sc, zuo_name_to_value(sc, ticFnName), zuo_ext_void());
    }
}

static void callZuoBoot(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;
    zuo_scheme* sc = core->currentVM;

    static const char* bootFnName = "BOOT";
    const bool isBootDefined = zuo_is_defined(sc, bootFnName);
    if (isBootDefined) {
        zuo_call(sc, zuo_name_to_value(sc, "BOOT"), zuo_ext_void());
    }
}

static void callZuoScanline(tic_mem* tic, s32 row, void* data)
{
    tic_core* core = (tic_core*)tic;
    zuo_scheme* sc = core->currentVM;

    static const char* scnFnName = "SCN";
    const bool isScnDefined = zuo_is_defined(sc, scnFnName);
    if (isScnDefined) {
        zuo_call(sc, zuo_name_to_value(sc, scnFnName), zuo_cons(sc, zuo_ext_integer(sc, row), zuo_ext_void()));
    }
}

static void callZuoBorder(tic_mem* tic, s32 row, void* data)
{
    tic_core* core = (tic_core*)tic;
    zuo_scheme* sc = core->currentVM;

    static const char* bdrFnName = "BDR";
    bool isBdrDefined = zuo_is_defined(sc, bdrFnName);
    if (isBdrDefined) {
        zuo_call(sc, zuo_name_to_value(sc, bdrFnName), zuo_cons(sc, zuo_ext_integer(sc, row), zuo_ext_void()));
    }
}

static void callZuoMenu(tic_mem* tic, s32 index, void* data)
{
    tic_core* core = (tic_core*)tic;
    zuo_scheme* sc = core->currentVM;

    static const char* menuFnName = "MENU";
    bool isMenuDefined = zuo_is_defined(sc, menuFnName);
    if (isMenuDefined) {
        zuo_call(sc, zuo_name_to_value(sc, menuFnName), zuo_cons(sc, zuo_ext_integer(sc, index), zuo_ext_void()));
    }
}

static const char* const ZuoKeywords [] =
{
    "define", "lambda", "begin", "set!", "=", "<", "<=", ">", ">=", "+", "*",
    "/", "'", "`", "`@", "define-macro", "let", "let*", "letrec", "defstruct",
    "if", "cond", "floor", "ceiling", "sin", "cos", "log", "sqrt", "abs"
    "logand", "logior", "logxor", "lognot", "logbit?", "sinh", "cosh", "tanh",
    "asinh", "acosh", "atanh", "nan?", "infinite?", "nan",
    "exp", "expt", "tan", "acos", "asin", "atan", "truncate", "round",
    "exact->inexact", "inexact->exact", "exact?", "inexact?",
    "modulo", "quotient", "remainder", "gcd", "lcm", "and", "or",
    "eq?", "eqv?", "equal?", "equivalent?", "boolean?", "pair?",
    "cons", "car", "cdr", "set-car!", "set-cdr!", "cadr", "cddr",
    "cdar", "caar", "caadr", "caddr", "cadar", "caaar", "cdadr",
    "cdddr", "caddar", "cadaar", "cdaadr", "cdaddr", "cdadar",
    "cdaaar", "cddadr", "cddddr", "cdddar", "cddaar", "list?"
    "proper-list?", "length", "make-list", "list", "reverse",
    "map", "for-each", "list->vector", "list->string", "list-ref", "null?",
    "not", "number->string", "odd?", "even?", "zero?",
    "append", "list-ref", "assoc", "assq", "member", "memq", "memv",
    "tree-memq", "string?", "string-length", "character?",
    "number?", "integer?", "real?", "rational?", "rationalize",
    "numerator", "denominator", "random", "random-state",
    "random-state->list", "random-state?", "complex?",
    "real-part", "imag-part", "number->string", "vector?",
    "vector-length", "float-vector?", "int-vector?",
    "byte-vector?", "vector-ref", "vector-set!", "make-vector",
    "vector-fill!", "vector->list", "make-hash-table",
    "hash-table-ref", "hash-table-set!", "hash-code",
    "hook-functions", "open-input-string", "open-output-string",
    "get-output-string", "read-char", "peek-char", "read",
    "newline", "write-char", "write", "display", "format",
    "syntax?", "symbol?", "symbol->string", "string->symbol",
    "gensym", "keyword?", "string->keyword", "keyword->string",
    "rootlet", "curlet", "outlet", "sublet", "inlet", "let->list",
    "let?", "let-ref", "openlet", "openlet?"
};

static inline bool zuo_isalnum(char c)
{
    return isalnum(c) || c == '_' || c == '-' || c == ':' || c == '#' || c == '!'
        || c == '+' || c == '=' || c == '&' || c == '^' || c == '%' || c == '$' || c == '@';
}

static const tic_outline_item* getZuoOutline(const char* code, s32* size)
{
    enum{Size = sizeof(tic_outline_item)};
    *size = 0;

    static tic_outline_item* items = NULL;

    if(items)
    {
        free(items);
        items = NULL;
    }

    const char* ptr = code;

    while(true)
    {
        static const char FuncString[] = "(define (";

        ptr = strstr(ptr, FuncString);

        if(ptr)
        {
            ptr += sizeof FuncString - 1;

            const char* start = ptr;
            const char* end = start;

            while(*ptr)
            {
                char c = *ptr;

                if(zuo_isalnum(c));
                else
                {
                    end = ptr;
                    break;
                }
                ptr++;
            }

            if(end > start)
            {
                items = realloc(items, (*size + 1) * Size);

                items[*size].pos = start;
                items[*size].size = (s32)(end - start);

                (*size)++;
            }
        }
        else break;
    }

    return items;
}

void evalZuo(tic_mem* tic, const char* code) {
    tic_core* core = (tic_core*)tic;
    zuo_scheme* sc = core->currentVM;

    // make sure that the Zuo interpreter is initialized.
    if (sc == NULL)
    {
        if (!initZuo(tic, ""))
            return;
        sc = core->currentVM;
    }
    
    zuo_eval_c_string(sc, code);
}

static const char* ZuoAPIKeywords[] = {
#define TIC_CALLBACK_DEF(name, ...) #name,
        TIC_CALLBACK_LIST(TIC_CALLBACK_DEF)
#undef  TIC_CALLBACK_DEF

#define API_KEYWORD_DEF(name, ...) "t80::" #name,
        TIC_API_LIST(API_KEYWORD_DEF)
#undef  API_KEYWORD_DEF
};

tic_script_config ZuoSyntaxConfig =
{
    .id                     = 19,
    .name                   = "zuo",
    .fileExtension          = ".zuo",
    .projectComment         = ";;",
    {
      .init                 = initZuo,
      .close                = closeZuo,
      .tick                 = callZuoTick,
      .boot                 = callZuoBoot,

      .callback             =
      {
        .scanline           = callZuoScanline,
        .border             = callZuoBorder,
        .menu               = callZuoMenu,
      },
    },

    .getOutline             = getZuoOutline,
    .eval                   = evalZuo,

    .blockCommentStart      = NULL,
    .blockCommentEnd        = NULL,
    .blockCommentStart2     = NULL,
    .blockCommentEnd2       = NULL,
    .singleComment          = ";;",
    .blockStringStart       = "\"",
    .blockStringEnd         = "\"",
    .stdStringStartEnd      = "\"",
    .blockEnd               = NULL,
    .lang_isalnum           = zuo_isalnum,
    .api_keywords           = ZuoAPIKeywords,
    .api_keywordsCount      = COUNT_OF(ZuoAPIKeywords),
    .useStructuredEdition   = true,

    .keywords               = ZuoKeywords,
    .keywordsCount          = COUNT_OF(ZuoKeywords),
};

#endif /* defined(TIC_BUILD_WITH_ZUO) */
