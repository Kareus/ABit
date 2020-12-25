//------------------------------------------------------------------------
// Copyright(c) 2020 Kareus
//------------------------------------------------------------------------

#ifndef __VERSION__
#define __VERSION__

#include "pluginterfaces/base/fplatform.h"

// Here you can define the version of your plug-in: "Major.Sub.Release.Build""
#define MAJOR_VERSION_STR "1"
#define MAJOR_VERSION_INT 1

#define SUB_VERSION_STR "0"
#define SUB_VERSION_INT 0

#define RELEASE_NUMBER_STR "0"
#define RELEASE_NUMBER_INT 0

#define BUILD_NUMBER_STR "1" // Build number to be sure that each result could be identified.
#define BUILD_NUMBER_INT 1

#define BUILD_AS_VST2 0

// Version with build number (example "1.0.3.342")
#define FULL_VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR "." BUILD_NUMBER_STR

// Version without build number (example "1.0.3")
#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR

#define stringOriginalFilename	"ABit.vst3"
#if SMTG_PLATFORM_64
#define stringFileDescription	"ABit (64Bit)"
#else
#define stringFileDescription	"ABit"
#endif
#define stringCompanyName		"Kareus\0"
#define stringLegalCopyright	"Copyright(c) 2020 Kareus."
#define stringLegalTrademarks	"VST is a trademark of Steinberg Media Technologies GmbH"

#endif
