#include "darray.h"
#include "../core/logger.h"
#include "../core/tmemory.h"

void* _darray_create(u32 stride, u32 capacity) {
  DynamicArrayHeader* header = tallocate(DARRAY_HEADER_SIZE + stride*capacity, MEMORY_TAG_DARRAY);
  header->stride = stride;
  header->length = 0;
  header->capacity = capacity;
  void* array = (void*)((char*)header + DARRAY_HEADER_SIZE);
  return array;
}

void _darray_destroy(void* darray) {
  DynamicArrayHeader* header = darray - DARRAY_HEADER_SIZE;
  tfree(header, header->stride*header->capacity + DARRAY_HEADER_SIZE, MEMORY_TAG_DARRAY);
}

void* _darray_grow(void* darray, u32 newCapacity) {
  DynamicArrayHeader* previousHeader = darray - DARRAY_HEADER_SIZE;
  DynamicArrayHeader* newHeader = treallocate(previousHeader, DARRAY_HEADER_SIZE + previousHeader->stride*previousHeader->capacity,  DARRAY_HEADER_SIZE + previousHeader->stride*newCapacity, MEMORY_TAG_DARRAY);
  if(!newHeader) {
    TERROR("Failed to grow memory");
    return false;
  }
  newHeader->capacity = newCapacity;
  void* array = (void*)((char*)newHeader + DARRAY_HEADER_SIZE);
  darray = array;
  return darray;
}

void* _darray_append(void* darray, void* value) {
  u32 length;
  u32 stride;
  DynamicArrayHeader* header = darray - DARRAY_HEADER_SIZE;
  header->length++;
  length = header->length;
  stride = header->stride;
  if(header->length >= header->capacity) {
    darray = _darray_grow(darray, header->capacity*DARRAY_GROW_FACTOR);
  }
  tcopy_memory((darray) + stride*(length-1), value, stride);
  
  return darray;
}

b8 _darray_pop(void* darray, void* dest) {
  DynamicArrayHeader* header = darray - DARRAY_HEADER_SIZE;
  header->length--;
  if(dest) tcopy_memory(dest, darray + header->length*header->stride, header->stride);
  return true;
}

void* _darray_insert(void* darray, void* value, u32 index) {
  u32 length;
  u32 stride;
  DynamicArrayHeader* header = darray - DARRAY_HEADER_SIZE;
  if(index > header->length) {
    return darray;
  }

  header->length++;
  length = header->length;
  stride = header->stride;
  if(header->length >= header->capacity) {
    darray = _darray_grow(darray, header->capacity*DARRAY_GROW_FACTOR);
  }
  tmove_memory((darray) + stride*(index+1), (darray) + stride*index, (length-index)*stride);
  tcopy_memory((darray) + stride*index, value, stride);
  return darray;
}

b8 _darray_remove(void* darray, u32 index, void* dest) {
  DynamicArrayHeader* header = darray - DARRAY_HEADER_SIZE;
  if(index >= header->length) {
    TERROR("Index outside of the bounds of this array!: Length: %i, Index %i", header->length, index);
    return false;
  }
  header->length--;
  if(dest) tcopy_memory(dest, darray + header->stride*index, header->stride);
  tmove_memory(darray + header->stride*index, darray + header->stride*(index+1), header->stride*(header->length-index));
  return true;
}

u64 darray_length(const void* darray) {
  const DynamicArrayHeader* header = darray - DARRAY_HEADER_SIZE;
  return header->length;
}
