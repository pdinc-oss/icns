/*
File:       icns.h
Copyright (C) 2001-2008 Mathew Eis <mathew@eisbox.net>
Copyright (C) 2002 Chenxiao Zhao <chenxiao.zhao@gmail.com>

With the exception of the limited portions mentiond, this library
is free software; you can redistribute it and/or modify it under
the terms of the GNU Library General Public License as published
by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef _ICNS_H_
#define	_ICNS_H_

/*  Compile-time variables   */
/*  These should really be set from the Makefile */

// Enable debugging messages?
// #define	ICNS_DEBUG

// Enable openjpeg for 256x256 and 512x512 support
// #define	ICNS_OPENJPEG


/*  OpenJPEG headers   */
#ifdef ICNS_OPENJPEG
#include <openjpeg.h>

/*  OpenJPEG version check   */
// OPENJPEG_VERSION seems to be a reliable test for having
// the proper openjpeg header files.
#ifndef OPENJPEG_VERSION
	#warning "libicns: Can't determine OpenJPEG version."
	#warning "libicns: 256x256 and 512x512 support will not be available."
	#undef	ICNS_OPENJPEG
#endif
#endif

/* basic data types */
typedef uint8_t         icns_bool_t;

typedef uint8_t         icns_uint8_t;
typedef int8_t          icns_sint8_t;
typedef uint16_t        icns_uint16_t;
typedef int16_t         icns_sint16_t;
typedef uint32_t        icns_uint32_t;
typedef int32_t         icns_sint32_t;
typedef uint64_t        icns_uint64_t;
typedef int64_t         icns_sint64_t;

typedef uint8_t         icns_byte_t;


/* data header types */
/*
icns type is a 4 byte char array
use a struct so it can be easily
passed a function parameter.
*/
typedef struct icns_type_t {
  int8_t                c[4];
} icns_type_t;

typedef int32_t         icns_size_t;

/* icon family and element types */
typedef struct icns_element_t {
  icns_type_t           elementType;    /* 'ICN#', 'icl8', etc... */
  icns_size_t           elementSize;    /* Total size of element  */
  icns_byte_t           elementData[1]; /* icon image data */
} icns_element_t;

typedef struct icns_family_t {
  icns_type_t           resourceType;	/* Always should be 'icns' */
  icns_size_t           resourceSize;	/* Total size of resource  */
  icns_element_t        elements[1];    /* icon elements */
} icns_family_t;

/* icon image data structure */
typedef struct icns_image_t
{
  icns_uint32_t         imageWidth;     // width of image in pixels
  icns_uint32_t         imageHeight;    // height of image in pixels
  icns_uint8_t          imageChannels;  // number of channels in data
  icns_uint16_t         imagePixelDepth;// number of bits-per-pixel
  icns_uint64_t         imageDataSize;  // bytes = width * height * depth / bits-per-pixel
  icns_byte_t           *imageData;     // pointer to base address of uncompressed raw image data
} icns_image_t;

/*  icns element type constants */

static const icns_type_t  ICNS_512x512_32BIT_ARGB_DATA   = {{'i','c','0','9'}};
static const icns_type_t  ICNS_256x256_32BIT_ARGB_DATA   = {{'i','c','0','8'}};

static const icns_type_t  ICNS_128X128_32BIT_DATA        = {{'i','t','3','2'}};
static const icns_type_t  ICNS_128X128_8BIT_MASK         = {{'t','8','m','k'}};

static const icns_type_t  ICNS_48x48_1BIT_DATA           = {{'i','c','h','#'}};
static const icns_type_t  ICNS_48x48_4BIT_DATA           = {{'i','c','h','4'}};
static const icns_type_t  ICNS_48x48_8BIT_DATA           = {{'i','c','h','8'}};
static const icns_type_t  ICNS_48x48_32BIT_DATA          = {{'i','h','3','2'}};
static const icns_type_t  ICNS_48x48_1BIT_MASK           = {{'i','c','h','#'}};
static const icns_type_t  ICNS_48x48_8BIT_MASK           = {{'h','8','m','k'}};

static const icns_type_t  ICNS_32x32_1BIT_DATA           = {{'I','C','N','#'}};
static const icns_type_t  ICNS_32x32_4BIT_DATA           = {{'i','c','l','4'}};
static const icns_type_t  ICNS_32x32_8BIT_DATA           = {{'i','c','l','8'}};
static const icns_type_t  ICNS_32x32_32BIT_DATA          = {{'i','l','3','2'}};
static const icns_type_t  ICNS_32x32_1BIT_MASK           = {{'I','C','N','#'}};
static const icns_type_t  ICNS_32x32_8BIT_MASK           = {{'l','8','m','k'}};

