#ifndef __ABIT_CONTROLLER__
#define __ABIT_CONTROLLER__

/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - https://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <vector>
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace VSTGUI;

namespace Kareus {

//------------------------------------------------------------------------
//  ABitController
//------------------------------------------------------------------------
class ABitController : public Steinberg::Vst::EditControllerEx1, public Steinberg::Vst::IMidiMapping, public VST3EditorDelegate
{
public:

	//------------------------------------------------------------------------

	// Create function
	static Steinberg::FUnknown* createInstance(void* /*context*/)
	{
		return (Steinberg::Vst::IEditController*)new ABitController;
	}

	// IPluginBase
	Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

	// EditController
	Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setParamNormalized(Steinberg::Vst::ParamID tag,
		Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamStringByValue(Steinberg::Vst::ParamID tag,
		Steinberg::Vst::ParamValue valueNormalized,
		Steinberg::Vst::String128 string) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamValueByString(Steinberg::Vst::ParamID tag,
		Steinberg::Vst::TChar* string,
		Steinberg::Vst::ParamValue& valueNormalized) SMTG_OVERRIDE;

	Steinberg::tresult PLUGIN_API getMidiControllerAssignment(Steinberg::int32 busIndex, Steinberg::int16 channel, Steinberg::Vst::CtrlNumber midiControllerNumber, Steinberg::Vst::ParamID& tag) SMTG_OVERRIDE;

	//---Interface---------
	DELEGATE_REFCOUNT(EditController)
	Steinberg::tresult PLUGIN_API queryInterface(const char* iid, void** obj) SMTG_OVERRIDE;

	VSTGUI::CView* verifyView(CView* view, const UIAttributes&, const IUIDescription*, VST3Editor* editor) SMTG_OVERRIDE;

	//------------------------------------------------------------------------

private:
	CTextLabel* waveformName = nullptr;

	void updateWaveformLabel(char* text);
};
//------------------------------------------------------------------------
};
#endif