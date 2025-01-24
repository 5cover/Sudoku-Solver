/** @file
 * @brief Memory debugger
 * @author 5cover
 *
 * This memory debugger allow seeing which allocations have been made at any
 * point in the program. It checks if everytying has been freed on exit.
 *
 * Current allocations can be printed in a table as such:
 * status    | ptr          | method | size | comment
 * freed     | 0x0008945613 | malloc | 18   | grid cell 0,1
 * allocated | 0x0008965431 | calloc | 1024 | grid
 *
 * Ideas for more features:
 * - Make memory allocation functions (calloc, malloc) articifially return null
 * to test error handling.
 */

#pragma once

#include <inttypes.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "utils.c"

#if defined __GNUC__ && !defined __clang__
#define attr_malloc __attribute__((__malloc__, __malloc__(dbg_free, 3)))
#else
#define attr_malloc
#endif

/// @brief Integer: an exit code for when a memory allocation fails.
#define EXIT_MALLOC_FAILED (-1)

/// @brief Aborts the program with a debug message.
#define dbg_fail(fmt_msg, ...)                                                 \
  do {                                                                         \
    fputs("!! ", stderr);                                                      \
    fprintf(stderr, (fmt_msg)__VA_OPT__(, ) __VA_ARGS__);                      \
    putc('\n', stderr);                                                        \
    abort();                                                                   \
  } while (0)

#ifdef NDEBUG

// Disable memory allocation checks.
#define check_alloc(mallocResult, ...) (mallocResult)

#else

/// @brief Prints information about the state of the heap.
/// @param outStream in: the stream to print to.
void dbg_print_allocations(FILE *outStream);

/// @brief Performs an emergency full memory cleanup.
/// @remark This function is called just before exiting in @ref
/// exit_mallocFailed, in order to shutdown with the memory freed.
/// @remark This function must be implemented by the client considering that it
/// may be called at any point in the program.
void perform_emergencyMemoryCleanup(void);

/// @brief Checks that an allocation succeeded and adds a comment to it for
/// memory debugging
/// @param mallocResult in: result of the memory allocation
/// @param fmt_allocComment in: comment format string
/// @param ...: comment format arguments
/// @return @p mallocResult. Aborts if @p mallocResult is null.
void *check_alloc(void *mallocResult, char const *fmt_allocComment, ...);

/// @brief Frees memory with debugging.
/// @param file in: current file
/// @param line in: current line
/// @param ptr in: pointer to free
void dbg_free(char const *file, int line, void *ptr);

/// @brief Allocates memory using malloc with debugging.
/// @param file in: current file
/// @param line in: current line
/// @param size in: number of bytes to allocate
/// @return A pointer to the newly allocated memory region.
void *dbg_malloc(char const *file, int line, size_t size) attr_malloc;

/// @brief Allocates memory using calloc with debugging.
/// @param file in: current file
/// @param line in: current line
/// @param nmemb in: number of members
/// @param size in: member size.
/// @return A pointer to the newly allocated memory region.
void *dbg_calloc(char const *file, int line, size_t nmemb,
                 size_t size) attr_malloc;

#endif // NDEBUG

/////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG

#define STB_DS_IMPLEMENTATION
#include "stb/ds.h"

#ifdef MEMBDG_VERBOSE
#define verbose
#else
#define verbose __attribute__((unused))
#endif // MEMDBG_VERBOSE

typedef enum {
  AM_malloc,
  AM_calloc,
} AllocationMethod;

typedef enum {
  AS_freed,
  AS_allocated,
} AllocationStatus;

typedef struct {
  size_t size;
  char *comment;
  AllocationMethod method;
  AllocationStatus status;
} Allocation;

typedef struct {
  void *key;
  Allocation value;
} AllocationsMapItem;

static AllocationsMapItem *gs_allocations_map;

static bool gs_initialized = false;

static void memdbg_exit(void) {
  // Check that everything has been freed
  bool foundUnfreedAlloc = false;
  for (size_t i = 0; i < hmlenu(gs_allocations_map); ++i) {
    Allocation alloc = gs_allocations_map[i].value;
    if (alloc.status != AS_freed) {
      foundUnfreedAlloc = true;
      fprintf(stderr, "! Allocation %zu unfreed at exit (%s)\n", i,
              alloc.comment);
    }
  }

  if (foundUnfreedAlloc) {
    dbg_fail("Unfreed allocations have been found.");
  }

  // Cleanup
  for (size_t i = 0; i < hmlenu(gs_allocations_map); ++i) {
    free(gs_allocations_map[i].value.comment);
  }
  hmfree(gs_allocations_map);
}

static void signalHandler(int sigid) {
  if (sigid == SIGABRT) {
#ifdef MEMDBG_VERBOSE
    // Print allocations on abort
    dbg_print_allocations(stderr);
#endif // MEMDBG_VERBOSE
  }
}

void lazyInit(void) {
  if (!gs_initialized) {
    signal(SIGABRT, signalHandler);
    atexit(memdbg_exit);
  }
  gs_initialized = true;
}

void *dbg_malloc(verbose char const *file, verbose int line, size_t size) {
  lazyInit();
  void *ptr = malloc(size);
#ifdef MEMDBG_VERBOSE
  fprintf(stderr, "%s:%d: memdbg: malloc(%zu) -> %p\n", file, line, size, ptr);
#endif // MEMDBG_VERBOSE
  hmput(gs_allocations_map, ptr,
        ((Allocation){
            .method = AM_malloc,
            .size = size,
            .status = AS_allocated,
            .comment = NULL,
        }));
  return ptr;
}

