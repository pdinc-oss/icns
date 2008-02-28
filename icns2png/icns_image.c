/*
File:       icns_image.c
Copyright (C) 2001-2008 Mathew Eis <mathew@eisbox.net>
              2007 Thomas L�bking <thomas.luebking@web.de>
              2002 Chenxiao Zhao <chenxiao.zhao@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

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

#include "icns.h"

#include "endianswap.h"
#include "icns_colormaps.h"

icns_type_t icns_get_mask_type_for_icon_type(icns_type_t iconType)
{
	switch(iconType)
	{
		// Mask is already in ARGB 32-Bit icons
		case ICNS_512x512_32BIT_ARGB_DATA:
		case ICNS_256x256_32BIT_ARGB_DATA:
			return ICNS_INVALID_MASK;
		// 8-Bit masks for 32-Bit icons
		case ICNS_128X128_32BIT_DATA:
			return ICNS_128X128_8BIT_MASK;
		case ICNS_48x48_32BIT_DATA:
			return ICNS_48x48_8BIT_MASK;
		case ICNS_32x32_32BIT_DATA:
			return ICNS_32x32_8BIT_MASK;
		case ICNS_16x16_32BIT_DATA:
			return ICNS_16x16_8BIT_MASK;
		// 1-Bit masks for 1,4,8-Bit icons
		case ICNS_48x48_8BIT_DATA:
		case ICNS_48x48_4BIT_DATA:
		case ICNS_48x48_1BIT_DATA:
			return ICNS_48x48_1BIT_MASK;
		case ICNS_32x32_8BIT_DATA:
		case ICNS_32x32_4BIT_DATA:
		case ICNS_32x32_1BIT_DATA:
			return ICNS_32x32_1BIT_MASK;
		case ICNS_16x16_8BIT_DATA:
		case ICNS_16x16_4BIT_DATA:
		case ICNS_16x16_1BIT_DATA:
			return ICNS_16x16_1BIT_MASK;
		case ICNS_16x12_8BIT_DATA:
		case ICNS_16x12_4BIT_DATA:
		case ICNS_16x12_1BIT_DATA:
			return ICNS_16x12_1BIT_MASK;
		default:
			break;
	}
	return ICNS_INVALID_MASK;
}


int icns_get_image32_with_mask_from_family(icns_family_t *iconFamily,icns_type_t sourceType,icns_image_t *imageOut)
{
	int		error = 0;
	icns_type_t	iconType = 0x00000000;
	icns_type_t	maskType = 0x00000000;
	icns_element_t	*iconElement = NULL;
	icns_element_t	*maskElement = NULL;
	icns_image_t	iconImage;
	icns_image_t	maskImage;
	unsigned long	dataCount = 0;
	unsigned char	dataValue = 0;
	unsigned long	pixelCount = 0;
	unsigned long	pixelID = 0;
	unsigned char	colorIndex = 0;	
	
	memset ( &iconImage, 0, sizeof(icns_image_t) );
	memset ( &maskImage, 0, sizeof(icns_image_t) );
	
	if(iconFamily == NULL)
	{
		fprintf(stderr,"libicns: icns_get_image32_from_family: Icon element is NULL!\n");
		return -1;
	}
	
	if(imageOut == NULL)
	{
		fprintf(stderr,"libicns: icns_get_image32_from_family: Icon image structure is NULL!\n");
		return -1;
	}
	else
	{
		icns_free_image(imageOut);
	}
	
	if( (sourceType == ICNS_128X128_8BIT_MASK) || (sourceType == ICNS_48x48_8BIT_MASK) || (sourceType == ICNS_32x32_8BIT_MASK) || (sourceType == ICNS_16x16_8BIT_MASK) )
	{
		fprintf(stderr,"libicns: icns_get_image32_from_family: Can't make an image with mask from a mask\n");
		return -1;
	}

	// We use the jp2 processor for these two
	if((sourceType == ICNS_512x512_32BIT_ARGB_DATA) || (sourceType == ICNS_256x256_32BIT_ARGB_DATA))
	{
		#ifdef ICNS_OPENJPEG
		icns_size_t	elementSize;
		opj_image_t	*image = NULL;
		
		elementSize = EndianSwapBtoN( iconElement->elementSize, sizeof(icns_type_t) );
		image = jp2dec((unsigned char *)&(iconElement->elementData[0]), elementSize);
		if(image == NULL)
			return -1;
		
		error = icns_opj_to_image(image,imageOut);
		
		opj_image_destroy(image);
		
		return error;
		
		#else
		
		fprintf(stderr,"libicns: icns_get_image_from_element: libicns requires openjpeg for this data type!\n");
		return -1;
		
		#endif /* ifdef ICNS_OPENJPEG */
	}
	
	iconType = sourceType;
	maskType = icns_get_mask_type_for_icon_type(iconType);
	
	#ifdef ICNS_DEBUG
	fprintf(stderr,"Making 32-bit image...\n");
	fprintf(stderr,"  using icon type 0x%08X\n",iconType);
	fprintf(stderr,"  using mask type 0x%08X\n",maskType);
	#endif
	
	if( maskType == ICNS_INVALID_MASK )
	{
		fprintf(stderr,"libicns: icns_get_image32_from_family: Can't find mask for type 0x%08X\n",iconType);
		return -1;
	}
	
	// Preliminaries checked - carry on with the icon/mask merge
	
	// Load icon element then image
	error = icns_get_element_from_family(iconFamily,iconType,&iconElement);
	
	if(error) {
		fprintf(stderr,"libicns: icns_get_image32_from_family: Unable to load icon element from icon family!\n");
		goto cleanup;
	}
	
	error = icns_get_image_from_element(iconElement,&iconImage);
	
	if(error) {
		fprintf(stderr,"libicns: icns_get_image32_from_family: Unable to load icon image data from icon element!\n");
		goto cleanup;
	}
	
	// Load mask element then image
	error = icns_get_element_from_family(iconFamily,maskType,&maskElement);

	if(error) {
		fprintf(stderr,"libicns: icns_get_image32_from_family: Unable to load mask element from icon family!\n");
		goto cleanup;
	}
	
	error = icns_get_mask_from_element(maskElement,&maskImage);
	
	if(error) {
		fprintf(stderr,"libicns: icns_get_image32_from_family: Unable to load mask image data from icon element!\n");
		goto cleanup;
	}
	
	if(iconImage.imageWidth != maskImage.imageWidth) {
		fprintf(stderr,"libicns: icns_get_image32_from_family: icon and mask widths do not match!\n");
		goto cleanup;
	}
	
	if(iconImage.imageHeight != maskImage.imageHeight) {
		fprintf(stderr,"libicns: icns_get_image32_from_family: icon and mask heights do not match!\n");
		goto cleanup;
	}
	
	// Unpack image pixels if depth is < 32
	if((iconImage.pixel_depth * iconImage.imageChannels) < 32)
	{
		unsigned char	*oldData = NULL;
		unsigned char	*newData = NULL;
		unsigned int	oldBitDepth = 0;
		unsigned int	oldDataSize = 0;
		unsigned long	newBlockSize = 0;
		unsigned long	newDataSize = 0;
		icns_colormap_rgb_t	colorRGB;
		
		oldBitDepth = (iconImage.pixel_depth * iconImage.imageChannels);
		oldDataSize = iconImage.imageDataSize;
		
		pixelCount = iconImage.imageWidth * iconImage.imageHeight;
		
		newBlockSize = iconImage.imageWidth * 32;
		newDataSize = newBlockSize * iconImage.imageHeight;
		
		oldData = iconImage.imageData;
		newData = (unsigned char *)malloc(newDataSize);
		
		if(newData == NULL)
		{
			fprintf(stderr,"libicns: icns_get_image32_from_family: Unable to allocate memory block of size: %d!\n",(int)newDataSize);
			return -1;
		}
		
		dataCount = 0;
		
		switch(iconType)
		{
			// 8-Bit Icon Image Data Types
			case ICNS_48x48_8BIT_DATA:
			case ICNS_32x32_8BIT_DATA:
			case ICNS_16x16_8BIT_DATA:
			case ICNS_16x12_8BIT_DATA:
				if(oldBitDepth != 8)
				{
					fprintf(stderr,"libicns: icns_get_image32_from_family: Icon bit depth type mismatch!\n");
					free(newData);
					error = -1;
					goto cleanup;
				}
				for(pixelID = 0; pixelID < pixelCount; pixelID++)
				{
					colorIndex = oldData[dataCount++];
					colorRGB = icns_colormap_8[colorIndex];
					newData[pixelID * 4 + 0] = colorRGB.r;
					newData[pixelID * 4 + 1] = colorRGB.g;
					newData[pixelID * 4 + 2] = colorRGB.b;
					newData[pixelID * 4 + 3] = 0xff;
				}
				break;
			// 4-Bit Icon Image Data Types
			case ICNS_48x48_4BIT_DATA:
			case ICNS_32x32_4BIT_DATA:
			case ICNS_16x16_4BIT_DATA:
			case ICNS_16x12_4BIT_DATA:
				if(oldBitDepth != 4)
				{
					fprintf(stderr,"libicns: icns_get_image32_from_family: Icon bit depth type mismatch!\n");
					free(newData);
					error = -1;
					goto cleanup;
				}
				for(pixelID = 0; pixelID < pixelCount; pixelID++)
				{
					if(pixelID % 2 == 0)
						dataValue = oldData[dataCount++];
					colorIndex = (dataValue & 0xF0) >> 4;
					dataValue = dataValue << 4;
					colorRGB = icns_colormap_4[colorIndex];
					newData[pixelID * 4 + 0] = colorRGB.r;
					newData[pixelID * 4 + 1] = colorRGB.g;
					newData[pixelID * 4 + 2] = colorRGB.b;
					newData[pixelID * 4 + 3] = 0xFF;
				}
				break;
			// 1-Bit Icon Image Data Types
			case ICNS_48x48_1BIT_DATA:
			case ICNS_32x32_1BIT_DATA:
			case ICNS_16x16_1BIT_DATA:
			case ICNS_16x12_1BIT_DATA:
				if(oldBitDepth != 1)
				{
					fprintf(stderr,"libicns: icns_get_image32_from_family: Icon bit depth type mismatch!\n");
					free(newData);
					error = -1;
					goto cleanup;
				}
				for(pixelID = 0; pixelID < pixelCount; pixelID++)
				{
					if(pixelID % 8 == 0)
						dataValue = oldData[dataCount++];
					colorIndex = (dataValue & 0x80) ? 0x00 : 0xFF;
					dataValue = dataValue << 1;
					newData[pixelID * 4 + 0] = colorIndex;
					newData[pixelID * 4 + 1] = colorIndex;
					newData[pixelID * 4 + 2] = colorIndex;
					newData[pixelID * 4 + 3] = 0xFF;
				}
				break;
			default:
				fprintf(stderr,"libicns: icns_get_image32_from_family: Unpack error - unknown icon type!\n");
				free(newData);
				error = -1;
				goto cleanup;
				break;
		}
		
		iconImage.pixel_depth = 8;
		iconImage.imageChannels = 4;
		iconImage.imageDataSize = newDataSize;
		iconImage.imageData = newData;
		
		free(oldData);
	}
	
	// Copy in the mask, unpacking it if we need to
	switch(maskType)
	{
		// 8-Bit Icon Mask Data Types
		case ICNS_128X128_8BIT_MASK:
		case ICNS_48x48_8BIT_MASK:
		case ICNS_32x32_8BIT_MASK:
		case ICNS_16x16_8BIT_MASK:
			pixelCount = maskImage.imageWidth * maskImage.imageHeight;
			dataCount = 0;
			if((maskImage.pixel_depth * maskImage.imageChannels) != 8)
			{
				fprintf(stderr,"libicns: icns_get_image32_from_family: Mask bit depth type mismatch!\n");
				error = -1;
				goto cleanup;
			}
			for(pixelID = 0; pixelID < pixelCount; pixelID++)
			{
				iconImage.imageData[pixelID * 4 + 3] = maskImage.imageData[dataCount++];
			}
			break;
		// 1-Bit Icon Mask Data Types
		case ICNS_48x48_1BIT_MASK:
		case ICNS_32x32_1BIT_MASK:
		case ICNS_16x16_1BIT_MASK:
		case ICNS_16x12_1BIT_MASK:
			pixelCount = maskImage.imageWidth * maskImage.imageHeight;
			dataCount = 0;
			if((maskImage.pixel_depth * maskImage.imageChannels) != 1)
			{
				fprintf(stderr,"libicns: icns_get_image32_from_family: Mask bit depth type mismatch!\n");
				error = -1;
				goto cleanup;
			}
			for(pixelID = 0; pixelID < pixelCount; pixelID++)
			{
				if(pixelID % 8 == 0)
					dataValue = maskImage.imageData[dataCount++];
				colorIndex = (dataValue & 0x80) ? 0xFF : 0x00;
				dataValue = dataValue << 1;
				iconImage.imageData[pixelID * 4 + 3] = colorIndex;
			}
			break;
		default:			
			fprintf(stderr,"libicns: icns_get_image32_from_family: Unpack error - unknown mask type!\n");
			error = -1;
			goto cleanup;
			break;
	}
	
