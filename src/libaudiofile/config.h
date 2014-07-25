#if defined(__MSVC__)
  #include <BaseTsd.h>
  typedef int ssize_t;
  #define snprintf  _snprintf
  #define HAVE_UNISTD_H 0

  #pragma warning(disable: 4244)
  #pragma warning(disable: 4800)
#else
  /* Define to 1 if you have the <unistd.h> header file. */
  #define HAVE_UNISTD_H 1
#endif
