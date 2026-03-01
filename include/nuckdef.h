#ifndef NUCKDEF_H
#define NUCKDEF_H

#ifndef __int8_t_defined
typedef __INT8_TYPE__ int8_t;
#define __int8_t_defined
#endif
#ifndef __uint8_t_defined
typedef __UINT8_TYPE__ uint8_t;
#define __uint8_t_defined
#endif

#ifndef __int16_t_defined
typedef __INT16_TYPE__ int16_t;
#define __int16_t_defined
#endif
#ifndef __uint16_t_defined
typedef __UINT16_TYPE__ uint16_t;
#define __uint16_t_defined
#endif

#ifndef __int32_t_defined
typedef __INT32_TYPE__ int32_t;
#define __int32_t_defined
#endif
#ifndef __uint32_t_defined
typedef __UINT32_TYPE__ uint32_t;
#define __uint32_t_defined
#endif

#ifndef __int64_t_defined
typedef __INT64_TYPE__ int64_t;
#define __int64_t_defined
#endif
#ifndef __uint64_t_defined
typedef __UINT64_TYPE__ uint64_t;
#define __uint64_t_defined
#endif



#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif





#endif