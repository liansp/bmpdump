
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
struct BitMapInfo{
    uint16_t magic;
    uint32_t size;
    uint32_t unused;
    uint32_t offset_data;

    uint32_t header_bytes;
    int32_t width;
    int32_t height;
    uint16_t color_planes;
    uint16_t color_bpp;
    uint32_t compression;
    uint32_t data_size;
    int32_t  h_resolution;
    int32_t  v_resolution;
    uint32_t colors_palette;
    uint32_t mean_palette;
};
#pragma pack(pop)

static void drawBmp(const char *filename)
{
	FILE* file;
	struct BitMapInfo header;

	uint32_t rowSize;
	uint8_t *palette=NULL;
	uint8_t  *buffer=NULL;
	uint32_t bufferidx;
	uint16_t color;
	uint8_t  r, g, b;
	int w, h, x, y;

	if ((file = fopen(filename, "rb")) == NULL) return;

	if (fread(&header, sizeof(uint8_t), sizeof(header), file) != sizeof(header)) {
		goto bmp_end;
	}
	if (header.magic != 0x4D42) //BMP signature
		goto bmp_end;
	if (!(header.color_planes == 1 && header.compression == 0))
		goto bmp_end;

	if (header.colors_palette != 0) {
		palette = (uint8_t*)malloc(header.colors_palette * 4); //BGRA
		fread(palette, sizeof(uint8_t), header.colors_palette * 4, file);
	}

	// BMP rows are padded (if needed) to 4-byte boundary
	rowSize = (((header.width * header.color_bpp) + 31) >> 5) << 2;
	buffer = (uint8_t*)malloc(header.height * rowSize);
	if (buffer == NULL)
		goto bmp_end;
	fseek(file, header.offset_data, SEEK_SET);
	fread(buffer, sizeof(uint8_t), header.height * rowSize, file);

	// Crop area to be loaded
	w = header.width;
	h = header.height;

	for (y=0; y<h; y++) {
		// Bitmap is stored bottom-to-top order (normal BMP)
		bufferidx = (header.height - 1 - y) * rowSize;
		for (x=0; x<w; x++) {
			switch(header.color_bpp)
			{
			case 8: //use palette
				if (palette != NULL)
				{
					color = buffer[bufferidx++];
					b = palette[color*4];
					g = palette[color*4+1];
					r = palette[color*4+2];
					//lcdbuffer[lcdidx++] = rgb24to16(r,g,b);
				}
				break;
			case 16: //RGB555
				color = buffer[bufferidx] | (buffer[bufferidx+1]<<8);
				bufferidx++;
				bufferidx++;
				b = color & 0x001F;
				g = (color & 0x03E0) >> 5;
				r = (color & 0x7C00) >> 10;
				//lcdbuffer[lcdidx++] = torgb16(r,g,b);
				break;
			case 24: //RGB888
				b = buffer[bufferidx++];
				g = buffer[bufferidx++];
				r = buffer[bufferidx++];
				//lcdbuffer[lcdidx++] = rgb24to16(r,g,b);
				break;
			case 32: //ARGB8888
				b = buffer[bufferidx++];
				g = buffer[bufferidx++];
				r = buffer[bufferidx++];
				bufferidx++;
				//lcdbuffer[lcdidx++] = rgb24to16(r,g,b);
				break;
			}
            printf("0x%02X,0x%02X,0x%02X, ", r, g, b);
		}
        printf("\n");
	}

bmp_end:
	if (palette != NULL) free(palette);
	if (buffer != NULL) free(buffer);
	fclose(file);
}

int main(int argc,char *argv[])
{
    if (argc > 1) {
        drawBmp(argv[1]);
    }
}

