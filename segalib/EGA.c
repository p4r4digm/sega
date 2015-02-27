#include "EGA.h"
#include "segautils\BitBuffer.h"
#include "segashared\CheckedMemory.h"
#include "segautils\BitTwiddling.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

Frame *frameCreate() {
   return checkedCalloc(1, sizeof(Frame));
}
void frameDestroy(Frame *self) {
   checkedFree(self);
}

void _scanLineRenderImageScanLine(ScanLine *sl, short screenPos, byte *colorBuffer, byte *alphaBuffer, short bitOffset, short width) {
   int i, intRun, byteRun, alignRun, frac;
   int imgX = bitOffset;
   int x = screenPos;
   int last = x + width;

   int intBits = sizeof(uint32_t) * 8;
   int alignedBits = x % 8;
   uint32_t *screenArr, *imgArr, *alphaArr;

   if (alignedBits){
      while (x < last && alignedBits++ < 8) {
         //setbits...
         byte t = scanLineGetBit((ScanLine*)alphaBuffer, imgX);//trans
         byte c = scanLineGetBit((ScanLine*)colorBuffer, imgX);//c
         byte s = scanLineGetBit(sl, x);//screen

         //if alpha, use color, else use screen
         scanLineSetBit(sl, x, ((s & t) | c));

         ++x;
         ++imgX;
      }
   }

   if (x == last)  {
      return;
   }

   byteRun = (last - x ) / 8;
   alignRun = (4 - ((x % intBits) / 8)) % 4;
   if (alignRun < byteRun){
      byteRun = alignRun;
   }


   frac = imgX % 8;  //how far we are into the image 
   {
      //1 << 3 == 8
      uint8_t *screenArr = ((uint8_t*)sl->pixels) + (x >> 3);
      uint8_t *imgArr = ((uint8_t*)colorBuffer) + (imgX >> 3);
      uint8_t *alphaArr = ((uint8_t*)alphaBuffer) + (imgX >> 3);

      if (!frac) { //fast path!
         for (i = 0; i < byteRun; ++i, x += 8, imgX += 8) {
            //simply aligned, do regular int operations
            *screenArr &= *alphaArr++;
            *screenArr++ |= *imgArr++;
         }
      }
      else {//slow path...
         for (i = 0; i < byteRun; ++i, x += 8, imgX += 8) {
            //since this is unaligned, we do it in two slices.  Let's build the image bits into an aligned int32_t.
            *screenArr &= (*alphaArr >> frac) | ((*(alphaArr + 1)) << (8 - frac));
            *screenArr |= (*imgArr >> frac) | ((*(imgArr + 1)) << (8 - frac));

            ++imgArr;
            ++screenArr;
            ++alphaArr;
         }
      }

   }

   intRun = (last - x) / intBits;
   frac = imgX % intBits;  //how far we are into the image 

   //1 << 5 == 32
   screenArr = ((uint32_t*)sl->pixels) + (x >> 5);
   imgArr = ((uint32_t*)colorBuffer) + (imgX >> 5);
   alphaArr = ((uint32_t*)alphaBuffer) + (imgX >> 5);

   if (!frac) { //fast path!
      for (i = 0; i < intRun; ++i, x += intBits, imgX += intBits) {
         //simply aligned, do regular int operations
         *screenArr &= *alphaArr++;
         *screenArr++ |= *imgArr++;        
      }
   }
   else {//slow path...
      for (i = 0; i < intRun; ++i, x += intBits, imgX += intBits) {
         //since this is unaligned, we do it in two slices.  Let's build the image bits into an aligned int32_t.
         *screenArr &= (*alphaArr >> frac) | ((*(alphaArr + 1)) << (intBits - frac));
         *screenArr |= (*imgArr >> frac) | ((*(imgArr + 1)) << (intBits - frac));

         ++imgArr;
         ++screenArr;
         ++alphaArr;
      }
   }

   byteRun = (last - x) / 8;

   frac = imgX % 8;  //how far we are into the image 
   {
      //1 << 3 == 8
      uint8_t *screenArr = ((uint8_t*)sl->pixels) + (x >> 3);
      uint8_t *imgArr = ((uint8_t*)colorBuffer) + (imgX >> 3);
      uint8_t *alphaArr = ((uint8_t*)alphaBuffer) + (imgX >> 3);

      if (!frac) { //fast path!
         for (i = 0; i < byteRun; ++i, x += 8, imgX += 8) {
            //simply aligned, do regular int operations
            *screenArr &= *alphaArr++;
            *screenArr++ |= *imgArr++;
         }
      }
      else {//slow path...
         for (i = 0; i < byteRun; ++i, x += 8, imgX += 8) {
            //since this is unaligned, we do it in two slices.  Let's build the image bits into an aligned int32_t.
            *screenArr &= (*alphaArr >> frac) | ((*(alphaArr + 1)) << (8 - frac));
            *screenArr |= (*imgArr >> frac) | ((*(imgArr + 1)) << (8 - frac));

            ++imgArr;
            ++screenArr;
            ++alphaArr;
         }
      }

   }


   while (x < last) {
      //setbits...
      byte t = scanLineGetBit((ScanLine*)alphaBuffer, imgX);//trans
      byte c = scanLineGetBit((ScanLine*)colorBuffer, imgX);//c
      byte s = scanLineGetBit(sl, x);//screen

      //if alpha, use color, else use screen
      scanLineSetBit(sl, x, ((s & t) | c));
      ++x;
      ++imgX;
   }

}

