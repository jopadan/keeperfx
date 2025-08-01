/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_vidraw_spr_remp.c
 *     Graphics canvas drawing library, scaled sprite drawing with remaped colors.
 * @par Purpose:
 *    Screen drawing routines; draws rescaled sprite.
 * @par Comment:
 *     Medium level library, draws on screen buffer used in bflib_video.
 *     Used for drawing screen components.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 01 Aug 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_vidraw.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"

#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_mouse.h"
#include "bflib_render.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern long xsteps_array[2*SPRITE_SCALING_XSTEPS];
extern long ysteps_array[2*SPRITE_SCALING_YSTEPS];
/******************************************************************************/
void LbPixelBlockCopyForward(TbPixel * dst, const TbPixel * src, long len);
/******************************************************************************/
/**
 * Draws a scaled up sprite on given buffer, with transparency mapping and source colours remapped, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingUpDataTrans1RL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            const unsigned char *prevdata;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            prevdata = sprdata;
            while (ydup > 0)
            {
                sprdata = prevdata;
                xcurstep = xstep;
                TbPixel *out_end;
                out_end = outbuf;
                while ( 1 )
                {
                    long pxlen;
                    pxlen = (signed char)*sprdata;
                    sprdata++;
                    if (pxlen == 0)
                        break;
                    if (pxlen < 0)
                    {
                        pxlen = -pxlen;
                        out_end -= xcurstep[0] + xcurstep[1];
                        xcurstep -= 2 * pxlen;
                        out_end += xcurstep[0] + xcurstep[1];
                    }
                    else
                    {
                        for (;pxlen > 0; pxlen--)
                        {
                            xdup = xcurstep[1];
                            if (xcurstep[0]+xdup > abs(scanline))
                                xdup = abs(scanline)-xcurstep[0];
                            if (xdup > 0)
                            {
                                unsigned int pxmap;
                                pxmap = ((cmap[*sprdata]) << 8);
                                for (;xdup > 0; xdup--)
                                {
                                    pxmap = (pxmap & ~0x00ff) | ((*out_end));
                                    *out_end = transmap[pxmap];
                                    out_end--;
                                }
                            }
                            sprdata++;
                            xcurstep -= 2;
                        }
                    }
                }
                outbuf += scanline;
                ydup--;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with transparency mapping and source colours remapped, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @param transmap The transparency mapping table to be used. Should have a size of 256x256 to avoid invalid memory reads.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingUpDataTrans1LR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            const unsigned char *prevdata;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            prevdata = sprdata;
            while (ydup > 0)
            {
                sprdata = prevdata;
                xcurstep = xstep;
                TbPixel *out_end;
                out_end = outbuf;
                while ( 1 )
                {
                    long pxlen;
                    pxlen = (signed char)*sprdata;
                    sprdata++;
                    if (pxlen == 0)
                        break;
                    if (pxlen < 0)
                    {
                        pxlen = -pxlen;
                        out_end -= xcurstep[0];
                        xcurstep += 2 * pxlen;
                        out_end += xcurstep[0];
                    }
                    else
                    {
                        for (;pxlen > 0; pxlen--)
                        {
                            xdup = xcurstep[1];
                            if (xcurstep[0]+xdup > abs(scanline))
                                xdup = abs(scanline)-xcurstep[0];
                            if (xdup > 0)
                            {
                                unsigned int pxmap;
                                pxmap = ((cmap[*sprdata]) << 8);
                                for (;xdup > 0; xdup--)
                                {
                                    pxmap = (pxmap & ~0x00ff) | ((*out_end));
                                    *out_end = transmap[pxmap];
                                    out_end++;
                                }
                            }
                            sprdata++;
                            xcurstep += 2;
                        }
                    }
                }
                outbuf += scanline;
                ydup--;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with reversed transparency mapping and source colours remapped, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingUpDataTrans2RL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            const unsigned char *prevdata;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            prevdata = sprdata;
            while (ydup > 0)
            {
                sprdata = prevdata;
                xcurstep = xstep;
                TbPixel *out_end;
                out_end = outbuf;
                while ( 1 )
                {
                    long pxlen;
                    pxlen = (signed char)*sprdata;
                    sprdata++;
                    if (pxlen == 0)
                        break;
                    if (pxlen < 0)
                    {
                        pxlen = -pxlen;
                        out_end -= xcurstep[0] + xcurstep[1];
                        xcurstep -= 2 * pxlen;
                        out_end += xcurstep[0] + xcurstep[1];
                    }
                    else
                    {
                        for (;pxlen > 0; pxlen--)
                        {
                            xdup = xcurstep[1];
                            if (xcurstep[0]+xdup > abs(scanline))
                                xdup = abs(scanline)-xcurstep[0];
                            if (xdup > 0)
                            {
                                unsigned int pxmap;
                                pxmap = (cmap[*sprdata]);
                                for (;xdup > 0; xdup--)
                                {
                                    pxmap = (pxmap & ~0xff00) | ((*out_end) << 8);
                                    *out_end = transmap[pxmap];
                                    out_end--;
                                }
                            }
                            sprdata++;
                            xcurstep -= 2;
                        }
                    }
                }
                outbuf += scanline;
                ydup--;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with reversed transparency mapping and source colours remapped, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingUpDataTrans2LR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            const unsigned char *prevdata;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            prevdata = sprdata;
            while (ydup > 0)
            {
                sprdata = prevdata;
                xcurstep = xstep;
                TbPixel *out_end;
                out_end = outbuf;
                while ( 1 )
                {
                    long pxlen;
                    pxlen = (signed char)*sprdata;
                    sprdata++;
                    if (pxlen == 0)
                        break;
                    if (pxlen < 0)
                    {
                        pxlen = -pxlen;
                        out_end -= xcurstep[0];
                        xcurstep += 2 * pxlen;
                        out_end += xcurstep[0];
                    }
                    else
                    {
                        for (;pxlen > 0; pxlen--)
                        {
                            xdup = xcurstep[1];
                            if (xcurstep[0]+xdup > abs(scanline))
                                xdup = abs(scanline)-xcurstep[0];
                            if (xdup > 0)
                            {
                                unsigned int pxmap;
                                pxmap = (cmap[*sprdata]);
                                for (;xdup > 0; xdup--)
                                {
                                    pxmap = (pxmap & ~0xff00) | ((*out_end) << 8);
                                    *out_end = transmap[pxmap];
                                    out_end++;
                                }
                            }
                            sprdata++;
                            xcurstep += 2;
                        }
                    }
                }
                outbuf += scanline;
                ydup--;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with source colours remapped, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingUpDataSolidRL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            int ycur;
            int solid_len;
            TbPixel * out_line;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0] + xcurstep[1];
                    xcurstep -= 2 * pxlen;
                    out_end += xcurstep[0] + xcurstep[1];
                }
                else
                {
                    TbPixel *out_start;
                    out_start = out_end;
                    for(;pxlen > 0; pxlen--)
                    {
                        xdup = xcurstep[1];
                        if (xcurstep[0]+xdup > abs(scanline))
                            xdup = abs(scanline)-xcurstep[0];
                        if (xdup > 0)
                        {
                            unsigned char pxval;
                            pxval = (cmap[*sprdata]);
                            for (;xdup > 0; xdup--)
                            {
                                *out_end = pxval;
                                out_end--;
                            }
                        }
                        sprdata++;
                        xcurstep -= 2;
                    }
                    ycur = ydup - 1;
                    if (ycur > 0)
                    {
                        solid_len = out_start - out_end;
                        out_start = out_end;
                        solid_len++;
                        out_line = out_start + scanline;
                        for (;ycur > 0; ycur--)
                        {
                            if (solid_len > 0) {
                                LbPixelBlockCopyForward(out_line, out_start, solid_len);
                            }
                            out_line += scanline;
                        }
                    }
                }
            }
            outbuf += scanline;
            ycur = ydup - 1;
            for (;ycur > 0; ycur--)
            {
                outbuf += scanline;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled up sprite on given buffer, with source colours remapped, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingUpDataSolidLR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            int ycur;
            int solid_len;
            TbPixel * out_line;
            int xdup;
            int ydup;
            long *xcurstep;
            ydup = ycurstep[1];
            if (ycurstep[0]+ydup > outheight)
                ydup = outheight-ycurstep[0];
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0];
                    xcurstep += 2 * pxlen;
                    out_end += xcurstep[0];
                }
                else
                {
                    TbPixel *out_start;
                    out_start = out_end;
                    for(;pxlen > 0; pxlen--)
                    {
                        xdup = xcurstep[1];
                        if (xcurstep[0]+xdup > abs(scanline))
                            xdup = abs(scanline)-xcurstep[0];
                        if (xdup > 0)
                        {
                            unsigned char pxval;
                            pxval = (cmap[*sprdata]);
                            for (;xdup > 0; xdup--)
                            {
                                *out_end = pxval;
                                out_end++;
                            }
                        }
                        sprdata++;
                        xcurstep += 2;
                    }
                    ycur = ydup - 1;
                    if (ycur > 0)
                    {
                        solid_len = out_end - out_start;
                        out_line = out_start + scanline;
                        for (;ycur > 0; ycur--)
                        {
                            if (solid_len > 0) {
                                LbPixelBlockCopyForward(out_line, out_start, solid_len);
                            }
                            out_line += scanline;
                        }
                    }
                }
            }
            outbuf += scanline;
            ycur = ydup - 1;
            for (;ycur > 0; ycur--)
            {
                outbuf += scanline;
            }
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with transparency mapping and source colours remapped, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingDownDataTrans1RL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0] + xcurstep[1];
                    xcurstep -= 2 * pxlen;
                    out_end += xcurstep[0] + xcurstep[1];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned int pxmap;
                            pxmap = ((cmap[*sprdata]) << 8);
                            {
                                pxmap = (pxmap & ~0x00ff) | ((*out_end));
                                *out_end = transmap[pxmap];
                                out_end--;
                            }
                        }
                        sprdata++;
                        xcurstep -= 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with transparency mapping and source colours remapped, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingDownDataTrans1LR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0];
                    xcurstep += 2 * pxlen;
                    out_end += xcurstep[0];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned int pxmap;
                            pxmap = ((cmap[*sprdata]) << 8);
                            {
                                pxmap = (pxmap & ~0x00ff) | ((*out_end));
                                *out_end = transmap[pxmap];
                                out_end++;
                            }
                        }
                        sprdata++;
                        xcurstep += 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with reverse transparency mapping and source colours remapped, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingDownDataTrans2RL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0] + xcurstep[1];
                    xcurstep -= 2 * pxlen;
                    out_end += xcurstep[0] + xcurstep[1];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned int pxmap;
                            pxmap = ((cmap[*sprdata]) << 8);
                            {
                                pxmap = (pxmap & ~0xff00) | ((*out_end) << 8);
                                *out_end = transmap[pxmap];
                                out_end--;
                            }
                        }
                        sprdata++;
                        xcurstep -= 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with reverse transparency mapping and source colours remapped, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @param transmap The transparency mapping table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingDownDataTrans2LR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap, const TbPixel *transmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0];
                    xcurstep += 2 * pxlen;
                    out_end += xcurstep[0];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned int pxmap;
                            pxmap = (cmap[*sprdata]);
                            {
                                pxmap = (pxmap & ~0xff00) | ((*out_end) << 8);
                                *out_end = transmap[pxmap];
                                out_end++;
                            }
                        }
                        sprdata++;
                        xcurstep += 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with source colours remapped, from right to left.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingDownDataSolidRL(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0] + xcurstep[1];
                    xcurstep -= 2 * pxlen;
                    out_end += xcurstep[0] + xcurstep[1];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned char pxval;
                            pxval = (cmap[*sprdata]);
                            {
                                *out_end = pxval;
                                out_end--;
                            }
                        }
                        sprdata++;
                        xcurstep -= 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled down sprite on given buffer, with source colours remapped, from left to right.
 * Requires step arrays for scaling.
 *
 * @param outbuf The output buffer.
 * @param scanline Length of the output buffer scanline.
 * @param xstep Scaling steps array, x dimension.
 * @param ystep Scaling steps array, y dimension.
 * @param sprite The source sprite.
 * @param cmap The colour remap table to be used.
 * @return Gives 0 on success.
 */
