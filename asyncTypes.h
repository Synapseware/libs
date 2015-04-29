#ifndef __ASYNCTYPES_H
#define __ASYNCTYPES_H

#include "inttypes.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef uint8_t (*fValueCallback)(void);
typedef void (*fVoidCallback)(void);
typedef void (*fStatusCallback)(uint8_t);

#ifdef  __cplusplus
}
#endif

#endif