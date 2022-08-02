/* This file contains codes related to encoding */

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

/* Validating the files given through CLA */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    /* Checking for .bmp source image passed */
    if(strcmp((strstr(argv[2],".")), ".bmp") == 0)
    {
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    
    /* Checking if .txt secret file passed */
	if(strcmp((strstr(argv[3],".")), ".txt") == 0)
    {
        encInfo -> secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }

    /* Checking if stego.bmp given, if not assign by default */
    if(argv[4] != NULL)
    {
        encInfo -> stego_image_fname = argv[4];
    }
    else
    {
        encInfo -> stego_image_fname = "stego.bmp";
    }
    return e_success;
}

/* Function definition to get the image size details */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    /* Variables for width and height */
    uint width, height;

    /* Place pointer to 18th byte to get the size details */ 
    fseek(fptr_image, 18, SEEK_SET);

    /* Read the width */
    fread(&width, sizeof(int), 1, fptr_image);
    printf("Source image width = %u\n", width);

    /* Read the height */ 
	fread(&height, sizeof(int), 1, fptr_image);
    printf("Source image height = %u\n", height);

    /* Return image capacity */
	return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

/* Function definition to open files in relevant modes */
Status open_files(EncodeInfo *encInfo)
{
    /* Source image file */ 
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");

	/* Do Error handling */ 
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    /* Secret file */ 
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");

    /* Do Error handling */ 
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }
    
	/* Stego Image file */ 
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    
    /* Do Error handling */ 
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Function definition to get the secret file size */
uint get_file_size(FILE *fptr_secret)
{
    /* Pointing to last position */
    fseek(fptr_secret, 0, SEEK_END);

    /* Return the last position value which in turn is the file size */
    return ftell(fptr_secret);
}

/* Function definition to check if the secret file size is less than the source image size */
Status check_capacity(EncodeInfo *encInfo)
{
	/* File pointers for source image and secret file */
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    
	/* 54 byte header file,  2 byte magic string, 4 byte for .txt extension's size, 4 bytes of .txt, 4 bytes of secret file size, number of bytes that are to be encoded */
	if(encInfo->image_capacity > (54 + ((2 + 4 + 4 + 4 + encInfo->size_secret_file) * 8)))
    {
        return e_success;
    }
    else
    {
        return e_failure; 
    }
}

/* Function definition to copy the source image header to stego image */
Status copy_bmp_header(FILE *fptr_src, FILE *fptr_stego)
{
    fseek(fptr_src, 0, SEEK_SET);                //Point to starting of source image
    char str[54];                                //Array to store the header
    fread(str, sizeof(char), 54, fptr_src);      //Read the 54 bytes from source image
    fwrite(str, sizeof(char), 54,fptr_stego);    //Store the 54 bytes in stego.bmp
    return e_success;
}

/* Function definition to encode data into bytes of the stego image */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
   
    /* fetch 1 bit at a time and encode in the LSB of each 8 bytes in the image buffer */
    unsigned int mask = 1 << 7;

	/* Encode a byte to 8 bytes each time */
    for(int i = 0; i < 8; i++)
    {
        //data & mask will give the MSB bit in the 1st iteration
        //Bring the MSB of data to LSB by (data & mask) >> 7
        //fetch byte by byte from image_buffer[i]
        //put the MSB bit by bit to the LSB of each byte of image buffer
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        
		mask = mask >> 1;	//right shift mask by 1 each time to get next bit
    }

    // No failure return e_success
    return e_success;
}

/* Function definition to encode size related data */
Status encode_size_to_lsb(char *buffer, int size)
{
    unsigned int mask = 1 << 31;
    for(int i = 0; i < 32; i++)
    {
        //data & mask will give the MSB bit in the 1st iteration
        //Bring the MSB of data to LSB by (data & mask) >> 31
        //fetch byte by byte from image_buffer[i]
        //put the MSB bit by bit to the LSB of each byte of image buffer
        buffer[i] = (buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        
		mask = mask >> 1;	//right shift mask by 1 each time to get next bit
    }
    
	// No failure return e_success
    return e_success;
}

/* Function definition related to encode size related data */
Status encode_size(int size, FILE *src, FILE *stego)
{
    char str[32];						//data buffer
    fread(str,32,sizeof (char), src);	//store data to buffer
    encode_size_to_lsb(str,size);		//function call to encode size into the bytes in the buffer
    fwrite(str,32,sizeof (char),stego);	//write the modified buffer into stego image
    
	// No failure return e_success
    return e_success;
}

/* Function definition to encode data into the stego image */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_img, FILE *fptr_stego_img, EncodeInfo *encInfo)
{
    
    //Each time pass 1 byte data along with 8 byte of beautiful.bmp
    for(int i = 0; i < size; i++)
    {
        fread(encInfo->image_data, sizeof(char), 8, fptr_src_img);		//Read source image bytes into a buffer
        encode_byte_to_lsb(data[i], encInfo->image_data);				//Modify the buffer bytes by encoding the data
        fwrite(encInfo->image_data, sizeof(char), 8, fptr_stego_img);	//Write the buffer bytes to stego image
    }
	
	// No failure return e_success
    return e_success;
}

