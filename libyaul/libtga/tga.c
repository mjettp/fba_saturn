/*
 * Copyright (c) 2013
 * See LICENSE for details.
 *
 * Israel Jacquez <mrkotfw@gmail.com>
 * David Oberhollenzer
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "tga.h"

#define READ_LITTLE_ENDIAN_16(array, index)                                    \
        (((uint32_t)(array)[(index)]) | ((uint32_t)(array)[(index) + 1]) << 8)

#define TGA_HEADER_LEN  18
#define TGA_FOOTER_LEN  26
#define TGA_ORIGIN_LEFT     0x00
#define TGA_ORIGIN_RIGHT    0x10
#define TGA_ORIGIN_LOWER    0x00
#define TGA_ORIGIN_UPPER    0x20

#define TGA_PACKET_TYPE_RLE 1
#define TGA_PACKET_TYPE_RAW 0

//static uint16_t bgr16_to_rgb555(const uint8_t *);
//static uint16_t bgr24_to_rgb555(const uint8_t *);
//static uint16_t bgra32_to_rgb555(const uint8_t *);

static void tga_cmap_decode(uint16_t *, const uint8_t *, size_t, uint8_t);
//static void tga_image_decode(uint16_t *, const uint8_t *, uint16_t, uint16_t,
//    uint8_t);
//static void tga_rle_image_decode(uint16_t *, const uint8_t *, uint16_t,
//    uint16_t, uint8_t);

/*
 *
 */
int
tga_load(const uint8_t *file, uint16_t *vram, uint16_t *cram)
{
        const uint8_t *header;
        const uint8_t *imagebuf;
        const uint16_t *cmapbuf;

        uint8_t id_len;
        uint8_t image_type;
        uint8_t bytespp;
        uint16_t width;
        uint16_t height;

        uint8_t have_cmap;
        uint8_t cmap_bpp;
        uint16_t cmap_offset;
        uint16_t cmap_len;
        size_t cmap_bytes;
		uint8_t flip;//flags;

        /* Read the TGA header */
        header = &file[0];
        id_len = header[0];

        /* Skip the header */
        file += TGA_HEADER_LEN;

        image_type = header[2];
        bytespp = header[16] >> 3;
        width = READ_LITTLE_ENDIAN_16(header, 12);
        height = READ_LITTLE_ENDIAN_16(header, 14);

        have_cmap = header[1];
        cmapbuf = NULL;
        cmap_offset = READ_LITTLE_ENDIAN_16(header, 3);
        cmap_len = READ_LITTLE_ENDIAN_16(header, 5);
        cmap_bpp = header[7];
        cmap_bytes = cmap_len * (cmap_bpp >> 3);
		flip = (header[17] && TGA_ORIGIN_LOWER);
//        memset(tga, 0x00, sizeof(tga_t));

        /* Sanity checks */
        /* The maximum size of a TGA image is 512 pixels wide by 482
         * pixels high. */
        if ((width == 0) || (height == 0) || (width > 512) || (height > 482)) {
                return TGA_FILE_CORRUPTED;
        }

        switch (image_type) {
        case TGA_IMAGE_TYPE_GRAYSCALE:
        case TGA_IMAGE_TYPE_RLE_GRAYSCALE:
                if ((bytespp != 1) || have_cmap) {
                        return TGA_FILE_CORRUPTED;
                }

                break;
        case TGA_IMAGE_TYPE_TRUE_COLOR:
        case TGA_IMAGE_TYPE_RLE_TRUE_COLOR:
                if (((bytespp != 2) && (bytespp != 3) && (bytespp != 4)) ||
                    have_cmap) {
                        return TGA_FILE_CORRUPTED;
                }

                break;
        case TGA_IMAGE_TYPE_CMAP:
        case TGA_IMAGE_TYPE_RLE_CMAP:
                if (!have_cmap || (cmap_offset >= cmap_len)) {
                        return TGA_FILE_CORRUPTED;
                }

                switch (cmap_bpp) {
                case 15:
                case 16:
                case 24:
                case 32:
                        break;
                default:
                        return TGA_FILE_CORRUPTED;
                }

                break;
        case TGA_IMAGE_TYPE_NONE:
        default:
                return TGA_FILE_CORRUPTED;
        }

        file += id_len;

        /* Read the color map if present and allocate an image buffer */
        if (have_cmap) {
                tga_cmap_decode(cram, file, cmap_len, cmap_bpp);

                file += cmap_bytes;
        }

        imagebuf = file;

        switch (image_type) {
        case TGA_IMAGE_TYPE_GRAYSCALE:
        case TGA_IMAGE_TYPE_RLE_GRAYSCALE:
                return TGA_FILE_NOT_SUPPORTED;
        case TGA_IMAGE_TYPE_CMAP:
                break;
        case TGA_IMAGE_TYPE_TRUE_COLOR:
                break;
        case TGA_IMAGE_TYPE_RLE_TRUE_COLOR:
				break;
        case TGA_IMAGE_TYPE_RLE_CMAP:
				decode_map_rle8((unsigned char *)imagebuf, (unsigned char *)vram, width, height,bytespp,flip);
                break;
        }
        return TGA_FILE_OK;
}