void *dbg_calloc(verbose char const *file, verbose int line, size_t nmemb,
                 size_t size) {
  lazyInit();
  void *ptr = calloc(nmemb, size);
#ifdef MEMDBG_VERBOSE
  fprintf(stderr, "%s:%d: memdbg: calloc(%zu, %zu) -> %p\n", file, line, nmemb,
          size, ptr);
#endif // MEMDBG_VERBOSE
  hmput(gs_allocations_map, ptr,
        ((Allocation){
            .method = AM_malloc,
            .size = nmemb * size,
            .status = AS_allocated,
            .comment = NULL,
        }));
  return ptr;
}

void dbg_free(char const *file, int line, void *ptr) {
  lazyInit();
  AllocationsMapItem *item = hmgetp_null(gs_allocations_map, ptr);
  if (item != NULL) {
#ifdef MEMDBG_VERBOSE
    fprintf(stderr, "%s:%d: memdbg: free: %s\n", file, line,
            item->value.comment);
#endif // MEMDBG_VERBOSE
    free(ptr);
    item->value.status = AS_freed;
  } else {
    fprintf(stderr, "%s:%d: memdbg: free(%p)\n", file, line, ptr);
    dbg_fail("Tried to free an invalid pointer: %p", ptr);
  }
}

void *check_alloc(void *mallocResult, char const *fmt_allocComment, ...) {
  lazyInit();
  va_list args;
  va_start(args, fmt_allocComment);

  if (mallocResult != NULL) {
    if (gs_initialized) {
      AllocationsMapItem *item = hmgetp_null(gs_allocations_map, mallocResult);
      if (item == NULL) {
        fprintf(stderr, "!! Tried to check an allocation that never occured: ");
        vfprintf(stderr, fmt_allocComment, args);
        fprintf(stderr, " (%p)\n", mallocResult);
        abort();
      } else {
        item->value.comment = malloc(bufferSize(fmt_allocComment, args));
        // reset the arguments after bufferSize consumed them
        va_end(args);
        va_start(args, fmt_allocComment);
        vsprintf(item->value.comment, fmt_allocComment, args);
      }
    }

    return mallocResult;
  }

  fprintf(stderr, "!! Out of memory allocating ");
  vfprintf(stderr, fmt_allocComment, args);
  putc('\n', stderr);

  va_end(args);

  perform_emergencyMemoryCleanup();

  exit(EXIT_MALLOC_FAILED);
}

void dbg_print_allocations(FILE *outStream) {
  lazyInit();
#define STR_AS_ALLOCATED "allocated"
#define STR_AS_FREED "freed"

#define STR_AM_MALLOC "malloc"
#define STR_AM_CALLOC "calloc"

#define COL_HEAD_INDEX "#"
#define COL_HEAD_STATUS "status"
#define COL_HEAD_PTR "ptr"
#define COL_HEAD_METHOD "method"
#define COL_HEAD_SIZE "size (bytes)"
#define COL_HEAD_COMMENT "comment"

#define COL_LEN_INDEX (digitCount(hmlenu(gs_allocations_map) - 1, 10))
#define COL_LEN_STATUS                                                         \
  ((int)max(sizeof COL_HEAD_STATUS,                                            \
            max(sizeof STR_AS_FREED, sizeof STR_AS_ALLOCATED)) -               \
   1)
#define COL_LEN_PTR ((int)sizeof(void *) + 4)
#define COL_LEN_METHOD                                                         \
  ((int)max(sizeof COL_HEAD_METHOD,                                            \
            max(sizeof STR_AM_MALLOC, sizeof STR_AM_CALLOC)) -                 \
   1)
#define COL_LEN_SIZE ((int)sizeof COL_HEAD_SIZE - 1)

  // Header
  fprintf(outStream, "%*s | %*s | %*s | %*s | %*s | %s\n", COL_LEN_INDEX,
          COL_HEAD_INDEX, COL_LEN_STATUS, COL_HEAD_STATUS, COL_LEN_METHOD,
          COL_HEAD_METHOD, COL_LEN_SIZE, COL_HEAD_SIZE, COL_LEN_PTR,
          COL_HEAD_PTR, COL_HEAD_COMMENT);

  size_t freedCount = 0, allocatedCount = 0;
  for (size_t i = 0; i < hmlenu(gs_allocations_map); ++i) {
    Allocation alloc = gs_allocations_map[i].value;

    allocatedCount += alloc.status == AS_allocated;
    freedCount += alloc.status == AS_freed;

    void *ptr = gs_allocations_map[i].key;

    fprintf(outStream, "%*zu | %*s | %*s | %*zu | %.*" PRIxPTR " | %s\n",
            COL_LEN_INDEX, i, COL_LEN_STATUS,
            alloc.status == AS_freed ? STR_AS_FREED : STR_AS_ALLOCATED,
            COL_LEN_METHOD,
            alloc.method == AM_malloc ? STR_AM_MALLOC : STR_AM_CALLOC,
            COL_LEN_SIZE, alloc.size, COL_LEN_PTR, (intptr_t)ptr,
            alloc.comment);
  }
  fprintf(outStream, "%zu allocations (%zu currently allocated, %zu freed)\n",
          hmlen(gs_allocations_map), allocatedCount, freedCount);
}

// Redefine stdlib functions

#define malloc(size) dbg_malloc(__FILE__, __LINE__, size)
#define calloc(nmemb, size) dbg_calloc(__FILE__, __LINE__, nmemb, size)
#define free(size) dbg_free(__FILE__, __LINE__, size)

#endif // NDEBUG
