#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

#define BAD_NUMBER_ARGS 1
#define FSEEK_ERROR 2
#define FREAD_ERROR 3
#define MALLOC_ERROR 4
#define FWRITE_ERROR 5

/**
 * Parses the command line.
 *
 * argc:      the number of items on the command line (and length of the
 *            argv array) including the executable
 * argv:      the array of arguments as strings (char* array)
 * grayscale: the integer value is set to TRUE if grayscale output indicated
 *            outherwise FALSE for threshold output
 *
 * returns the input file pointer (FILE*)
 **/
FILE *parseCommandLine(int argc, char **argv, int *isGrayscale) {
  if (argc > 2) {
    printf("Usage: %s [-g]\n", argv[0]);
    exit(BAD_NUMBER_ARGS);
  }
  
  if (argc == 2 && strcmp(argv[1], "-g") == 0) {
    *isGrayscale = TRUE;
  } else {
    *isGrayscale = FALSE;
  }

  return stdin;
}

unsigned getFileSizeInBytes(FILE* stream) {
  unsigned fileSizeInBytes = 0;
  
  rewind(stream);
  if (fseek(stream, 0L, SEEK_END) != 0) {
    exit(FSEEK_ERROR);
  }
  fileSizeInBytes = ftell(stream);

  return fileSizeInBytes;
}

void getBmpFileAsBytes(unsigned char* ptr, unsigned fileSizeInBytes, FILE* stream) {
  rewind(stream);
  if (fread(ptr, fileSizeInBytes, 1, stream) != 1) {
//#ifdef DEBUG
//    printf("feof() = %x\n", feof(stream));
//    printf("ferror() = %x\n", ferror(stream));
//#endif
    exit(FREAD_ERROR);
  }
}

unsigned char getAverageIntensity(unsigned char blue, unsigned char green, unsigned char red) {
    int average = 0;
    average = (blue + green + red)/3;
    return average;
}

void applyGrayscaleToPixel(unsigned char* pixel) {
//    Each pixel in the image is converted to either white
//    or black based on the average intensity of its three
//    color values. Pixels in the original image with an average
//    intensity of 128 or more will become white (all colors 0xff);
//    those with average intensities below 128 will become
//    black (all colors 0x00).
    unsigned char *blue = pixel;
    unsigned char *green = (pixel + 1);
    unsigned char *red = pixel + 2;
    int average = getAverageIntensity(*blue, *green, *red);
    *red = average;
    *blue = average;
    *green = average;
}

void applyThresholdToPixel(unsigned char* pixel) {
//    write the new value of the pixel using step 5
//    Each pixel in the image is converted to either white or black
//    based on the average intensity of its three color values. Pixels
//    in the original image with an average intensity of 128 or more
//    will become white (all colors 0xff); those with average intensities
//    below 128 will become black (all colors 0x00).
    unsigned char *blue =pixel;
    unsigned char *green = (pixel + 1);
    unsigned char *red = (pixel + 2);
    if(getAverageIntensity(*blue, *green, *red) >= 128){
            *blue = 0xff;
            *green = 0xff;
            *red = 0xff;
    }
    else if(getAverageIntensity(*blue, *green, *red) < 128){
            *blue = 0x00;
            *red = 0x00;
            *green = 0x00;
    }
}

void applyFilterToPixel(unsigned char* pixel, int isGrayscale) {
//    call the appropriate step 4 filter on each pixel
    if(isGrayscale) {
        applyGrayscaleToPixel(pixel);
    }
    else {
        applyThresholdToPixel(pixel);
    }
}

void applyFilterToRow(unsigned char* row, int width, int isGrayscale) {
//    iterate over each pixel in the row and call step 3 on each one --> applyFilterToPixel(unsigned char* pixel, int isGrayscale)
    int i = 0;
    while(i < 3 * width){
        applyFilterToPixel(row + i, isGrayscale);
        i = 3 + i;
    }
}

void applyFilterToPixelArray(unsigned char* pixelArray, int width, int height, int isGrayscale) {
//      iterating through the rows and applying filter to eaach
  int padding = 0;
  int rowWidth = 0;
  int i = 0;
  padding = width % 4;
  rowWidth = (width * 3) + padding;
#ifdef DEBUG
  printf("padding = %d\n", padding);
#endif
    while (i < rowWidth * height) {
        applyFilterToRow(pixelArray + i, width, isGrayscale);
        i = rowWidth + i;
    }
}

void parseHeaderAndApplyFilter(unsigned char* bmpFileAsBytes, int isGrayscale) {
  int offsetFirstBytePixelArray = 0;
  int width = 0;
  int height = 0;
  offsetFirstBytePixelArray = *((int *)(bmpFileAsBytes + 10));
  width = *((int *)(bmpFileAsBytes + 18));
  height = *((int *)(bmpFileAsBytes + 22));
  unsigned char* pixelArray =  offsetFirstBytePixelArray + bmpFileAsBytes;


#ifdef DEBUG
  printf("offsetFirstBytePixelArray = %u\n", offsetFirstBytePixelArray);
  printf("width = %u\n", width);
  printf("height = %u\n", height);
  printf("pixelArray = %p\n", pixelArray);
#endif

  applyFilterToPixelArray(pixelArray, width, height, isGrayscale);
}

int main(int argc, char **argv) {
  int grayscale = FALSE;
  unsigned fileSizeInBytes = 0;
  unsigned char* bmpFileAsBytes = NULL;
  FILE *stream = NULL;
  
  stream = parseCommandLine(argc, argv, &grayscale);
  fileSizeInBytes = getFileSizeInBytes(stream);

#ifdef DEBUG
  printf("fileSizeInBytes = %u\n", fileSizeInBytes);
#endif

  bmpFileAsBytes = (unsigned char *)malloc(fileSizeInBytes);
  if (bmpFileAsBytes == NULL) {
    exit(MALLOC_ERROR);
  }
  getBmpFileAsBytes(bmpFileAsBytes, fileSizeInBytes, stream);

  parseHeaderAndApplyFilter(bmpFileAsBytes, grayscale);

#ifndef DEBUG
  if (fwrite(bmpFileAsBytes, fileSizeInBytes, 1, stdout) != 1) {
    exit(FWRITE_ERROR);
  }
#endif

  free(bmpFileAsBytes);
  return 0;
}
