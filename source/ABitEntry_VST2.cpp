#include "version.h"

#if BUILD_AS_VST2
#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"
#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.sdk.cpp"

#include "ABitProcessor.h"
#include "ABitController.h"
#include "ABitcids.h"

using namespace Steinberg::Vst;
using namespace Kareus;

::AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return Steinberg::Vst::Vst2Wrapper::create(GetPluginFactory(), kABitProcessorUID, VST_ID, audioMaster);
}

#endif