ImageScanLine *createSmallestScanLine(short bitCount, byte *data){
   ImageScanLine *scanline = 0;
   scanline = createSolidScanLine(bitCount, data);
   if(!scanline) {
      scanline = createRLEScanLine(bitCount, data);
      if(!scanline) {
         scanline = createUncompressedScanLine(bitCount, data);
      }
   }

   return scanline;
}

void frameRenderImage(Frame *self, short x, short y, Image *img) {
   short imgWidth = imageGetWidth(img);
   short imgHeight = imageGetHeight(img);

   short clipSizeX, clipSizeY, ignoreOffsetX, ignoreOffsetY;
   int j, i;
   byte colorBuffer[MAX_IMAGE_WIDTH];
   byte alphaBuffer[MAX_IMAGE_WIDTH];

   ignoreOffsetX = x < 0 ? -x : 0;
   ignoreOffsetY = y < 0 ? -y : 0;

   clipSizeX = imgWidth;
   clipSizeY = imgHeight;
   if(clipSizeX + x > EGA_RES_WIDTH) clipSizeX = EGA_RES_WIDTH - x;
   if(clipSizeY + y > EGA_RES_HEIGHT) clipSizeY = EGA_RES_HEIGHT - y;

   clipSizeX -= ignoreOffsetX;
   clipSizeY -= ignoreOffsetY;

   if(clipSizeX <= 0 || clipSizeY <= 0) {
      return;
   }

   for(j = 0; j < clipSizeY; ++j) {
      imageScanLineRender(imageGetScanLine(img, j + ignoreOffsetY, 0), alphaBuffer);//transparency

      for(i = 0; i < EGA_PLANES; ++i) {
         imageScanLineRender(imageGetScanLine(img, j + ignoreOffsetY, i + 1), colorBuffer);

         _scanLineRenderImageScanLine(&self->planes[i].lines[j+y+ignoreOffsetY], x + ignoreOffsetX, 
            colorBuffer, alphaBuffer, ignoreOffsetX, clipSizeX);
      }
   }
}

void frameClear(Frame *self, byte color){
   int i;
   for (i = 0; i < EGA_PLANES; ++i) {
      byte current = getBit(color, i) ? 255 : 0;
      memset((byte*)(self->planes + i), current, EGA_BYTES);
   }
}

byte scanLineGetBit(ScanLine *self, short position) {
   return  getBitFromArray(self->pixels, position);
}

void scanLineSetBit(ScanLine *self, short position, byte value) {
   setBitInArray(self->pixels, position, value);
}

void buildEGAColorTable(int *table) {
   int i;
   //                  00 01  10   11
   byte rgbLookup[] = {0, 85, 170, 255};
   for(i = 0; i < EGA_COLORS; ++i) {
      byte shift = 5;

      byte r = getBit(i, shift--);
      byte g = getBit(i, shift--);
      byte b = getBit(i, shift--);
      byte R = getBit(i, shift--);
      byte G = getBit(i, shift--);
      byte B = getBit(i, shift);

      byte rgb_r = rgbLookup[(R << 1) + r];
      byte rgb_g = rgbLookup[(G << 1) + g];
      byte rgb_b = rgbLookup[(B << 1) + b];

      byte out[] = {rgb_r, rgb_g, rgb_b, 255};

      table[i] = *(int*)out;
   }
}

int getEGAColor(byte c) {
   static int lookup[EGA_COLORS] = {0};
   static int loaded = 0;

   if(!loaded) {
      buildEGAColorTable(lookup);
      loaded = 1;
   }

   return lookup[c];

}

Palette paletteDeserialize(const char *path) {
   long fSize;
   BitBuffer *buffer = bitBufferCreate(readFullFile(path, &fSize), 1);
   Palette p = {0};

   int i;

   if(fSize == 12) {
      for(i = 0; i < 16; ++i) {
         bitBufferReadBits(buffer, (byte*)&p+i, 6);
      }
   }
   
   bitBufferDestroy(buffer);
   return p;
}

void paletteSerialize(byte *data, const char *path) {
   BitBuffer *buff = bitBufferCreate(checkedCalloc(1, 12), 1);
   int i;
   FILE *out;
   for(i = 0; i < 16; ++i) {
      bitBufferWriteBits(buff, 6, data + i);
   }

   out = fopen(path, "wb");
   fwrite(bitBufferGetData(buff), sizeof(char), 12, out);
   fclose (out);
   bitBufferDestroy(buff);
}

Palette paletteCreatePartial(byte *data, byte pOffset, byte pCount, byte totalCount){
   Palette r;
   byte i;

   memset(r.colors, EGA_COLOR_UNUSED, EGA_PALETTE_COLORS);

   if(pOffset < 0 || 
      pOffset >= EGA_PALETTE_COLORS || 
      totalCount > EGA_PALETTE_COLORS || 
      pOffset + pCount > EGA_PALETTE_COLORS ||
      pCount < 0) {
         return r;
   }

   for(i = 0; i < totalCount; ++i){
      r.colors[i] = EGA_COLOR_UNDEFINED;
   }

   for(i = 0; i < pCount; ++i){
      r.colors[i] = data[pOffset+i];
   }

   return r;
}

Frame *buildCheckerboardFrame(int width, byte color1, byte color2) {
   Frame *fb = frameCreate();
   int x, y, j;

   for(y = 0; y < EGA_RES_HEIGHT; ++y){
      for(x = 0; x < EGA_RES_WIDTH;++x) {

         byte currentColor =  ((x / width) + (y/width)) % 2 ? color1 : color2;

         for(j = 0; j < EGA_PLANES; ++j) {
            scanLineSetBit(&fb->planes[j].lines[y], x, !!(currentColor&(1<<j)));
         }
      }
   }

   return fb;
}

