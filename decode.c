/* This file contains codes related to decoding */

#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */
/* Validating the files given through CLA */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
	/* Checking for stego image passed */
    if(strcmp((strstr(argv[2],".")), ".bmp") == 0)
    {
        decInfo -> stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    
	/* Checking if decode.txt given, if not assign by default */
    if(argv[3] != NULL)
    {
        decInfo -> decode_fname = argv[3];
    }
    else
    {
        decInfo -> decode_fname = "decode.txt";
    }

    /* No failure return e_success */
    return e_success;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Stego Image file and decode.txt
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

/* Open stego image in read only mode and decode.txt in write only mode */
Status open_decode_files(DecodeInfo *decInfo)
{
    /* Stego Image file pointer */
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    
	/* Do Error handling */
    if (decInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

    	return e_failure;
    }

    /* decode.txt file pointer */
    decInfo->fptr_decode_text = fopen(decInfo->decode_fname, "w");
    
	/* Do Error handling */
    if (decInfo->fptr_decode_text == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->decode_fname);

    	return e_failure;
    }

    /* No failure return e_success */
    return e_success;
}

/* Function definition to fetch LSB bit from 8 bytes of stego image */
Status decode_byte_from_lsb(char *ch, char *data_buffer)
{
    *ch = 0;
	
	//fetch 1 bit at a time and decode from the LSB of each 8 bytes from the stego image buffer
    for(int i = 0; i < 8; i++)
    {
		//data & 0x01 gives the LSB of the byte
		//assign it to the MSB of a char variable at first iteration
		//then keep assigning towards the LSB of the char variable during each loop
        *ch |= ((data_buffer[i] & 0x01) << (7-i)) ;
    }
    
	/* No failure return e_success */
    return e_success;
}

/* Function definition used to decode data from stego image */
Status decode_data_from_image(const char *data, int size, FILE *fptr_stego_img, DecodeInfo *decInfo)
{
    char ch;
    
	/* Run loop until byte size reached */
    for(int i = 0; i < size; i++)
    {
		/* Read bytes from stego image and pass to decode function */
        fread(decInfo -> decode_data, 8, sizeof (char), decInfo -> fptr_stego_image);
        decode_byte_from_lsb(&ch, decInfo -> decode_data);
        
        /* Failure if data is not matching return e_failure */
        if(ch != data[i])
        {
            return e_failure;
        }
    }

	/* No failure return e_success */
    return e_success;
}

/* Function definition to decode the magic string  */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
	/* Place pointer to 54th position to skip the header file */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);
    
	/* Every decoding needs to call a function decode_data_from_image */
	if(decode_data_from_image(magic_string, strlen(magic_string), decInfo->fptr_stego_image, decInfo) == e_failure)
    {
        return e_failure;
    }
    
	/* No failure return e_success */
    return e_success;
}

/* Function definition to decode size related data */
Status decode_size_from_lsb(char *buffer, long int *ch)
{
    *ch = 0;
    
	/* Size data is 4 bytes hence run for 32 times */
	for(int i = 0; i < 32; i++)
    {
		//data & 0x01 gives the LSB of the byte
		//assign it to the MSB of a char variable at first iteration
		//then keep assigning towards the LSB of the char variable during each loop
        *ch |= ((buffer[i] & 0x01) << (31-i)) ;
    }
	/* No failure return e_success */
    return e_success;
}

/* Function definition related to decoding size related data */
Status decode_size(int size, DecodeInfo *decInfo)
{
    char str[32];		//Buffer for storing 4 bytes
    long int ch;		//Variable to store the decoded data

	/* Read size data from stego image and pass to decoding function */
    fread(str,32,sizeof (char), decInfo->fptr_stego_image);
    decode_size_from_lsb(str, &ch);
    
	/* Failure if size data is not matching return e_failure */
    if(ch != size)
    {
        return e_failure;
    }
	/* No failure return e_success */
    return e_success;
}

/* Function definition related to decode .txt */
Status decode_secret_file_extn(const char *decode_file_ext, DecodeInfo *decInfo)
{
    decode_file_ext = ".txt";

	/* Every decoding needs to call a function decode_data_from_image */
    if(decode_data_from_image(decode_file_ext, strlen(decode_file_ext), decInfo->fptr_stego_image, decInfo) == e_failure)
    {
        return e_failure;
    }

	/* No failure return e_success */
    return e_success;
}

/* Function definition related to decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char str[32];	//Buffer to store the 4 bytes from stego image
    long int ch;	//Variable to store the size data

	/* Read the 4 bytes from stego image and pass it to the decoding function*/
    fread(str, 32, sizeof (char), decInfo->fptr_stego_image);
    decode_size_from_lsb(str,&ch);
    
	/* Store the file size to the struct variable */
    decInfo->decode_file_size = ch;
    

	/* No failure return e_success */
    return e_success;
}

/* Function definition related to decoding the secret data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;	//Variable to store each character
    
	/* Run loop until the size of secret file */
    for(int i = 0; i < decInfo -> decode_file_size; i++)
    {
		/* Read bytes from stego image, then pass it to decoding function and write the decoded characters character by character into decode.txt */
        fread(decInfo -> decode_data, 8, sizeof (char), decInfo -> fptr_stego_image);
        decode_byte_from_lsb(&ch, decInfo -> decode_data);
        fprintf(decInfo->fptr_decode_text,"%c",ch);
    }
	/* No failure return e_success */
    return e_success;
}

/* Function definition for decoding */
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo) == e_success)
    {
        printf("Opened all files successfully\n");
        printf("Starting Decoding...\n");
        if(decode_magic_string(MAGIC_STRING, decInfo) == e_success)
        {
            printf("Magic string decoded successfully from stego image\n");
            if(decode_size(strlen(".txt"), decInfo) == e_success)
            {
                printf("Size of text file extension matched as 4 bytes\n");
                if(decode_secret_file_extn(decInfo->extn_decode_file,decInfo) == e_success)
                {
                    printf("File extension is matching as %s\n",".txt");
                    if(decode_secret_file_size(decInfo) == e_success)
                    {
                        printf("Size of secret data to be decoded is %ld bytes\n",decInfo->decode_file_size);
                        
						if(decode_secret_file_data(decInfo) == e_success)
                        {
                            printf("Text copied successfully to %s\n","decode.txt");
                        }
						else
						{
							printf("Text copying failed!!!\n");
							return e_failure;
						}
                    }
					else
					{
						printf("File size could not be retrieved!!!\n");
						return e_failure;
					}
                }
                else
                {
                    printf("File extension is not matching %s!!!\n",".txt");
                    return e_failure;
                }
            }
            else
            {
                printf("Size of file extension not matching as 4 bytes!!!\n");
                return e_failure;
            }
        }
        else
        {
            printf("Magic string decoding failed!!!\n");
            return e_failure;
        }
    }
	else
	{
		printf("File open failed!!!\n");
		return e_failure;
	}
	
	/* No failure return e_success */
    return e_success;
}