TbResult LbSpriteDrawRemapUsingScalingDownDataSolidLR(uchar *outbuf, int scanline, int outheight, long *xstep, long *ystep, const struct TbSourceBuffer * src_buf, const TbPixel *cmap)
{
    SYNCDBG(17,"Drawing");
    int ystep_delta;
    long *ycurstep;

    if (!outbuf || !xstep || !ystep || !src_buf || !src_buf->data || !cmap)
        return -1;

    ystep_delta = 2;
    if (scanline < 0) {
        ystep_delta = -2;
    }
    const unsigned char * sprdata = src_buf->data;
    ycurstep = ystep;

    for (int h = src_buf->height; h > 0; h--)
    {
        if (ycurstep[1] != 0)
        {
            long *xcurstep;
            xcurstep = xstep;
            TbPixel *out_end;
            out_end = outbuf;
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                    break;
                if (pxlen < 0)
                {
                    pxlen = -pxlen;
                    out_end -= xcurstep[0];
                    xcurstep += 2 * pxlen;
                    out_end += xcurstep[0];
                }
                else
                {
                    for (;pxlen > 0; pxlen--)
                    {
                        if (xcurstep[1] > 0)
                        {
                            unsigned char pxval;
                            pxval = (cmap[*sprdata]);
                            {
                                *out_end = pxval;
                                out_end++;
                            }
                        }
                        sprdata++;
                        xcurstep += 2;
                    }
                }
            }
            outbuf += scanline;
        }
        else
        {
            while ( 1 )
            {
                long pxlen;
                pxlen = (signed char)*sprdata;
                sprdata++;
                if (pxlen == 0)
                  break;
                if (pxlen > 0)
                {
                    sprdata += pxlen;
                }
            }
        }
        ycurstep += ystep_delta;
    }
    return 0;
}

