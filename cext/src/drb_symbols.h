#ifndef CEXT_DRB_SYMBOLS_H
#define CEXT_DRB_SYMBOLS_H

#ifdef NOT_DRAGONRUBY
#define DRB_FFI //This will be accessible from DRGTK
#define DRB_FFI_NAME(name) //This will be accessible from DRGTK as `name`
#else
#include <dragonruby.h>
#endif

#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "globals.h"

#ifndef NOT_DRAGONRUBY
extern void *(*drb_symbol_lookup)(const char *sym);
typedef void (*drb_upload_pixel_array_fn)(const char *name, const int w, const int h, const uint32_t *pixels);
DRB_FFI
int screen_to_sprite(const char *name)
{
    static drb_upload_pixel_array_fn drb_upload_pixel_array = NULL;
    if (!drb_upload_pixel_array) {
        drb_upload_pixel_array = drb_symbol_lookup("drb_upload_pixel_array");
        if (!drb_upload_pixel_array) {
            return 1;  // oh well.
        }
    }
    drb_upload_pixel_array(name, SCREEN_WIDTH, SCREEN_HEIGHT, (const uint32_t *) &SCREEN);
    return 0;
}
extern void *(*drb_symbol_lookup)(const char *sym);
#endif

typedef void *(*drb_load_image_fn)(const char *fname, int *w, int *h);
typedef void (*drb_free_image_fn)(void *pixels);

Color *load_image(const char *fname, int *w, int *h) {
    Color *out = NULL;
#ifndef NOT_DRAGONRUBY
    static drb_load_image_fn drb_load_image = NULL;
    if (!drb_load_image) {
        drb_load_image = drb_symbol_lookup("drb_load_image");
    }
    if (drb_load_image) {
        void *vp = drb_load_image(fname, w, h);
        if(vp){
            out = calloc((*w) * (*h), sizeof(Color));
            memcpy(out, vp, (*w) * (*h) * sizeof(Color));
            static drb_free_image_fn drb_free_image = NULL;
            if (!drb_free_image) {
                drb_free_image = drb_symbol_lookup("drb_free_image");
            }
            if (drb_free_image) {
                drb_free_image(vp);
            }
        }
    }
#endif
    if (!out) {
        // Default to checkered black and magenta for missing textures.
        *w = 16;
        *h = 16;
        out = calloc((*w) * (*h), sizeof(Color));
        uint32_t  raw = 0xFF000000;
        for (int i   = 0; i < (*w)*(*h); ++i) {
            out[i] = (Color) {raw};
            raw = raw == 0xFF000000 ? 0xFFFF00FF : 0xFF000000;
        }
    }
    return out;
}
#endif //CEXT_DRB_SYMBOLS_H
