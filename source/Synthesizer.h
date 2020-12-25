#ifndef __SYNTHESIZER__
#define __SYNTHESIZER__

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
#include "values.h"
#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/base/funknown.h"

namespace Kareus
{

	struct ADSR
	{
		float attack;
		float decay;
		float sustain;
		float release;

		float envelope;
		int maxLength;

		float attack_duration;
		float attack_value;

		float decay_duration;
		float decay_value;

		float release_duration;
		float release_value;

		ADSR()
		{
			attack = 0.f;
			decay = 0.f;
			sustain = 1.f;
			release = 0.f;
			envelope = 1.f;
			maxLength = 0;

			attack_duration = 0.f;
			attack_value = 0.f;
			decay_duration = 0.f;
			decay_value = 0.f;
			release_duration = 0.f;
			release_value = 0.f;
		}
	};

	struct Note
	{
		Steinberg::int32 id;
		Steinberg::int16 pitch;
		bool released;
		bool muted;
		float baseFrequency;
		float frequency;
		float phase;

		int arpOffset;
		int arpIndex;

		ADSR adsr;
	};

	enum WaveForm
	{
		TRIANGLE = 0, SINE, SAW, PULSE_12_5, PULSE_25, PULSE_5, PULSE_75, NOISE, WAVEFORM_SIZE
	};

	class Synthesizer
	{
	private:
		std::vector<Note*> notes;
		std::vector<int> arps;

		float TWO_PI_OVER_SR;
		float NOISE_PHASE;
		int sampleRate, bufferSize, max_envelope_samples, arp_duration;
		float duration_speed;

		bool doArp;
		bool enableArp;

		Note* getExistingNote(Steinberg::int16 pitch);
		Note* getNoteById(Steinberg::int32 id);

		bool removeNote(Note* note);

		void reset();

		void handleNoteAmountChange();
		float getArpFrequency(int index);
		bool isArpNote(Note* note);
		
		int note_ids;

		int getArpSpeedByTempo(float tempo);

		WaveForm waveform;

	public:
		Synthesizer();
		~Synthesizer();

		double TEMPO;

		void init(int sampleRate, double tempo);

		void noteOn(Steinberg::int16 pitch);
		void noteOff(Steinberg::int16 pitch);

		void update(float fAttack, float fDecay, float fSustain, float fRelease, WaveForm waveform);

		void setEnableArp(bool enableArp);

		void setArpDurationSpeed(float durationSpeed);

		void setNoiseBase(Steinberg::int16 pitch);

		bool synthesize(float** output, int numChannels, int bufferSize, Steinberg::uint32 sampleFramesSize);

		const static int ARP_THRESHOLD = 2;

		ADSR adsr;
	};
}
#endif