//------------------------------------------------------------------------
// Copyright(c) 2020 Kareus.
//------------------------------------------------------------------------

#include "ABitController.h"
#include "ABitParamIds.h"
#include "Synthesizer.h"

#include "base/source/fstreamer.h"
#include "base/source/fstring.h"

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"

#include "vstgui/uidescription/delegationcontroller.h"


using namespace Steinberg;

namespace Kareus {

//------------------------------------------------------------------------
// ABitController Implementation
//------------------------------------------------------------------------

tresult PLUGIN_API ABitController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// Here you could register some parameters

	Vst::UnitInfo info;
	Vst::Unit* unit;

	info.id = 1;
	info.parentUnitId = Vst::kRootUnitId;

	UString(info.name, USTRINGSIZE(info.name)).assign(USTRING("ADSR"));

	info.programListId = Vst::kNoProgramListId;

	unit = new Vst::Unit(info);
	addUnit(unit);

	int32 unitID = 1;

	Vst::RangeParameter* p_attack = new Vst::RangeParameter(USTRING("Attack"), kAttack, USTRING("secs"), 0.f, 1.f, 0.f, 0, Vst::ParameterInfo::kCanAutomate, unitID);
	parameters.addParameter(p_attack);

	Vst::RangeParameter* p_decay = new Vst::RangeParameter(USTRING("Decay"), kDecay, USTRING("secs"), 0.f, 1.f, 0.f, 0, Vst::ParameterInfo::kCanAutomate, unitID);
	parameters.addParameter(p_decay);

	Vst::RangeParameter* p_sustain = new Vst::RangeParameter(USTRING("Sustain"), kSustain, USTRING("0 ~ 1"), 0.f, 1.f, 0.f, 0, Vst::ParameterInfo::kCanAutomate, unitID);
	parameters.addParameter(p_sustain);

	Vst::RangeParameter* p_release = new Vst::RangeParameter(USTRING("Release"), kRelease, USTRING("secs"), 0.f, 1.f, 0.f, 0, Vst::ParameterInfo::kCanAutomate, unitID);
	parameters.addParameter(p_release);

	Vst::RangeParameter* p_waveform = new Vst::RangeParameter(USTRING("Waveform"), kWaveform, nullptr, 0.f, 8.f, 0.f, 0, Vst::ParameterInfo::kCanAutomate, unitID);
	parameters.addParameter(p_waveform);
	//we're gonna convert float value (0~1) to waveform id in each segment

	Vst::RangeParameter* p_arpduration = new Vst::RangeParameter(USTRING("Arp Time"), kArpDuration, USTRING("x"), 0.f, 1.f, 0.5f, 0, Vst::ParameterInfo::kCanAutomate, unitID);
	parameters.addParameter(p_arpduration);

	Vst::RangeParameter* p_arp = new Vst::RangeParameter(USTRING("Arp"), kArpOnOff, nullptr, 0.f, 1.f, 0.f, 1, Vst::ParameterInfo::kCanAutomate, unitID);
	parameters.addParameter(p_arp);

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitController::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	IBStreamer streamer(state, kLittleEndian);

	float s_attack = 1.f;
	if (!streamer.readFloat(s_attack)) return kResultFalse;

	float s_decay = 1.f;
	if (!streamer.readFloat(s_decay)) return kResultFalse;

	float s_sustain = 1.f;
	if (!streamer.readFloat(s_sustain)) return kResultFalse;

	float s_release = 1.f;
	if (!streamer.readFloat(s_release)) return kResultFalse;

	float s_waveform = 0.f;
	if (!streamer.readFloat(s_waveform)) return kResultFalse;

	float s_arpduration = 0.5f;
	if (!streamer.readFloat(s_arpduration)) return kResultFalse;

	float s_arp = 0.f;
	if (!streamer.readFloat(s_arp)) return kResultFalse;

	setParamNormalized(kAttack, s_attack);
	setParamNormalized(kDecay, s_decay);
	setParamNormalized(kSustain, s_sustain);
	setParamNormalized(kRelease, s_release);
	setParamNormalized(kWaveform, s_waveform);
	setParamNormalized(kArpDuration, s_arpduration);
	setParamNormalized(kArpOnOff, s_arp);