static uint16_t
bgr16_to_rgb555(const uint8_t *cmapbuf)
{
        uint8_t r;
        uint8_t g;
        uint8_t b;

        uint16_t color;

        color = *(uint16_t *)cmapbuf & ~0x8000;

        /* Ignore the MSB */
        /* Swap channels R and B */
        r = (color & 0x7C00) >> 10;
        g = (color & 0x03E0) >> 5;
        b = color & 0x001F;

        return 0x8000 | ((b << 10) | (g << 5) | r);
}

static uint16_t
bgr24_to_rgb555(const uint8_t *cmapbuf)
{
        uint8_t r;
        uint8_t g;
        uint8_t b;

        r = cmapbuf[2] >> 3;
        g = cmapbuf[1] >> 3;
        b = cmapbuf[0] >> 3;

        return 0x8000 | ((b << 10) | (g << 5) | r);
}

static uint16_t
bgra32_to_rgb555(const uint8_t *cmapbuf)
{
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        a = cmapbuf[3];
        r = cmapbuf[2] >> 3;
        g = cmapbuf[1] >> 3;
        b = cmapbuf[0] >> 3;

        return 0x8000 | ((a == 0x00) ? 0x0000 : ((b << 10) | (g << 5) | r));
}

static void tga_cmap_decode(uint16_t *cram, const uint8_t *cmapbuf, size_t cmap_len,
    uint8_t cmap_bpp)
{
        uint16_t cmap_idx;
        for (cmap_idx = 0; cmap_idx < cmap_len; cmap_idx++) {
                switch (cmap_bpp) {
                case 16:
                        cram[cmap_idx] =
                            bgr16_to_rgb555(&cmapbuf[cmap_idx * 2]);
						break;
                case 24:
                        cram[cmap_idx] =
                            bgr24_to_rgb555(&cmapbuf[cmap_idx * 3]);
						break;
                case 32:
                        cram[cmap_idx] =
                            bgra32_to_rgb555(&cmapbuf[cmap_idx * 4]);
					break;
                }
        }
}

/*
 *
 */
static void
tga_image_decode(uint16_t *vram, const uint8_t *buf, uint16_t width,
    uint16_t height, uint8_t bytespp)
{
        int pixels;

        pixels = 0;

        while (pixels < (width * height)) {
                uint16_t color;

                switch (bytespp) {
                case 2:
                        color = bgr16_to_rgb555(&buf[2 * pixels]);

                        *vram++ = color;
                        break;
                case 3:
                        /* Convert BGR24 to RGB555 */
                        color = bgr24_to_rgb555(&buf[3 * pixels]);

                        *vram++ = color;
                        break;
                case 4:
                        /* Convert BGRA32 to RGB555 */
                        color = bgra32_to_rgb555(&buf[4 * pixels]);

                        *vram++ = color;
                        break;
                }

                pixels++;
        }
}




