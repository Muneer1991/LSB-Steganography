#ifndef TYPES_H
#define TYPES_H

/* User defined type for unsigned int */
typedef unsigned int uint;

/* Status will be used as function return type */
typedef enum
{
    e_success,
    e_failure
} Status;

/* Operation will be used to check encode or decode mode */
typedef enum
{
    e_encode,
    e_decode,
    e_unsupported
} OperationType;

#endif
