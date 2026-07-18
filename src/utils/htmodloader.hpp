#ifndef __UTILS_HTMODLOADER_HPP__
#define __UTILS_HTMODLOADER_HPP__

#include <includes/htmodloader.h>

namespace HTModLoader {

HTStatus createHookAndEnable(
  const HTAsmSig *sigXX,
  HTAsmFunction *sfn);

}

#endif
