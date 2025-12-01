#pragma once

#include "../defines.h"

#define DARRAY(type) type*

typedef struct {
  u64 stride;
  u64 length;
  u64 capacity;
} DynamicArrayHeader;

#define DARRAY_HEADER_SIZE sizeof(DynamicArrayHeader)
#define DARRAY_GROW_FACTOR 2

TAPI void* _darray_create(u32 stride, u32 capacity);
TAPI void _darray_destroy(void* darray);

TAPI void* _darray_grow(void* darray, u32 newCapacity);
TAPI void* _darray_append(void* darray, void* value);
TAPI b8 _darray_pop(void* darray, void* dest);
TAPI void* _darray_insert(void* darray, void* value, u32 index);
TAPI b8 _darray_remove(void* darray, u32 index, void* dest);
TAPI u64 darray_length(const void* darray);

#define darray_create(type, capacity) _darray_create(sizeof(type), capacity) 
#define darray_destroy(darray) _darray_destroy(darray)

#define darray_append(darray, value) {\
  typeof(value) temp = value;\
  darray = _darray_append(darray, &temp);\
} while(0)
#define darray_pop(darray, dest) _darray_pop(darray, dest)
#define darray_insert(darray, value, index) {\
  typeof(value) temp = value;\
  darray = _darray_insert(darray, &temp, index);\
} while(0)
#define darray_remove(darray, index, dest) _darray_remove(darray, index, dest);