/* Function definition for encoding magic string */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    //Every encoding needs to call a function encode_data_to_image
    encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
	
	// No failure return e_success
    return e_success;
}

/* Function definition to encode secret file extension */
Status encode_secret_file_extn(const char *file_ext, EncodeInfo *encInfo)
{
    file_ext = ".txt";
    
	//Every encoding needs to call a function encode_data_to_image
    encode_data_to_image(file_ext, strlen(file_ext), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
	
	// No failure return e_success
    return e_success;
}

/* Function definition to encode secret file size */
Status encode_secret_file_size(long int size, EncodeInfo *encInfo)
{
    char str[32];											//Buffer to hold the bytes
    fread(str,32,sizeof (char), encInfo->fptr_src_image);	//Read the bytes from source image to buffer
    encode_size_to_lsb(str,size);							//Encode the size to bytes of the buffer
    fwrite(str,32,sizeof (char), encInfo->fptr_stego_image);//Write the buffer to stego image

	// No failure return e_success
    return e_success;
}

/* Function definition to encode the secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

	/* Point to the starting position of secret file */
    fseek(encInfo->fptr_secret, 0, SEEK_SET);

	/* Call encode function upto the file size reached */
    for(int i = 0; i < encInfo -> size_secret_file; i++)
    {
        fread(encInfo -> image_data, 8, sizeof (char), encInfo -> fptr_src_image);		//Read bytes from source image to buffer
        fread(&ch, 1, sizeof (char), encInfo -> fptr_secret);							//Read characters from secret file and store in ch
        encode_byte_to_lsb(ch, encInfo -> image_data);									//Encode the characters into the bytes of the buffer
        fwrite(encInfo -> image_data, 8, sizeof (char), encInfo -> fptr_stego_image);	//Write the buffer to stego image
    }
	
	// No failure return e_success
    return e_success;
}

/* Function definition to copy remaining bytes of source image to stego image */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_stego)
{
    char ch;

	/* Read and write from source image to stego image until end of file reached */
    while(fread(&ch, 1, 1, fptr_src) > 0)
    {
        fwrite(&ch, 1, 1, fptr_stego);        
    }
	
	// No failure return e_success
    return e_success;
}

/* Function definition for encoding */
Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        printf("Opened all files successfully\n");
        printf("Starting Encoding...\n");
        if(check_capacity(encInfo) == e_success)
        {
            printf("Secret data can be encoded in .bmp\n");

            if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("Header file of source image copied to stego image successfully\n");
                
				if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Magic string encoded successfully to stego image\n");
                    
					if(encode_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Encoded secret file extension size successfully to stego image\n");
                        
						if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_success)
                        {
                            printf("Encoded extension successfully to stego image\n");
                            
							if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Encoded secret file size succesfully to stego image\n");
                                
								if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Encoded secret data successfully to stego image\n");
                                    
									if(copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
                                    {
                                        printf("Copied remaining data of source image to stego image\n");
                                    }
                                    else
                                    {
                                        printf("Failed copying remaining data!!!\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Secret data not encoded successfully!!!\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Secret file size not encoded sucessfully!!!\n");
                                return e_failure;
                            }
                                
                        }
                        else
                        {
                            printf("File extension not encoded successfully!!!\n");
                            return e_failure;
                        }
                    }
                    else
                        printf("Secret file extension size not encoded successfully!!!\n");
                }
                else
                {
                    printf("Magic string encoding failed!!!\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Header file could not be copied!!!\n");
                return e_failure;
            }
        }
        else
        {
            printf("Encoding is not possible!!!\n");
            return e_failure;
        }
    }
    else
    {
        printf("File open failed!!!\n");
        return e_failure;
    }
    return e_success;
}
