//------------------------------------------------------------------------
// Copyright(c) 2020 Kareus.
//------------------------------------------------------------------------

#include "ABitProcessor.h"
#include "ABitcids.h"
#include "ABitParamIds.h"

#include "base/source/fstreamer.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstevents.h"

using namespace Steinberg;

namespace Kareus {
//------------------------------------------------------------------------
// ABitProcessor
//------------------------------------------------------------------------
ABitProcessor::ABitProcessor () : attack(0.f), decay(1.f), sustain(0.5f), release(0.f), waveform(WaveForm::TRIANGLE), arp(false), arpduration_f(0.5f), synth(nullptr)
{
	//--- set the wanted controller for our processor
	setControllerClass (kABitControllerUID);
}

//------------------------------------------------------------------------
ABitProcessor::~ABitProcessor ()
{
	delete synth;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitProcessor::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

    if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (Vst::IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData (index))
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
				switch (paramQueue->getParameterId())
				{
				case ABitParamId::kAttack:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						attack = (float)value;
					break;

				case ABitParamId::kDecay:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						decay = (float)value;
					break;

				case ABitParamId::kSustain:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						sustain = (float)value;
					break;

				case ABitParamId::kRelease:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						release = (float)value;
					break;

				case ABitParamId::kWaveform:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
					{
						waveform_f = (float)value;
						waveform = std::min((WaveForm)((int)((float)value * WaveForm::WAVEFORM_SIZE)), WaveForm::NOISE);
					}
					break;

				case ABitParamId::kArpDuration:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						arpduration_f = (float)value;
					break;

				case ABitParamId::kArpOnOff:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
						arp = ((float)value >= 0.5f);
					break;
				}
				update();
			}
		}
	}
	
	//--- Here you have to implement your processing

	if (data.processContext != nullptr && synth->TEMPO != data.processContext->tempo)
		synth->init((int)data.processContext->sampleRate, data.processContext->tempo);

	Vst::IEventList* eventList = data.inputEvents;
	if (eventList)
	{
		int32 numEvents = eventList->getEventCount();

		for (int32 index = 0; index < numEvents; index++)
		{
			Vst::Event event;
			if (eventList->getEvent(index, event) == kResultOk)
			{
				switch (event.type)
				{
				case Vst::Event::kNoteOnEvent:
					synth->noteOn(event.noteOn.pitch);
					break;

				case Vst::Event::kNoteOffEvent:
					synth->noteOff(event.noteOff.pitch);
					break;
				}
			}
		}
	}

	if (data.numOutputs == 0) return kResultOk;

	int32 numChannels = data.outputs[0].numChannels;

	uint32 sampleFramesSize = Vst::getSampleFramesSizeInBytes(processSetup, data.numSamples);
	void** out = Vst::getChannelBuffersPointer(processSetup, data.outputs[0]);

	data.outputs[0].silenceFlags = !synth->synthesize((float**)out, numChannels, data.numSamples, sampleFramesSize);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	
	synth = new Synthesizer();
	synth->init(newSetup.sampleRate, 120.f);
	
	update();

	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue;

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitProcessor::setState(IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer(state, kLittleEndian);

	float s_attack = 0.f;
	if (!streamer.readFloat(s_attack)) return kResultFalse;

	float s_decay = 0.f;
	if (!streamer.readFloat(s_decay)) return kResultFalse;

	float s_sustain = 0.f;
	if (!streamer.readFloat(s_sustain)) return kResultFalse;

	float s_release = 0.f;
	if (!streamer.readFloat(s_release)) return kResultFalse;

	float s_waveform = 0.f;
	if (!streamer.readFloat(s_waveform)) return kResultFalse;

	float s_arpduration = 0.f;
	if (!streamer.readFloat(s_arpduration)) return kResultFalse;

	float s_arp = 0.f;
	if (!streamer.readFloat(s_arp)) return kResultFalse;

	attack = s_attack;
	decay = s_decay;
	sustain = s_sustain;
	release = s_release;
	waveform_f = s_waveform;
	arpduration_f = s_arpduration;
	arp = (s_arp >= 0.5f);

	waveform = std::min((WaveForm)((int)((float)s_waveform * WaveForm::WAVEFORM_SIZE)), WaveForm::NOISE);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ABitProcessor::getState(IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer(state, kLittleEndian);

	if (!streamer.writeFloat(attack)) return kResultFalse;
	if (!streamer.writeFloat(decay)) return kResultFalse;
	if (!streamer.writeFloat(sustain)) return kResultFalse;
	if (!streamer.writeFloat(release)) return kResultFalse;
	if (!streamer.writeFloat(waveform_f)) return kResultFalse;
	if (!streamer.writeFloat(arpduration_f)) return kResultFalse;
	if (!streamer.writeFloat(arp ? 1.f : 0.f)) return kResultFalse;

	return kResultOk;
}

tresult PLUGIN_API ABitProcessor::setBusArrangements(Vst::SpeakerArrangement* inputs, int32 numIns, Vst::SpeakerArrangement* outputs, int32 numOuts)
{
	if (numIns == 1 && numOuts == 1)
	{
		if (Vst::SpeakerArr::getChannelCount(inputs[0]) == 1 && Vst::SpeakerArr::getChannelCount(outputs[0]) == 1)
		{
			Vst::AudioBus* bus = FCast<Vst::AudioBus>(audioInputs.at(0));
			if (bus)
			{
				if (bus->getArrangement() != inputs[0])
				{
					removeAudioBusses();
					addAudioInput(STR16("Mono In"), inputs[0]);
					addAudioOutput(STR16("Mono Out"), outputs[0]);
				}

				return kResultOk;
			}
		}
		else
		{
			Vst::AudioBus* bus = FCast<Vst::AudioBus>(audioInputs.at(0));
			if (bus)
			{
				if (Vst::SpeakerArr::getChannelCount(inputs[0]) == 2 && Vst::SpeakerArr::getChannelCount(outputs[0]) == 2)
				{
					removeAudioBusses();
					addAudioInput(STR16("Stereo In"), inputs[0]);
					addAudioOutput(STR16("Stereo Out"), outputs[0]);
					return kResultTrue;
				}
				else if (bus->getArrangement() != Vst::SpeakerArr::kStereo)
				{
					removeAudioBusses();
					addAudioInput(STR16("Stereo In"), Vst::SpeakerArr::kStereo);
					addAudioOutput(STR16("Stereo Out"), Vst::SpeakerArr::kStereo);
					return kResultTrue;
				}
			}
		}
	}

	return kResultFalse;
}

tresult PLUGIN_API ABitProcessor::notify(Vst::IMessage* message)
{
	if (!message) return kResultFalse;
	return Vst::AudioEffect::notify(message);
}

void ABitProcessor::update()
{
	synth->update(attack, decay, sustain, release, waveform);
	synth->setEnableArp(arp);
	synth->setArpDurationSpeed(pow(16, arpduration_f) / 4.f);
}

//------------------------------------------------------------------------
} // namespace Kareus