	char text[32];
	int waveFormID = (int)(s_waveform * WaveForm::WAVEFORM_SIZE);

	switch (waveFormID)
	{
	case 0:
		sprintf(text, "%s", "Triangle");
		break;

	case 1:
		sprintf(text, "%s", "Sine");
		break;

	case 2:
		sprintf(text, "%s", "Saw");
		break;

	case 3:
		sprintf(text, "%s", "Pulse 12.5%");
		break;

	case 4:
		sprintf(text, "%s", "Pulse 25%");
		break;

	case 5:
		sprintf(text, "%s", "Pulse 50%");
		break;

	case 6:
		sprintf(text, "%s", "Pulse 75%");
		break;

	case 7:
	case 8: //for max case
		sprintf(text, "%s", "Noise");
		break;

	default: //what?
		sprintf(text, "%s", "ERROR");
		break;
	}

	updateWaveformLabel(text);

	streamer.seek(sizeof(float), Steinberg::FSeekMode::kSeekCurrent);
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitController::setState (IBStream* state)
{
	// Here you get the state of the controller
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor
	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API ABitController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		return new VST3Editor(this, "view", "abit.uidesc");
	}
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = EditControllerEx1::setParamNormalized (tag, value);
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	// called by host to get a string for given normalized value of a specific parameter
	// (without having to set the value!)

	switch (tag)
	{
	case kAttack:
	case kDecay:
	case kSustain:
	case kRelease:
	case kArpDuration:
	{
		char text[32];
		sprintf(text, "%2.f", (float)valueNormalized);
		UString(string, 128).fromAscii(text);

		return kResultTrue;
	}

	case kWaveform:
	{
		char text[32];
		int waveFormID = (int)((float)valueNormalized * WaveForm::WAVEFORM_SIZE);

		switch (waveFormID)
		{
		case 0:
			sprintf(text, "%s", "Triangle");
			break;

		case 1:
			sprintf(text, "%s", "Sine");
			break;

		case 2:
			sprintf(text, "%s", "Saw");
			break;

		case 3:
			sprintf(text, "%s", "Pulse 12.5%");
			break;

		case 4:
			sprintf(text, "%s", "Pulse 25%");
			break;

		case 5:
			sprintf(text, "%s", "Pulse 50%");
			break;

		case 6:
			sprintf(text, "%s", "Pulse 75%");
			break;

		case 7:
		case 8: //for max case
			sprintf(text, "%s", "Noise");
			break;

		default: //what?
			sprintf(text, "%s", "ERROR");
			break;
		}

		UString(string, 128).fromAscii(text);

		updateWaveformLabel(text);
		return kResultTrue;
	}

	case kArpOnOff:
		UString(string, 128).fromAscii(valueNormalized >= 0.5f ? "Arpeggio ON" : "Arpeggio OFF");
		return kResultTrue;

	default:
		return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
	}
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)

	return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

tresult PLUGIN_API ABitController::getMidiControllerAssignment(int32 butIndex, int16 channel, Vst::CtrlNumber midiControllerNumber, Vst::ParamID& id)
{
	return kResultFalse;
}
//------------------------------------------------------------------------

tresult PLUGIN_API ABitController::queryInterface(const char* iid, void** obj)
{
	QUERY_INTERFACE(iid, obj, Vst::IMidiMapping::iid, Vst::IMidiMapping);
	return EditControllerEx1::queryInterface(iid, obj);
}
//------------------------------------------------------------------------

CView* ABitController::verifyView(CView* view, const UIAttributes&, const IUIDescription*, VST3Editor*)
{
	CTextLabel* label = dynamic_cast<CTextLabel*>(view);
	if (label) waveformName = label;

	return view;
}

void ABitController::updateWaveformLabel(char* text)
{
	if (waveformName == nullptr) return;

	Steinberg::String str(text);
	str.toMultiByte(kCP_Utf8);
	waveformName->setText(str.text8());
}
}