cleanup:
	
	if(maskElement != NULL) {
		free(maskElement);
		maskElement = NULL;
	}
	icns_free_image(&maskImage);
	
	if(iconElement != NULL) {
		free(iconElement);
		iconElement = NULL;
	}
	
	// We only free the icon image if there was an error. Otherwise, we
	// pass the data onto the outgoing image
	if(error) {
		icns_free_image(&iconImage);
	} else {
		memcpy(imageOut,&iconImage,sizeof(icns_image_t));
		#ifdef ICNS_DEBUG
		printf("Finished 32-bit image...\n");
		printf("  height: %d\n",imageOut->imageHeight);
		printf("  width: %d\n",imageOut->imageHeight);
		printf("  channels: %d\n",imageOut->imageChannels);
		printf("  pixel depth: %d\n",imageOut->pixel_depth);
		printf("  data size: %d\n",(int)(imageOut->imageDataSize));
		#endif
	}
	
	return error;
}


//***************************** icns_get_image_from_element **************************//
// Actual conversion of the icon data into uncompressed raw pixels

int icns_get_image_from_element(icns_element_t *iconElement,icns_image_t *imageOut)
{
	int		error = 0;
	unsigned long	dataCount = 0;
	icns_type_t	elementType = 0x00000000;
	icns_size_t	elementSize = 0;
	icns_type_t	iconType = 0x00000000;
	unsigned long	rawDataSize = 0;
	unsigned char	*rawDataPtr = NULL;
	unsigned int	iconBitDepth = 0;
	unsigned long	iconDataSize = 0;
	unsigned long	iconDataRowSize = 0;
	
	if(iconElement == NULL)
	{
		fprintf(stderr,"libicns: icns_get_image_from_element: Icon element is NULL!\n");
		return -1;
	}
	
	if(imageOut == NULL)
	{
		fprintf(stderr,"libicns: icns_get_image_from_element: Icon image structure is NULL!\n");
		return -1;
	}
	
	elementType = iconElement->elementType;
	elementSize = iconElement->elementSize;
	elementType = EndianSwapBtoN( elementType, sizeof(icns_type_t) );
	elementSize = EndianSwapBtoN( elementSize, sizeof(icns_size_t) );
	
	#if ICNS_DEBUG
	printf("Retreiving image from icon element...\n");
	printf("  type is: 0x%8X\n",(unsigned int)elementType);
	printf("  size is: %d\n",(int)elementSize);	
	#endif

	iconType = elementType;
	rawDataSize = elementSize - sizeof(icns_type_t) - sizeof(icns_size_t);
	rawDataPtr = (unsigned char*)&(iconElement->elementData[0]);
	
	#if ICNS_DEBUG
	printf("  data size is: %d\n",(int)rawDataSize);	
	#endif

	switch(iconType)
	{
		// 32-Bit Icon Image Data Types ( > 256px )
		case ICNS_512x512_32BIT_ARGB_DATA:
		case ICNS_256x256_32BIT_ARGB_DATA:
			// We use the jp2 processor for these two
			{
				#ifdef ICNS_OPENJPEG
	
				opj_image_t* image = NULL;
	
				image = jp2dec((unsigned char *)rawDataPtr, (int)rawDataSize);
				if(!image)
					return -1;
	
				error = icns_opj_to_image(image,imageOut);
	
				opj_image_destroy(image);
	
				return error;
	
				#else
	
				fprintf(stderr,"libicns: icns_get_image_from_element: libicns requires openjpeg for this data type!\n");
				icns_free_image(imageOut);
				return -1;
	
				#endif
			}
			break;
		// 32-Bit Icon Image Data Types
		case ICNS_128X128_32BIT_DATA:
		case ICNS_48x48_32BIT_DATA:
		case ICNS_32x32_32BIT_DATA:
		case ICNS_16x16_32BIT_DATA:
			{
				error = icns_init_image_for_type(iconType,imageOut);
				
				if(error)
				{
					fprintf(stderr,"libicns: icns_get_image_from_element: Error allocating new icns image!\n");
					icns_free_image(imageOut);
					return -1;
				}
				
				iconBitDepth = imageOut->pixel_depth * imageOut->imageChannels;
				iconDataSize = imageOut->imageDataSize;
				iconDataRowSize = imageOut->imageWidth * iconBitDepth / icns_byte_bits;
				
				if(rawDataSize < imageOut->imageDataSize)
				{
					unsigned long	rleDataSize = 0;
					icns_sint32_t	*rleDataPtr = NULL;
					rleDataSize = iconDataSize;
					rleDataPtr = (icns_sint32_t*)(imageOut->imageData);
					icns_decode_rle24_data(rawDataSize,(icns_sint32_t*)rawDataPtr,&rleDataSize,&rleDataPtr);
				}
				else
				{
					unsigned long	pixelCount = 0;
					icns_byte_t	*swapPtr = NULL;
					icns_dword_t	*pixelPtr = NULL;

					for(dataCount = 0; dataCount < imageOut->imageHeight; dataCount++)
						memcpy(&(((char*)(imageOut->imageData))[dataCount*iconDataRowSize]),&(((char*)(rawDataPtr))[dataCount*iconDataRowSize]),iconDataRowSize);
					
					pixelCount = imageOut->imageWidth * imageOut->imageHeight;
					#ifdef ICNS_DEBUG
						printf("Converting %d pixels from argb to rgba\n",(int)pixelCount);
					#endif
					swapPtr = imageOut->imageData;
					for(dataCount = 0; dataCount < pixelCount; dataCount++)
					{
						pixelPtr = (icns_dword_t *)(swapPtr + (dataCount * 4));
						*((icns_rgba_t *)pixelPtr) = ICNS_ARGB_TO_RGBA( *((icns_argb_t *)pixelPtr) );
					}
				}
			}
			break;
		// 8-Bit Icon Image Data Types
		case ICNS_48x48_8BIT_DATA:
		case ICNS_32x32_8BIT_DATA:
		case ICNS_16x16_8BIT_DATA:
		case ICNS_16x12_8BIT_DATA:
		// 4-Bit Icon Image Data Types
		case ICNS_48x48_4BIT_DATA:
		case ICNS_32x32_4BIT_DATA:
		case ICNS_16x16_4BIT_DATA:
		case ICNS_16x12_4BIT_DATA:
		// 1-Bit Icon Image Data Types
		case ICNS_48x48_1BIT_DATA:
		case ICNS_32x32_1BIT_DATA:
		case ICNS_16x16_1BIT_DATA:
		case ICNS_16x12_1BIT_DATA:
			{
				error = icns_init_image_for_type(iconType,imageOut);
				
				if(error)
				{
					fprintf(stderr,"libicns: icns_get_image_from_element: Error allocating new icns image!\n");
					icns_free_image(imageOut);
					return -1;
				}
				
				iconBitDepth = imageOut->pixel_depth * imageOut->imageChannels;
				iconDataSize = imageOut->imageDataSize;
				iconDataRowSize = imageOut->imageWidth * iconBitDepth / icns_byte_bits;
				
				for(dataCount = 0; dataCount < imageOut->imageHeight; dataCount++)
					memcpy(&(((char*)(imageOut->imageData))[dataCount*iconDataRowSize]),&(((char*)(rawDataPtr))[dataCount*iconDataRowSize]),iconDataRowSize);
			}
			break;
		default:
			fprintf(stderr,"libicns: icns_get_image_from_element: Unknown icon type 0x%08X!\n",iconType);
			icns_free_image(imageOut);
			return -1;
			break;
	}
	
	return error;
}

