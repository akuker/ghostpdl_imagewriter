/* Copyright (C) 1990, 2000 Aladdin Enterprises.  All rights reserved.
  
  This software is provided AS-IS with no warranty, either express or
  implied.
  
  This software is distributed under license and may not be copied,
  modified or distributed except as expressly authorized under the terms
  of the license contained in the file LICENSE in this distribution.
  
  For more information about licensing, please refer to
  http://www.ghostscript.com/licensing/. For information on
  commercial licensing, go to http://www.artifex.com/licensing/ or
  contact Artifex Software, Inc., 101 Lucas Valley Road #110,
  San Rafael, CA  94903, U.S.A., +1(415)492-9861.
*/

/* $Id$ */
/* Type 1 hinter, a new algorithm, prototypes */

#ifndef gxhintn_INCLUDED
#  define gxhintn_INCLUDED

#ifndef gs_type1_data_s_DEFINED
struct gs_type1_data_s;
#endif
typedef struct gs_type1_data_s gs_type1_data;

#define T1_MAX_STEM_SNAPS 12
#define T1_MAX_ALIGNMENT_ZONES 6
#define T1_MAX_CONTOURS 10
#define T1_MAX_POLES (100 + T1_MAX_CONTOURS)
#define T1_MAX_HINTS 30 /* total number of hints in a glyph, not a maximal one at a moiment */

typedef int int32;
typedef fixed t1_glyph_space_coord; /* measured in original glyph space units */
typedef int32 t1_hinter_space_coord; /* measured in temporary outliner's space units */
typedef int32 int19;

enum t1_hint_type
{   hstem, vstem, dot, replace
};

enum t1_pole_type
{   offcurve, oncurve, closepath, moveto
};

enum t1_zone_type
{   topzone, botzone
};

enum t1_align_type
{   unaligned, aligned, topzn, botzn
};

typedef struct {
    double xx, xy, yx, yy;
} double_matrix;

typedef struct {
    int19 xx, xy, yx, yy;
    int32 denominator;
    unsigned int bitshift;
} fraction_matrix;

typedef struct t1_pole_s /* a pole of outline */
{   t1_glyph_space_coord gx,gy; /* source unaligned coords */
    t1_glyph_space_coord ax,ay; /* aligned coords */
    t1_hinter_space_coord ox,oy;
    enum t1_pole_type type;
    int contour_index;
    enum t1_align_type aligned_x, aligned_y;
} t1_pole;

typedef struct t1_hint_s
{   enum t1_hint_type type;
    t1_glyph_space_coord g0, g1; /* starting and ending transversal coord of the stem */
    t1_glyph_space_coord ag0, ag1; /* starting and ending transversal aligned coord of the stem */
    enum t1_alignment_type aligned0, aligned1; /* ag0, ag1 is aligned */
    short start_pole; /* outline pole to search for stem the stem from */
    short beg_pole, end_pole; /* outline interval of the stem */
    unsigned int stem3_index; /* 1,2,3 for stem3 (not used yet), 0 for other types */
    short complex_link; /* the index of next member of same complex */
    int contour_index;
    int range_index; /* type 2 only */
    bool completed;
} t1_hint;

typedef struct t1_hint_range_s
{   short beg_pole, end_pole;
    int next;
} t1_hint_range; /* type 2 only */

typedef struct t1_zone_s /* alignment zone */
{   enum t1_zone_type type;
    t1_glyph_space_coord y, overshoot_y;
    t1_glyph_space_coord y_min, y_max;
} t1_zone;

