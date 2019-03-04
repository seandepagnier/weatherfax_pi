#if defined(__MSVC__)
  #include <BaseTsd.h>
  typedef int ssize_t;
/*  #define snprintf  _snprintf
  #define HAVE_UNISTD_H 0

  #pragma warning(disable: 4244)
  #pragma warning(disable: 4800)

#else
  /* Define to 1 if you have the <unistd.h> header file. */
  #define HAVE_UNISTD_H 1
#endif

#if (defined(__GNUC__) && defined(__STRICT_ANSI__)) || (defined(_MSC_VER) && !defined(__cplusplus))   /* Visual Studio */
#  define inline __inline           /* Visual C is not C99, but supports some kind of inline */
#endif