//***************************** icns_get_mask_from_element **************************//
// Actual conversion of the mask data into uncompressed raw pixels

int icns_get_mask_from_element(icns_element_t *maskElement,icns_image_t *imageOut)
{
	int		error = 0;
	unsigned long	dataCount = 0;
	icns_type_t	elementType = 0x00000000;
	icns_size_t	elementSize = 0;
	icns_type_t	maskType = 0x00000000;
	unsigned long	rawDataSize = 0;
	unsigned char	*rawDataPtr = NULL;
	unsigned int	maskBitDepth = 0;
	unsigned long	maskDataSize = 0;
	unsigned long	maskDataRowSize = 0;
			
	if(maskElement == NULL)
	{
		fprintf(stderr,"libicns: icns_get_mask_from_element: Mask element is NULL!\n");
		return -1;
	}
	
	if(imageOut == NULL)
	{
		fprintf(stderr,"libicns: icns_get_mask_from_element: Mask image structure is NULL!\n");
		return -1;
	}
	
	elementType = maskElement->elementType;
	elementSize = maskElement->elementSize;
	elementType = EndianSwapBtoN( elementType, sizeof(icns_type_t) );
	elementSize = EndianSwapBtoN( elementSize, sizeof(icns_size_t) );
	
	#if ICNS_DEBUG
	printf("Retreiving image from mask element...\n");
	printf("  type is: 0x%8X\n",(unsigned int)elementType);
	printf("  size is: %d\n",(int)elementSize);	
	#endif

	maskType = elementType;
	rawDataSize = elementSize - sizeof(icns_type_t) - sizeof(icns_size_t);
	rawDataPtr = (unsigned char*)&(maskElement->elementData[0]);
	
	#if ICNS_DEBUG
	printf("  data size is: %d\n",(int)rawDataSize);	
	#endif

	switch(maskType)
	{
		// 8-Bit Mask Image Data Types
		case ICNS_128X128_8BIT_MASK:
		case ICNS_48x48_8BIT_MASK:
		case ICNS_32x32_8BIT_MASK:
		case ICNS_16x16_8BIT_MASK:
			{
				error = icns_init_image_for_type(maskType,imageOut);
				
				if(error)
				{
					fprintf(stderr,"libicns: icns_get_mask_from_element: Error allocating new icns image!\n");
					icns_free_image(imageOut);
					return -1;
				}
				
				maskBitDepth = imageOut->pixel_depth * imageOut->imageChannels;
				
				if(maskBitDepth != 8) {
					fprintf(stderr,"libicns: icns_get_mask_from_element: Unknown bit depth!\n");
					icns_free_image(imageOut);
					return -1;
				}
				
				maskDataSize = imageOut->imageDataSize;
				maskDataRowSize = imageOut->imageWidth * maskBitDepth / icns_byte_bits;
				
				for(dataCount = 0; dataCount < imageOut->imageHeight; dataCount++)
					memcpy(&(((char*)(imageOut->imageData))[dataCount*maskDataRowSize]),&(((char*)(rawDataPtr))[dataCount*maskDataRowSize]),maskDataRowSize);
			}
			break;
		// 1-Bit Mask Image Data Types
		case ICNS_48x48_1BIT_MASK:
		case ICNS_32x32_1BIT_MASK:
		case ICNS_16x16_1BIT_MASK:
		case ICNS_16x12_1BIT_MASK:
			{
				error = icns_init_image_for_type(maskType,imageOut);
	
				if(error)
				{
					fprintf(stderr,"libicns: icns_get_mask_from_element: Error allocating new icns image!\n");
					icns_free_image(imageOut);
					return -1;
				}
	
				maskBitDepth = imageOut->pixel_depth * imageOut->imageChannels;
				
				if(maskBitDepth != 1) {
					fprintf(stderr,"libicns: icns_get_mask_from_element: Unknown bit depth!\n");
					icns_free_image(imageOut);
					return -1;
				}
				
				maskDataSize = imageOut->imageDataSize;
				maskDataRowSize = imageOut->imageWidth * maskBitDepth / icns_byte_bits;
				
				#if ICNS_DEBUG
				printf("  raw mask data size is: %d\n",(int)rawDataSize);	
				printf("  image mask data size is: %d\n",(int)maskDataSize);	
				#endif

				if(rawDataSize == (maskDataSize * 2) )
				{
					#if ICNS_DEBUG
					printf("  mask data in second memory block\n");	
					#endif					
					// Mask data found - Copy the second block of memory
					for(dataCount = 0; dataCount < imageOut->imageHeight; dataCount++)
						memcpy(&(((char*)(imageOut->imageData))[dataCount*maskDataRowSize]),&(((char*)(rawDataPtr))[dataCount*maskDataRowSize+maskDataSize]),maskDataRowSize);
				}
				else
				{
					#if ICNS_DEBUG
					printf("  using icon data from first memory block\n");	
					#endif					
					// Hmm, no mask - copy the first block of memory
					for(dataCount = 0; dataCount < imageOut->imageHeight; dataCount++)
						memcpy(&(((char*)(imageOut->imageData))[dataCount*maskDataRowSize]),&(((char*)(rawDataPtr))[dataCount*maskDataRowSize]),maskDataRowSize);
				}
			}
			break;
		default:
			fprintf(stderr,"libicns: icns_get_mask_from_element: Unknown mask type 0x%08X!\n",maskType);
			icns_free_image(imageOut);
			return -1;
			break;
	}
	
	
	return error;
}