typedef struct t1_hinter_s
{   fraction_matrix ctmf;
    fraction_matrix ctmi;
    unsigned int g2o_fraction_bits;
    unsigned int import_shift;
    int32 g2o_fraction;
    t1_glyph_space_coord orig_gx, orig_gy; /* glyph origin in glyph space */
    t1_glyph_space_coord subglyph_orig_gx, subglyph_orig_gy; /* glyph origin in glyph space */
    fixed orig_dx, orig_dy; /* glyph origin in device space */
    t1_glyph_space_coord width_gx, width_gy; /* glyph space coords of the glyph origin */
    t1_glyph_space_coord cx, cy; /* current point */
    t1_glyph_space_coord bx, by; /* starting point of a contour */
    double BlueScale;
    t1_glyph_space_coord blue_shift, blue_fuzz;
    t1_pole pole0[T1_MAX_POLES], *pole;
    t1_hint hint0[T1_MAX_HINTS], *hint;
    t1_zone zone0[T1_MAX_ALIGNMENT_ZONES], *zone;
    int contour0[T1_MAX_CONTOURS], *contour;
    t1_glyph_space_coord stem_snap[2][T1_MAX_STEM_SNAPS + 1]; /* StdWH + StemSnapH, StdWV + StemSnapV */
    t1_hint_range hint_range0[T1_MAX_HINTS], *hint_range;
    int stem_snap_count[2]; /* H, V */
    int contour_count, max_contour_count;
    int zone_count, max_zone_count;
    int pole_count, max_pole_count;
    int hint_count, max_hint_count;
    int hint_range_count, max_hint_range_count;
    int primary_hint_count;
    int FontType; /* 1 or 2 */
    bool ForceBold;
    bool seac_flag;
    bool keep_stem_width;
    bool suppress_overshoots;
    bool disable_hinting;
    bool grid_fit_x, grid_fit_y;
    bool charpath_flag;
    double font_size;
    double resolution;
    double heigt_transform_coef;
    double width_transform_coef;
    double base_font_scale;
    int19 blue_rounding;
    int19 width_transform_coef_rat;
    int19 heigt_transform_coef_rat;
    int19 width_transform_coef_inv;
    int19 heigt_transform_coef_inv;
    t1_glyph_space_coord overshoot_threshold;
    gs_memory_t *memory;
} t1_hinter;

void t1_hinter__reset(t1_hinter * this, gs_memory_t * mem);
int  t1_hinter__init(t1_hinter * this, gs_matrix_fixed * ctm, gs_rect * FontBBox, 
			gs_matrix * FontMatrix, gs_matrix * baseFontMatrix);
int  t1_hinter__set_font_data(t1_hinter * this, int FontType, gs_type1_data *pdata, 
			bool charpath_flag, fixed origin_x, fixed origin_y);

int  t1_hinter__sbw(t1_hinter * this, t1_glyph_space_coord sbx, t1_glyph_space_coord sby
                                    , t1_glyph_space_coord wx,  t1_glyph_space_coord wy);
int  t1_hinter__sbw_seac(t1_hinter * this, t1_glyph_space_coord sbx, t1_glyph_space_coord sby);
int  t1_hinter__rmoveto(t1_hinter * this, t1_glyph_space_coord xx, t1_glyph_space_coord yy);
int  t1_hinter__rlineto(t1_hinter *, t1_glyph_space_coord xx, t1_glyph_space_coord yy);
int  t1_hinter__rcurveto(t1_hinter * this, t1_glyph_space_coord xx0, t1_glyph_space_coord yy0
                                         , t1_glyph_space_coord xx1, t1_glyph_space_coord yy1
                                         , t1_glyph_space_coord xx2, t1_glyph_space_coord yy2);
void t1_hinter__setcurrentpoint(t1_hinter * this, t1_glyph_space_coord xx, t1_glyph_space_coord yy);
int  t1_hinter__closepath(t1_hinter * this);

int  t1_hinter__hint_mask(t1_hinter * this, byte *mask);
int  t1_hinter__drop_hints(t1_hinter * this);
int  t1_hinter__dotsection(t1_hinter * this);
int  t1_hinter__hstem(t1_hinter * this, t1_glyph_space_coord x0, t1_glyph_space_coord x1);
int  t1_hinter__vstem(t1_hinter * this, t1_glyph_space_coord y0, t1_glyph_space_coord y1);
int  t1_hinter__hstem3(t1_hinter * this, t1_glyph_space_coord x0, t1_glyph_space_coord y1
                                       , t1_glyph_space_coord x2, t1_glyph_space_coord y3
                                       , t1_glyph_space_coord x4, t1_glyph_space_coord y5);
int  t1_hinter__vstem3(t1_hinter * this, t1_glyph_space_coord y0, t1_glyph_space_coord y1
                                       , t1_glyph_space_coord y2, t1_glyph_space_coord y3
                                       , t1_glyph_space_coord y4, t1_glyph_space_coord y5);

void t1_hinter__round_to_pixels(t1_hinter * this, t1_glyph_space_coord * gx, t1_glyph_space_coord * gy);
int  t1_hinter__endchar(t1_hinter * this, bool seac_flag);
int  t1_hinter__endglyph(t1_hinter * this, gs_op1_state * s);

#endif /* gxhintn_INCLUDED */
