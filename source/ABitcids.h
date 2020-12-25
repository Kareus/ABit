//------------------------------------------------------------------------
// Copyright(c) Kareus
//------------------------------------------------------------------------

#ifndef __ABIT_CIDS__
#define __ABIT_CIDS__

#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/base/funknown.h"

namespace Kareus {
//------------------------------------------------------------------------
static const Steinberg::FUID kABitProcessorUID (0x3C89485A, 0x107E5793, 0xA095AFE7, 0x8D9B0456);
static const Steinberg::FUID kABitControllerUID (0x37155ED9, 0x165C5E4C, 0x9D360A10, 0xDDE74FA9);
static const int VST_ID = 0; //VST_ID should be unique anyway

#define ABitVST3Category "Instrument"

//------------------------------------------------------------------------
} // namespace Kareus

#endif