//***************************** icns_init_image_for_type **************************//
// Initialize a new image structure for holding the data
// using the information for the specified type

int icns_init_image_for_type(icns_type_t iconType,icns_image_t *imageOut)
{
	unsigned int	iconWidth = 0;
	unsigned int	iconHeight = 0;
	unsigned int	iconChannels = 0;
	unsigned int	iconBitDepth = 0;
	
	if(imageOut == NULL)
	{
		fprintf(stderr,"libicns: icns_init_image_for_type: Icon image structure is NULL!\n");
		return -1;
	}
	
	switch(iconType)
	{
		// 32-Bit Icon Image Data Types
		case ICNS_128X128_32BIT_DATA:
			iconWidth = 128;
			iconHeight = 128;
			iconChannels = 4;
			iconBitDepth = 32;
			break;
		case ICNS_48x48_32BIT_DATA:
			iconWidth = 48;
			iconHeight = 48;
			iconChannels = 4;
			iconBitDepth = 32;
			break;
		case ICNS_32x32_32BIT_DATA:
			iconWidth = 32;
			iconHeight = 32;
			iconChannels = 4;
			iconBitDepth = 32;
			break;
		case ICNS_16x16_32BIT_DATA:
			iconWidth = 16;
			iconHeight = 16;
			iconChannels = 4;
			iconBitDepth = 32;
			break;
			
		// 8-Bit Icon Mask Data Types
		case ICNS_128X128_8BIT_MASK:
			iconWidth = 128;
			iconHeight = 128;
			iconChannels = 1;
			iconBitDepth = 8;
			break;
		case ICNS_48x48_8BIT_MASK:
			iconWidth = 48;
			iconHeight = 48;
			iconChannels = 1;
			iconBitDepth = 8;
			break;
		case ICNS_32x32_8BIT_MASK:
			iconWidth = 32;
			iconHeight = 32;
			iconChannels = 1;
			iconBitDepth = 8;
			break;
		case ICNS_16x16_8BIT_MASK:
			iconWidth = 16;
			iconHeight = 16;
			iconChannels = 1;
			iconBitDepth = 8;
			break;
			
		// 8-Bit Icon Image Data Types
		case ICNS_48x48_8BIT_DATA:
			iconWidth = 48;
			iconHeight = 48;
			iconChannels = 1;
			iconBitDepth = 8;
			break;
		case ICNS_32x32_8BIT_DATA:
			iconWidth = 32;
			iconHeight = 32;
			iconChannels = 1;
			iconBitDepth = 8;
			break;
		case ICNS_16x16_8BIT_DATA:
			iconWidth = 16;
			iconHeight = 16;
			iconChannels = 1;
			iconBitDepth = 8;
			break;
		case ICNS_16x12_8BIT_DATA:
			iconWidth = 16;
			iconHeight = 12;
			iconChannels = 1;
			iconBitDepth = 8;
			break;

		// 4-Bit Icon Image Data Types
		case ICNS_48x48_4BIT_DATA:
			iconWidth = 48;
			iconHeight = 48;
			iconChannels = 1;
			iconBitDepth = 4;
			break;
		case ICNS_32x32_4BIT_DATA:
			iconWidth = 32;
			iconHeight = 32;
			iconChannels = 1;
			iconBitDepth = 4;
			break;
		case ICNS_16x16_4BIT_DATA:
			iconWidth = 16;
			iconHeight = 16;
			iconChannels = 1;
			iconBitDepth = 4;
			break;
		case ICNS_16x12_4BIT_DATA:
			iconWidth = 16;
			iconHeight = 12;
			iconChannels = 1;
			iconBitDepth = 4;
			break;

		// 1-Bit Icon Image/Mask Data Types (Data is the same)
		// WARNING: The first 128 bytes are icon data. This may
		// or may not be followed by 128 more bytes of mask data.
		// it is important to note this on the receiving end
		// of this format of image data.
		case ICNS_48x48_1BIT_DATA:  // Also ICNS_48x48_1BIT_MASK
			iconWidth = 48;
			iconHeight = 48;
			iconChannels = 1;
			iconBitDepth = 1;
			break;
		case ICNS_32x32_1BIT_DATA: // Also ICNS_32x32_1BIT_MASK
			iconWidth = 32;
			iconHeight = 32;
			iconChannels = 1;
			iconBitDepth = 1;
			break;
		case ICNS_16x16_1BIT_DATA: // Also ICNS_16x16_1BIT_MASK
			iconWidth = 16;
			iconHeight = 16;
			iconChannels = 1;
			iconBitDepth = 1;
			break;
		case ICNS_16x12_1BIT_DATA: // Also ICNS_16x12_1BIT_MASK
			iconWidth = 16;
			iconHeight = 12;
			iconChannels = 1;
			iconBitDepth = 1;
			break;
			
		default:
			fprintf(stderr,"libicns: icns_init_image_for_type: Unable to parse icon type 0x%8X\n",iconType);
			return -1;
			break;
	}
	
	return icns_init_image(iconWidth,iconHeight,iconChannels,(iconBitDepth / iconChannels),imageOut);

}

