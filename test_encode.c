/**************Documentation**************
Name          : Muneer Mohammad Ali
Date          : 09/05/2022
Description   : LSB Steganography project
Sample Input  : Encoding : ./a.out -e beautiful.bmp secret.txt stego.bmp
				Decoding : ./a.out -d stego.bmp decode.txt
Sample Output : Encoding : stego.bmp
				Decoding : decode.txt
******************************************/
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
	/* Unsigned int variable to store the image size*/ 
    uint img_size;

    /* Check the operation type is encoding (-e) */
    if(check_operation_type(argv) == e_encode)
    {
		/* Struct variable to store encoding related info */
        EncodeInfo encInfo;
        
        printf("----------Selected Encoding----------\n");

        /* Read and validate CLA */
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Reading and validating inputs is successful\n");
            if(do_encoding(&encInfo) == e_success)
            {
                printf("Encoding is completed\n");
            }
            else
            {
                printf("Encoding failed!!!\n");
            }
        }
        else
        {
            printf("Reading and validating inputs failed!!!\n");
        }
    }

    /* Check the operation type is Decoding (-d) */
    else if(check_operation_type(argv) == e_decode)
    {
		/* Struct variable to store decoding related info */
        DecodeInfo decInfo;
        
        printf("----------Selected Decoding----------\n");

        /* Read and validate CLA */
        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Reading and validating inputs is successful\n");
            if(do_decoding(&decInfo) == e_success)
            {
                printf("Decoding is completed\n");
            }
            else
            {
                printf("Decoding failed!!!\n");
            }
        }
        else
        {
            printf("Reading and validating inputs failed!!!\n");
        }
    }

	/* Check if input given is correct */
    else
    {
        printf("Invalid Option\n");
        printf("Encoding : ./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("Decoding : ./a.out -d stego.bmp decode.txt\n");
    }
        
    return 0;
}

/* String compare and check if operation is -e or -d */
OperationType check_operation_type(char *argv[])
{
	/* String compare for -e */
    if(strcmp(argv[1],"-e") == 0)
    {
        return e_encode;
    }
	/* String compare for -d */
    else if(strcmp(argv[1],"-d") == 0)
    {
        return e_decode;
    }
	/* String compare not matching -e or -d failure */
    else
    {
        return e_unsupported;
    }
}
