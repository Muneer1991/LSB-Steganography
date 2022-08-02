/* This file contains the function prototypes and struct required for decoding the data from stego image */

#include <stdio.h>
#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

/* struct for storing relevant info */
typedef struct _DecodeInfo
{
    /* Decode File Info */
    char *decode_fname;
    long int decode_file_size;
    FILE *fptr_decode_text;
    char extn_decode_file[MAX_FILE_SUFFIX];
    char decode_data[MAX_SECRET_BUF_SIZE];
    long size_decode_text;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode and check Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode a byte from LSB of stego image data array */
Status decode_byte_from_lsb(char *ch, char *data_buffer);

/* Decode secret file extension size */
Status decode_size(int size, DecodeInfo *decInfo);

/* Decode size from LSB of stego image data array*/
Status decode_size_from_lsb(char *buffer, long int *ch);

/* Decode secret file size */
Status decode_secret_file_extn(const char *decode_file_extn, DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_data_from_image(const char *data, int size, FILE *fptr_stego_image, DecodeInfo *decInfo);

/* Copy decoded data to a new file decode.txt */
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
