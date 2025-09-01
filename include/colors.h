#ifndef COLORS_H
#define COLORS_H

/**
 * ANSI color and formatting codes
 */
#define ANSI_RESET     "\x1b[0m"
#define ANSI_BOLD      "\x1b[1m"
#define ANSI_ITALIC    "\x1b[3m"
#define ANSI_UNDERLINE "\x1b[4m"
#define ANSI_RED       "\x1b[31m"
#define ANSI_YELLOW    "\x1b[33m"
#define ANSI_BLUE      "\x1b[34m"
#define ANSI_GREEN     "\x1b[32m"
#define ANSI_MAGENTA   "\x1b[35m"
#define ANSI_CYAN      "\x1b[36m"
#define ANSI_WHITE     "\x1b[37m"
#define ANSI_GRAY      "\x1b[90m"

/**
 * Conditional color output macros
 */
#ifdef NO_COLOR
    #define COLOR(code, text) text
#else
    #define COLOR(code, text) code text ANSI_RESET
#endif

/**
 * Color shorthand macros
 */
#define COLOR_RED(text)     COLOR(ANSI_RED, text)
#define COLOR_GREEN(text)   COLOR(ANSI_GREEN, text)
#define COLOR_YELLOW(text)  COLOR(ANSI_YELLOW, text)
#define COLOR_BLUE(text)    COLOR(ANSI_BLUE, text)
#define COLOR_CYAN(text)    COLOR(ANSI_CYAN, text)
#define COLOR_MAGENTA(text) COLOR(ANSI_MAGENTA, text)
#define COLOR_GRAY(text)    COLOR(ANSI_GRAY, text)


#endif // COLORS_H