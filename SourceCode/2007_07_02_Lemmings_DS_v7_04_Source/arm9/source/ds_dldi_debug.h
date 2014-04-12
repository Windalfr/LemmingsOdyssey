// **************************************************************************
// *                                                                        *
// *  Matt's DS dldi debug splatterer                                       *
// *      by Mathew Carr                                                    *
// *                                                                        *
// **************************************************************************
                   
#ifndef _DS_DLDI_DEBUG_H_
#define _DS_DLDI_DEBUG_H_

// Set this symbol in your code to enable debugging
#ifdef DS_DLDI_DEBUG

// 16k of debug string memory.
#   define DEBUG_MEMORY_STRING_AREA_SIZE (1024 * 16)
    char *DEBUG_MEMORY_STRING_AREA;

#   define DEBUG_SECTION if (1)

#   define DebugAppend(x)                                                   \
     {                                                                      \
        strcat(DEBUG_MEMORY_STRING_AREA, (x));                              \
     }

#   define DebugWrite()                                                     \
     {                                                                      \
        FILE *d = fopen("/debug_out.txt", "ab");                            \
        fprintf(d, "%s", DEBUG_MEMORY_STRING_AREA);                         \
        memset(DEBUG_MEMORY_STRING_AREA, 0, DEBUG_MEMORY_STRING_AREA_SIZE); \
        fclose(d);                                                          \
     }

#   define NewDebug()                                                             \
     {                                                                            \
        FILE *d = fopen("/debug_out.txt", "wb");                                  \
        fprintf(d, "Beginning new debug session:\r\n");                           \
        DEBUG_MEMORY_STRING_AREA = (char *)malloc(DEBUG_MEMORY_STRING_AREA_SIZE); \
        memset(DEBUG_MEMORY_STRING_AREA, 0, DEBUG_MEMORY_STRING_AREA_SIZE);       \
        fclose(d);                                                                \
     }
     
#else
#   define DEBUG_SECTION if (0)
#   define DebugAppend(x)
#   define DebugWrite()
#   define NewDebug()
#endif

#endif // _DS_DLDI_DEBUG_H_