int decode_map_rle8(	  unsigned char *src0, unsigned char *dest0, int width, int height,	int chan, int flip) //, const unsigned char *color_map)
{
  const unsigned char *src = src0;
  unsigned char *dest = dest0;
  unsigned char *fdest = dest0;
	int line=0;	
  while(dest < dest0 + 512*height*chan) {
    // Pixels encoded in "packets"

    // First byte is raw/rle flag(upper bit) and count(1-128 as 0-127
    // in lower 7 bits) If raw, the next count chan-byte color values
    // in the file are taken verbatim If rle, the next single
    // chan-byte color value speaks for the next count pixels
		
    int raw = (*src & 0x80) == 0; // Is this packet raw pixels or a repeating color
    int count = (*src & 0x7f) + 1; // How many raw pixels or color repeats
    src++; // Advance src beyond first byte to map value

		
    if(dest + count*chan > dest0 + 512*height*chan)
      count = (dest0 + 512*height*chan - dest) / chan;
		 int c,j;
    for(j=0; j<count; j++)
    {

      for( c=0; c<chan; c++)
//        dest[chan - c - 1] = color_map[src[0] * chan + c];
          dest[chan - c - 1] = src[0] * chan + c;
			
      if(raw) // In raw mode, keep advancing "src" to subsequent values
        src++; // In RLE mode, just repeat the packet[1] RGB color
      dest += chan;
	  line+= chan;
	  if(line==width)
		{
		    memset(dest,0x00,512-width);
			dest+=(512-width);
			line=0;
		}
    }
    if(!raw) // After outputting count RGBA values, advance "src" if rle
      src ++;
  }
	
//	if (flip)
		
	{
		char save[512];
//		memcpyl(save,fdest,512);
		for (line=0;line <height/2;line++ )
		{
				memcpy(save,&fdest[line<<9],512);
				memcpy(&fdest[line<<9],&fdest[(height-line-1)<<9],512);
				memcpy(&fdest[(height-line-1)<<9],save,512);
		}
//		memcpyl(&fdest[height<<9],save,512);
	}						
	return(0);
}

/*
 *
 */
static void

tga_rle_image_decode(uint16_t *vram, const uint8_t *buf, uint16_t width,
    uint16_t height, uint8_t bytespp)
{
        const uint8_t *packet;
        const uint8_t *next_packet;

        uint8_t packet_type;

        int pixels;

        packet = buf;

        pixels = 0;

        while (pixels < (width * height)) {
                uint8_t rcf;

                packet_type = (packet[0] & 0x80) >> 7;
                /* Determine how many pixels are in this packet */
                rcf = (packet[0] & ~0x80) + 1;

                switch (packet_type) {
                case TGA_PACKET_TYPE_RAW:
                        next_packet = (uint8_t *)(packet + (rcf * bytespp) + 1);
                        break;
                case TGA_PACKET_TYPE_RLE:
                        next_packet = (uint8_t *)(packet + bytespp + 1);
                        break;
                }

                uint8_t rcf_idx;

                for (rcf_idx = 0; rcf_idx < rcf; rcf_idx++) {
                        uint32_t offset;

                        offset = (packet_type == TGA_PACKET_TYPE_RLE)
                            ? 0x00000000
                            : rcf_idx;

                        uint16_t color;

                        switch (bytespp) {
                        case 1:
                                /* FIXME: 1-byte write only */
                                *vram++ = (packet[1 + offset]);// || packet[2 + offset]<<8);
                                break;
                        case 2:
                                color = bgr16_to_rgb555(&buf[1 + (2 * pixels)]);

                                *vram++ = color;
                                break;
                        case 3:
                                /* Convert BGR24 to RGB555 */
                                color =
                                    bgr24_to_rgb555(&packet[1 + (3 * offset)]);

                                *vram++ = color;
                                break;
                        case 4:
                                /* Convert BGRA32 to RGB555 */
                                color =
                                    bgra32_to_rgb555(&packet[1 + (4 * offset)]);

                                *vram++ = color;
                                break;
                        }
                }

                packet = next_packet;
                pixels += rcf;
        }
}