static const icns_type_t  ICNS_16x16_1BIT_DATA           = {{'i','c','s','#'}};
static const icns_type_t  ICNS_16x16_4BIT_DATA           = {{'i','c','s','4'}};
static const icns_type_t  ICNS_16x16_8BIT_DATA           = {{'i','c','s','8'}};
static const icns_type_t  ICNS_16x16_32BIT_DATA          = {{'i','s','3','2'}};
static const icns_type_t  ICNS_16x16_1BIT_MASK           = {{'i','c','s','#'}};
static const icns_type_t  ICNS_16x16_8BIT_MASK           = {{'s','8','m','k'}};

static const icns_type_t  ICNS_16x12_1BIT_DATA           = {{'i','c','m','#'}};
static const icns_type_t  ICNS_16x12_4BIT_DATA           = {{'i','c','m','4'}};
static const icns_type_t  ICNS_16x12_1BIT_MASK           = {{'i','c','m','#'}};
static const icns_type_t  ICNS_16x12_8BIT_DATA           = {{'i','c','m','8'}};



static const icns_type_t  ICNS_32x32_1BIT_ICON           = {{'I','C','O','N'}};

static const icns_type_t  ICNS_NULL_DATA                 = {{ 0 , 0 , 0 , 0 }};
static const icns_type_t  ICNS_NULL_MASK                 = {{ 0 , 0 , 0 , 0 }};

/* icns file / resource type constants */

static const icns_type_t  ICNS_FAMILY_TYPE               = {{'i','c','n','s'}};

static const icns_type_t  ICNS_MACBINARY_TYPE            = {{'m','B','I','N'}};

static const icns_type_t  ICNS_NULL_TYPE                 = {{ 0 , 0 , 0 , 0 }};

/* icns error return values */

#define	ICNS_STATUS_OK                     0

#define	ICNS_STATUS_NULL_PARAM            -1
#define	ICNS_STATUS_NO_MEMORY             -2
#define	ICNS_STATUS_INVALID_DATA          -3

#define	ICNS_STATUS_IO_READ_ERR		   1
#define	ICNS_STATUS_IO_WRITE_ERR	   2
#define	ICNS_STATUS_ENCODING_ERR           3
#define	ICNS_STATUS_TYPE_NOT_FOUND         4
#define	ICNS_STATUS_UNSUPPORTED            5

/* icns function prototypes */
/* NOTE: internal functions are found in icns_internals.h */

// icns_io.c
int icns_write_family_to_file(FILE *dataFile,icns_family_t *iconFamilyIn);
int icns_read_family_from_file(FILE *dataFile,icns_family_t **iconFamilyOut);
int icns_export_family_data(icns_family_t *iconFamily,unsigned long *dataSizeOut,icns_byte_t **dataPtrOut);
int icns_import_family_data(unsigned long dataSize,icns_byte_t *data,icns_family_t **iconFamilyOut);

// icns_family.c
int icns_create_family(icns_family_t **iconFamilyOut);

// icns_element.c
int icns_get_element_from_family(icns_family_t *iconFamily,icns_type_t iconType,icns_element_t **iconElementOut);
int icns_set_element_in_family(icns_family_t **iconFamilyRef,icns_element_t *newIconElement);
int icns_add_element_in_family(icns_family_t **iconFamilyRef,icns_element_t *newIconElement);
int icns_remove_element_in_family(icns_family_t **iconFamilyRef,icns_type_t iconType);
int icns_new_element_from_icon_image(icns_image_t *imageIn,icns_type_t iconType,icns_element_t **iconElementOut);
int icns_new_element_from_mask_image(icns_image_t *imageIn,icns_type_t iconType,icns_element_t **iconElementOut);

// icns_image.c
int icns_get_image32_with_mask_from_family(icns_family_t *iconFamily,icns_type_t sourceType,icns_image_t *imageOut);
int icns_get_image_from_element(icns_element_t *iconElement,icns_image_t *imageOut);
int icns_get_mask_from_element(icns_element_t *iconElement,icns_image_t *imageOut);
int icns_init_image_for_type(icns_type_t iconType,icns_image_t *imageOut);
int icns_init_image(icns_uint32_t iconWidth,icns_uint32_t iconHeight,icns_uint32_t iconChannels,icns_uint32_t iconPixelDepth,icns_image_t *imageOut);
int icns_free_image(icns_image_t *imageIn);

// icns_rle24.c
int icns_decode_rle24_data(unsigned long dataInSize, icns_sint32_t *dataInPtr,unsigned long *dataOutSize, icns_sint32_t **dataOutPtr);
int icns_encode_rle24_data(unsigned long dataInSize, icns_sint32_t *dataInPtr,unsigned long *dataOutSize, icns_sint32_t **dataOutPtr);

// icns_jp2.c
#ifdef ICNS_OPENJPEG
int icns_opj_to_image(opj_image_t *image, icns_image_t *outIcon);
opj_image_t * jp2dec(icns_byte_t *bufin, int len);
#endif

// icns_utils.c
icns_type_t icns_get_mask_type_for_icon_type(icns_type_t);
void icns_set_print_errors(icns_bool_t shouldPrint);

#endif