/**
 * Draws a scaled sprite with remapped colours on current graphics window at given position.
 * Requires LbSpriteSetScalingData() to be called before.
 *
 * @param posx The X coord within current graphics window.
 * @param posy The Y coord within current graphics window.
 * @param sprite The source sprite.
 * @param cmap Colour mapping array.
 * @return Gives 0 on success.
 * @see LbSpriteSetScalingData()
 */
TbResult LbSpriteDrawRemapUsingScalingData(long posx, long posy, const struct TbSourceBuffer * src_buf, const TbPixel *cmap)
{
    SYNCDBG(17,"Drawing at (%ld,%ld)",posx,posy);
    long *xstep;
    long *ystep;
    int scanline;
    uchar *outbuf;
    int outheight;
    setup_steps(posx, posy, src_buf, &xstep, &ystep, &scanline);
    setup_outbuf(xstep, ystep, &outbuf, &outheight);
    if ( scale_up )
    {
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawRemapUsingScalingUpDataTrans1RL(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap, render_ghost);
          }
          else
          {
              return LbSpriteDrawRemapUsingScalingUpDataTrans1LR(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap, render_ghost);
          }
        }
        else
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8) != 0)
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawRemapUsingScalingUpDataTrans2RL(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap, render_ghost);
          }
          else
          {
              return LbSpriteDrawRemapUsingScalingUpDataTrans2LR(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap, render_ghost);
          }
        }
        else
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawRemapUsingScalingUpDataSolidRL(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap);
          }
          else
          {
              return LbSpriteDrawRemapUsingScalingUpDataSolidLR(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap);
          }
        }
    }
    else
    {
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawRemapUsingScalingDownDataTrans1RL(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap, render_ghost);
          }
          else
          {
              return LbSpriteDrawRemapUsingScalingDownDataTrans1LR(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap, render_ghost);
          }
        }
        else
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8) != 0)
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawRemapUsingScalingDownDataTrans2RL(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap, render_ghost);
          }
          else
          {
              return LbSpriteDrawRemapUsingScalingDownDataTrans2LR(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap, render_ghost);
          }
        }
        else
        {
          if ((lbDisplay.DrawFlags & Lb_SPRITE_FLIP_HORIZ) != 0)
          {
              return LbSpriteDrawRemapUsingScalingDownDataSolidRL(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap);
          }
          else
          {
              return LbSpriteDrawRemapUsingScalingDownDataSolidLR(outbuf, scanline, outheight, xstep, ystep, src_buf, cmap);
          }
        }
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
