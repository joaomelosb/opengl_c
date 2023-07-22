#ifndef CONFIG_H
#define CONFIG_H

// for load.c
#define LOAD_BLOCK 1024

#if LOAD_BLOCK < 1
#error "LOAD_BLOCK macro shall be > 0"
#endif

#endif /* CONFIG_H */