//***************************** icns_init_image **************************//
// Initialize a new image structure for holding the data

int icns_init_image(unsigned int iconWidth,unsigned int iconHeight,unsigned int iconChannels,unsigned int iconPixelDepth,icns_image_t *imageOut)
{
	unsigned int	iconBitDepth = 0;
	unsigned long	iconDataSize = 0;
	unsigned long	iconDataRowSize = 0;

	iconBitDepth = iconPixelDepth * iconChannels;
	iconDataRowSize = iconWidth * iconBitDepth / icns_byte_bits;
	iconDataSize = iconHeight * iconDataRowSize;
	
	#ifdef ICNS_DEBUG
	printf("Initializing new image...\n");
	printf("  width is: %d\n",iconWidth);
	printf("  height is: %d\n",iconHeight);
	printf("  channels are: %d\n",iconChannels);
	printf("  bit depth is: %d\n",iconBitDepth);
	printf("  data size is: %d\n",(int)iconDataSize);
	#endif
	
	imageOut->imageWidth = iconWidth;
	imageOut->imageHeight = iconHeight;
	imageOut->imageChannels = iconChannels;
	imageOut->pixel_depth = (iconBitDepth / iconChannels);
	imageOut->imageDataSize = iconDataSize;
	imageOut->imageData = (unsigned char *)malloc(iconDataSize);
	if(!imageOut->imageData)
	{
		fprintf(stderr,"libicns: icns_init_image: Unable to allocate memory block of size: %d ($s:%m)!\n",(int)iconDataSize);
		return -1;
	}
	memset(imageOut->imageData,0,iconDataSize);
	
	return 0;
}

//***************************** icns_free_image **************************//
// Deallocate data in an image structure

int icns_free_image(icns_image_t *imageIn)
{
	imageIn->imageWidth = 0;
	imageIn->imageHeight = 0;
	imageIn->imageChannels = 0;
	imageIn->pixel_depth = 0;
	imageIn->imageDataSize = 0;
	
	if(imageIn->imageData != NULL)
	{
		free(imageIn->imageData);
		imageIn->imageData = NULL;
	}
	
	return 0;
}

