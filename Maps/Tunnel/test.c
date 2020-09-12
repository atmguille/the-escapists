#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
} RGB;

#define HEADER_MIN_SIZE 54

typedef struct {
    unsigned short width;
    unsigned short heigth;
    RGB** rgb;
} Image;


Image* image_ini(const char* filename) {
    FILE* f;
    unsigned char header[HEADER_MIN_SIZE]; /* The header is at least HEADER_MIN_SIZE bytes long */
    unsigned char* data; /* The data depends on the image size, so we'll allocate memory */
    int i, j;
    unsigned short width;
    unsigned short heigth;
    unsigned short bpp; /* Bytes per pixel, usually 3 */
    unsigned short headerSize; /* The header is not usually just HEADER_MIN_SIZE bytes long, but longer */
    unsigned short rowSize; /* It MUST be a multiple of 4, because of allignment */
    Image* image;

    /* Open file */
    f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "[!] The file %s does not exist\n", filename);
        return NULL;
    }
    if (f == NULL)
        return NULL;
    if (fread(header, sizeof(unsigned char), HEADER_MIN_SIZE,f) != HEADER_MIN_SIZE) {
        /* If they file doesn't have at least HEADER_MIN_SIZE bytes, there's an error */
        fclose(f);
        return NULL;
    }

    /* Read header parameters */
    if (header[0] != 'B' || header[1] != 'M') {
        /* If the first two bytes aren't BM, this file is not BMP */
        fclose(f);
        return NULL;
    }
    width = *(int *)&header[18];
    heigth = abs(*(int *)&header[22]);
    bpp = header[28] / 8;
    headerSize = *(int *)&header[10];
    rowSize = ((width * bpp - 1)|3) + 1; /* 4 byte alignment */
    /* Go forward to end of header */
    fseek(f, headerSize, 0);

    /* Create image struct */
    image = (Image *) malloc(sizeof(Image));
    if (image == NULL) {
        fclose(f);
        return NULL;
    }
    image->rgb = malloc(heigth * sizeof(RGB *));
    if (image->rgb == NULL) {
        fclose(f);
        free(image);
        return NULL;
    }
    for (i = 0; i < heigth; i++) {
        image->rgb[i] = (RGB *) malloc(width * sizeof(RGB));
        if (image->rgb[i] == NULL) {
            for (i = i - 1; i >= 0; i--)
                free(image->rgb[i]);
            free(image->rgb);
            free(image);
            fclose(f);
            return NULL;
        }
    }

    /* Create reading array (data) and populate the struct */
    data = (unsigned char *) malloc(sizeof(unsigned char) * rowSize);

    for (i = 0; i < heigth; i++) {
        fread(data, sizeof(unsigned char), rowSize, f);
        for (j = 0; j < width; j++) {
            /* The file has data upside down, thus, we use height - i - 1 so we don't get flipped images */
            /* The width * bpp is not a multiple of four, we'll read up to 3 bytes more we won't use */
            /* If the image had 32 bit per pixel (4 bpp), the data[j * bpp + 3] would give us white */
            image->rgb[heigth - i - 1][j].red = data[j * bpp + 2];
            image->rgb[heigth - i - 1][j].green = data[j * bpp + 1];
            image->rgb[heigth - i - 1][j].blue = data[j * bpp];
            unsigned short r = image->rgb[heigth - i - 1][j].red;
            unsigned short g = image->rgb[heigth - i - 1][j].green;
            unsigned short b = image->rgb[heigth - i - 1][j].blue;
            if (r == 0 && g == 0 && b == 0)
                continue;
            if (r == 255 && g == 255 && b == 255)
                continue;
            printf("%hd %hd %hd\n" ,r, g, b);
        }
    }
    image->heigth = heigth;
    image->width = width;


    /* Free stuff */
    free(data);
    fclose(f);
    return image;
}


int main() {
    image_ini("limits.bmp");
}
