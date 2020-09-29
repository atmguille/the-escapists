#include "image.h"

#define HEADER_MIN_SIZE 54

Image *image_ini(const char *filename) {
    FILE *f;
    unsigned char header[HEADER_MIN_SIZE]; /* The header is at least HEADER_MIN_SIZE bytes long */
    unsigned char *data; /* The data depends on the image size, so we'll allocate memory */
    int i, j;
    unsigned short width;
    unsigned short heigth;
    unsigned short bpp; /* Bytes per pixel, usually 3 */
    unsigned short header_size; /* The header is not usually just HEADER_MIN_SIZE bytes long, but longer */
    unsigned short row_size; /* It MUST be a multiple of 4, because of allignment */
    Image *image;

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
    header_size = *(int *)&header[10];
    row_size = ((width * bpp - 1)|3) + 1; /* 4 byte alignment */
    /* Go forward to end of header */
    fseek(f, header_size, 0);

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
    data = (unsigned char *) malloc(sizeof(unsigned char) * row_size);

    for (i = 0; i < heigth; i++) {
        fread(data, sizeof(unsigned char), row_size, f);
        for (j = 0; j < width; j++) {
            /* The file has data upside down, thus, we use height - i - 1 so we don't get flipped images */
            /* The width * bpp is not a multiple of four, we'll read up to 3 bytes more we won't use */
            /* If the image had 32 bit per pixel (4 bpp), the data[j * bpp + 3] would give us white */
            image->rgb[heigth - i - 1][j].red = data[j * bpp + 2];
            image->rgb[heigth - i - 1][j].green = data[j * bpp + 1];
            image->rgb[heigth - i - 1][j].blue = data[j * bpp];
        }
    }
    image->heigth = heigth;
    image->width = width;


    /* Free stuff */
    free(data);
    fclose(f);
    return image;
}

void image_print(Image *image, int x, int y) {
    int i, j; 
    if (image == NULL)
        return;

    pthread_mutex_lock(&semaphore);
    for (i = 0; i < image->heigth; i++) {
        move_cursor_to(x, y + i);
        for (j = 0; j < image->width; j++) {
            printf(COLOR_SPACE(image->rgb[i][j].red, image->rgb[i][j].green, image->rgb[i][j].blue));
        }
    }
    pthread_mutex_unlock(&semaphore);
}

void image_print_transparent(Image *image, Image *background, int x, int y) {
    int i, j;
    int red, green, blue;
    
    pthread_mutex_lock(&semaphore);
    for (i = 0; i < image->heigth; i++) {
        /* Jump to the next jump */
        move_cursor_to(x, y + i);
        for (j = 0; j < image->width; j++) {
            red = image->rgb[i][j].red;
            green = image->rgb[i][j].green;
            blue = image->rgb[i][j].blue;
            /* If the pixel's white, we paint the background */
            if (red == 255 && green == 255 && blue == 255) {
                red = background->rgb[y + i][x + j].red;
                green = background->rgb[y + i][x + j].green;
                blue = background->rgb[y + i][x + j].blue;
            }
            printf(COLOR_SPACE(red, green, blue));
        }
    }
    pthread_mutex_unlock(&semaphore);
}

void image_free(Image *image) {
    int row;
    if (image == NULL)
        return;
    for (row = 0; row < image->heigth; row++)
        free(image->rgb[row]);
    free(image->rgb);
    free(image);
}
