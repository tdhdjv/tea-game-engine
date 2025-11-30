#include "tmemory.h"

#include "logger.h"
#include "../platform/platform.h"

// TODO: Custom string lib
#include <string.h>
#include <stdio.h>

struct MemoryStats {
  u64 totalAllocated;
  u64 taggedAllocation[MEMORY_TAG_MAX_TAGS];
};

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
  "UNKNOWN          ",
  "ARRAY            ",
  "DARRAY           ",
  "DICT             ",
  "RING_QUEUE       ",
  "BST              ",
  "STRING           ",
  "JOB              ",
  "TEXTURE          ",
  "MATERIAL_INSTANCE",
  "RENDERER         ",
  "GAME             ",
  "TRANSFORM        ",
  "ENTITY           ",
  "ENTITY_NODE      ",
  "SCENE            "
};
static struct MemoryStats stats;

void memory_init() {
  platform_zero_memory(&stats, sizeof(stats));
}

void memory_shutdown() {
}

void *tallocate(u64 size, MemoryTag tag) {
  if(tag == MEMORY_TAG_UNKNOWN) {
    TWARN("tallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
  }
  stats.totalAllocated += size;
  stats.taggedAllocation[tag] += size;

  //TODO: Memory alignment
  void* block = platform_allocate(size, false);
  return block;
}

void *treallocate(void* block, u64 oldSize, u64 newSize, MemoryTag tag) {
  if(tag == MEMORY_TAG_UNKNOWN) {
    TWARN("tallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
  }
  stats.totalAllocated -= oldSize;
  stats.taggedAllocation[tag] -= oldSize;
  stats.totalAllocated += newSize;
  stats.taggedAllocation[tag] += newSize;

  //TODO: Memory alignment
  void* newBlock = platform_reallocate(block, newSize, false);
  return newBlock;
}

void tfree(void *block, u64 size, MemoryTag tag) {
  if(tag == MEMORY_TAG_UNKNOWN) {
    TWARN("tfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
  }
  stats.totalAllocated -= size;
  stats.taggedAllocation[tag] -= size;

  //TODO: Memory alignment
  platform_free(block, false);
}

void *tzero_memory(void *block, u64 size) {
  return platform_zero_memory(block, size);
}

void *tcopy_memory(void *dest, const void *source, u64 size) {
  return platform_copy_memory(dest, source, size);
}

void *tmove_memory(void *dest, const void *source, u64 size) {
  return platform_copy_memory(dest, source, size);
}

void *tset_memory(void *dest, i32 value, u64 size) {
  return platform_set_memory(dest, value, size);
}

#define GiB 1024 * 1024 * 1024
#define MiB 1024 * 1024
#define KiB 1024

char* get_memory_usage_str() {

  char buffer[8000] = "System memory use (tagged): \n";
  u64 offset = strlen(buffer);
  for(u32 i = 0; i < MEMORY_TAG_MAX_TAGS; i++) {
    char unit[4] = "XiB";
    float amount = 1.0f;
    if(stats.taggedAllocation[i] >= GiB) {
      unit[0] = 'G';
      amount = stats.taggedAllocation[i] / (float)GiB;
    } else if(stats.taggedAllocation[i] >= MiB) {
      unit[0] = 'M';
      amount = stats.taggedAllocation[i] / (float)MiB;
    } else if(stats.taggedAllocation[i] >= KiB) {
      unit[0] = 'K';
      amount = stats.taggedAllocation[i] / (float)KiB;
    } else {
      unit[0] = 'B';
      unit[1] = 0;
      amount = (float)stats.taggedAllocation[i];
    }

    i32 length = snprintf(buffer + offset, 9000, "%s: %.2f%s\n", memoryTagStrings[i], amount, unit);
    offset += length;
  }
  char *outString = strdup(buffer);
  return outString;
}
