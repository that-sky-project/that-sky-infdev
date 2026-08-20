#ifndef __UTILS_STRINGUTILS_HPP__
#define __UTILS_STRINGUTILS_HPP__

#include <cstring>
#include <Utils/Types.h>

namespace StringUtils {

inline cstring StrDup(
  cstring _Source
) {
  size_t l = strlen(_Source) + 1;
  char *s = new char[l];
  strncpy(s, _Source, l);

  return s;
}

}


#endif
