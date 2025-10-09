#ifndef SS_SANITIZERS
#define SS_SANITIZERS

#include <stddef.h>

#ifdef __has_feature
  #if __has_feature(address_sanitizer)
    #include <sanitizer/lsan_interface.h>
  #else
    // Define no-op versions if LSan isn’t enabled
    static inline void __lsan_disable(void) {}
    static inline void __lsan_enable(void) {}
    static inline void __lsan_register_root_region(const void *start, size_t size) {(void)start;(void)size;}
    static inline void __lsan_unregister_root_region(const void *start, size_t size) {(void)start;(void)size;}
  #endif
#else
  static inline void __lsan_disable(void) {}
  static inline void __lsan_enable(void) {}
  static inline void __lsan_register_root_region(const void *start, size_t size) {(void)start;(void)size;}
  static inline void __lsan_unregister_root_region(const void *start, size_t size) {(void)start;(void)size;}
#endif


#endif
