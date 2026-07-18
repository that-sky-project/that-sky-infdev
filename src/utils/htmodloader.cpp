#include <Windows.h>
#include <includes/htmodloader.h>
#include "utils/htmodloader.hpp"

extern HMODULE hModuleDll;

HTStatus HTModLoader::createHookAndEnable(
  const HTAsmSig *sigXX,
  HTAsmFunction *sfn
) {
  if (sigXX && !HTSigScanFunc(sigXX, sfn))
    return HT_FAIL;

  if (!HTAsmHookCreate(hModuleDll, sfn))
    return HT_FAIL;

  return HTAsmHookEnable(hModuleDll, sfn->fn);
}
