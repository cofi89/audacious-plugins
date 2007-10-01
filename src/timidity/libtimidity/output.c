/* 

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    output.c
    
    Audio output (to file / device) functions.
*/

#include <config.h>

#include "timidity.h"
#include "timidity_internal.h"
#include "options.h"
#include "output.h"

/*****************************************************************/
/* Some functions to convert signed 32-bit data to other formats */

void s32tos8(void *dp, sint32 *lp, sint32 c)
{
  sint8 *cp=(sint8 *)(dp);
  sint32 l;
  while (c--)
    {
      l=(*lp++)>>(32-8-GUARD_BITS);
      if (l>127) l=127;
      else if (l<-128) l=-128;
      *cp++ = (sint8) (l);
    }
}

void s32tou8(void *dp, sint32 *lp, sint32 c)
{
  uint8 *cp=(uint8 *)(dp);
  sint32 l;
  while (c--)
    {
      l=(*lp++)>>(32-8-GUARD_BITS);
      if (l>127) l=127;
      else if (l<-128) l=-128;
      *cp++ = 0x80 ^ ((uint8) l);
    }
}

void s32tos16(void *dp, sint32 *lp, sint32 c)
{
  sint16 *sp=(sint16 *)(dp);
  sint32 l;
  while (c--)
    {
      l=(*lp++)>>(32-16-GUARD_BITS);
      if (l > 32767) l=32767;
      else if (l<-32768) l=-32768;
      *sp++ = (sint16)(l);
    }
}

void s32tou16(void *dp, sint32 *lp, sint32 c)
{
  uint16 *sp=(uint16 *)(dp);
  sint32 l;
  while (c--)
    {
      l=(*lp++)>>(32-16-GUARD_BITS);
      if (l > 32767) l=32767;
      else if (l<-32768) l=-32768;
      *sp++ = 0x8000 ^ (uint16)(l);
    }
}

void s32tos16x(void *dp, sint32 *lp, sint32 c)
{
  sint16 *sp=(sint16 *)(dp);
  sint32 l;
  while (c--)
    {
      l=(*lp++)>>(32-16-GUARD_BITS);
      if (l > 32767) l=32767;
      else if (l<-32768) l=-32768;
      *sp++ = XCHG_SHORT((sint16)(l));
    }
}

void s32tou16x(void *dp, sint32 *lp, sint32 c)
{
  uint16 *sp=(uint16 *)(dp);
  sint32 l;
  while (c--)
    {
      l=(*lp++)>>(32-16-GUARD_BITS);
      if (l > 32767) l=32767;
      else if (l<-32768) l=-32768;
      *sp++ = XCHG_SHORT(0x8000 ^ (uint16)(l));
    }
}
