#include <cmath>
#include <algorithm>
#include "Synthesizer.h"

using namespace Steinberg;

namespace Kareus
{
	Synthesizer::Synthesizer()
	{
		doArp = false;
		enableArp = true;
		TEMPO = 120.f;

		TWO_PI_OVER_SR = Math::TWO_PI / 44100.f;
		NOISE_PHASE = MIDITable::frequencies[60] / 44100.f;
		sampleRate = 44100;
		bufferSize = 256;
		max_envelope_samples = 44100;
		arp_duration = 16;
		duration_speed = 1.f;
		waveform = WaveForm::TRIANGLE;

		note_ids = 0;
	}

	Synthesizer::~Synthesizer()
	{
		reset();
	}

	void Synthesizer::init(int sampleRate, double tempo)
	{
		this->sampleRate = sampleRate;
		TWO_PI_OVER_SR = Math::TWO_PI / (float)sampleRate;

		max_envelope_samples = (int)round(1.f * sampleRate);

		TEMPO = tempo;
	}

	void Synthesizer::noteOn(int16 pitch)
	{
		if (getExistingNote(pitch) != nullptr) removeNote(getExistingNote(pitch));

		Note* note = new Note();
		note->id = ++note_ids;
		note->pitch = pitch;
		note->released = false;
		note->muted = false;
		note->baseFrequency = MIDITable::frequencies[pitch];
		note->frequency = note->baseFrequency;
		note->phase = 0.f;
		note->arpIndex = 0;
		note->arpOffset = 0;

		notes.push_back(note);
		handleNoteAmountChange();

		if (doArp && enableArp)
		{
			int fullMeasure = round(((float)sampleRate * 60.f) / TEMPO * duration_speed);
			arp_duration = fullMeasure / getArpSpeedByTempo(TEMPO);
		}

		note->adsr.attack = adsr.attack;
		note->adsr.attack_value = 0.f;
		note->adsr.attack_duration = std::max(1.f, (float)max_envelope_samples * note->adsr.attack);

		note->adsr.sustain = adsr.sustain;

		note->adsr.decay = adsr.decay;
		note->adsr.decay_value = 0.f;
		note->adsr.decay_duration = std::max(1.f, (float)max_envelope_samples * note->adsr.decay);
	}

	void Synthesizer::noteOff(int16 pitch)
	{
		Note* note = getExistingNote(pitch);

		if (note == nullptr) return;

		if (adsr.release == 0 || note->adsr.sustain == 0)
		{
			removeNote(note);
			return;
		}

		if (!note->released)
		{
			note->adsr.release = adsr.release;
			note->adsr.release_value = 0.f;
			note->adsr.release_duration = std::max(1.f, (float)max_envelope_samples * note->adsr.release);

			note->released = true;

			if (doArp && enableArp)
			{
				arps.erase(std::find(arps.begin(), arps.end(), note->id));
				handleNoteAmountChange();
			}
		}
	}

	Note* Synthesizer::getExistingNote(int16 pitch)
	{
		for (Note* note : notes)
			if (note->pitch == pitch) return note;

		return nullptr;
	}

	Note* Synthesizer::getNoteById(int32 id)
	{
		for (Note* note : notes)
			if (note->id == id) return note;

		return nullptr;
	}

	bool Synthesizer::removeNote(Note* note)
	{
		if (isArpNote(note)) arps.erase(std::find(arps.begin(), arps.end(), note->id));

		auto iter = std::find(notes.begin(), notes.end(), note);

		if (iter != notes.end())
		{
			notes.erase(iter);
			handleNoteAmountChange();
			delete note;
			return true;
		}

		return false;
	}

	void Synthesizer::reset()
	{
		while (!notes.empty()) removeNote(notes.at(0));

		arps.clear();

		note_ids = 0;
	}

	void Synthesizer::handleNoteAmountChange()
	{
		int active = 0;

		for (Note* note : notes)
			if (!note->released) active++;

		doArp = (active >= ARP_THRESHOLD); //separate conditions from enableArp, so that we can simultaneously synthesize sounds when enableArp toggling

		for (Note* note : notes)
		{
			note->muted = doArp && enableArp;

			if (doArp && enableArp && !isArpNote(note)) arps.push_back(note->id);
		}

		if (!(doArp && enableArp)) return;

		for (Note* note : notes)
			if (!note->released)
			{
				note->muted = false;
				break;
			}
		
	}

	void Synthesizer::update(float attack, float decay, float sustain, float release, WaveForm waveform)
	{
		adsr.attack = attack;
		adsr.decay = decay;
		adsr.sustain = sustain;
		adsr.release = release;
		this->waveform = waveform;
	}

	void Synthesizer::setEnableArp(bool enableArp)
	{
		this->enableArp = enableArp;
	}

	void Synthesizer::setArpDurationSpeed(float durationSpeed)
	{
		duration_speed = std::max(0.25f, durationSpeed);
		if (doArp && enableArp)
		{
			int fullMeasure = round(((float)sampleRate * 60.f) / TEMPO * duration_speed);
			arp_duration = fullMeasure / getArpSpeedByTempo(TEMPO);
		}
	}

	void Synthesizer::setNoiseBase(Steinberg::int16 pitch)
	{
		NOISE_PHASE = MIDITable::frequencies[pitch] / (float)sampleRate;
		//update frequency of noise random value will be used as NOISE_PHASE
	}

	bool Synthesizer::synthesize(float** output, int numChannels, int bufferSize, uint32 sampleFramesSize)
	{
		for (int32 i = 0; i < numChannels; i++)
			memset(output[i], 0, sampleFramesSize);

		if (notes.empty()) return false;

		float phase, envelope, wave = 0, out;

		int arpIndex = -1;

		for (Note* event : notes)
		{
			bool doAttack = event->adsr.attack > 0.f;
			bool doDecay = event->adsr.decay > 0.f && event->adsr.sustain != 1.f;
			bool doRelease = event->adsr.release > 0.f && event->released;

			if (event->muted)
			{
				if (doRelease)
				{
					event->adsr.release_value += (event->adsr.sustain / event->adsr.release_duration * bufferSize);
					if (event->adsr.sustain - event->adsr.release_value < 0.f) removeNote(event);
				}

				continue;
			}

			bool arp = doArp && isArpNote(event) && enableArp;

			if (arpIndex == -1 && arp)
				arpIndex = event->arpIndex;

			bool disposeEvent = false;

			phase = event->phase;
			int randomvalue = 0;
			float phaseSum = 0.f;

			for (int32 i = 0; i < bufferSize; i++)
			{
				if (phaseSum == 0.f) randomvalue = rand();

				if (arp && event->arpOffset == 0)
				{
					event->frequency = getArpFrequency(arpIndex);
					event->arpOffset = arp_duration;

					if (++arpIndex == notes.size()) arpIndex = 0;
				}

				switch (waveform)
				{
				case WaveForm::TRIANGLE:
					if (phase < 0.25f) wave = 4 * phase;
					else if (phase > 0.75f) wave = 4 * phase - 4;
					else wave = -4 * phase + 2;
					break;

				case WaveForm::SINE:
					wave = sin(-phase * Math::TWO_PI);
					break;

				case WaveForm::PULSE_12_5:
					wave = (phase < 0.125f) ? 1.f : -1.f;
					break;

				case WaveForm::PULSE_25:
					wave = (phase < 0.25f) ? 1.f : -1.f;
					break;

				case WaveForm::PULSE_5:
					wave = (phase < 0.5f) ? 1.f : -1.f;
					break;

				case WaveForm::PULSE_75:
					wave = (phase < 0.75f) ? 1.f : -1.f;
					break;

				case WaveForm::SAW:
					wave = 1 - 2 * phase;
					break;

				case WaveForm::NOISE:
					wave = 2 * (randomvalue / (float)RAND_MAX) - 1;
					break;
				}

				out = wave;

				float f = event->frequency / (float)sampleRate;
				phase += f;
				phaseSum += f;

				if (phase >= 1.f) phase = 0.f;
				if (phaseSum >= NOISE_PHASE) phaseSum = 0.f;

				if (event->arpOffset > 0) event->arpOffset--;

				envelope = 1.f;

				if (doRelease)
				{
					envelope = event->adsr.sustain - event->adsr.release_value;

					if (envelope < 0.f)
					{
						envelope = 0.f;
						disposeEvent = true;
					}

					event->adsr.release_value += event->adsr.sustain / event->adsr.release_duration;
					out *= envelope;
				}
				else
				{
					if (doAttack && event->adsr.attack_value < event->adsr.attack)
					{
						event->adsr.envelope = event->adsr.attack_value;
						event->adsr.attack_value += 1.f / event->adsr.attack_duration;
						out *= event->adsr.envelope;
					}
					else if (doDecay && event->adsr.envelope > event->adsr.sustain)
					{
						event->adsr.envelope -= (1.f - event->adsr.sustain) / event->adsr.decay_duration;
						out *= event->adsr.envelope;
					}
					else //sustain
						out *= event->adsr.sustain;
				}

				for (int32 k = 0; k < numChannels; k++) output[k][i] += out;

				if (disposeEvent) break;
			}

			if (disposeEvent) removeNote(event);
			else
			{
				event->phase = phase;
				if (arp) event->arpIndex = arpIndex;
			}
		}

		return true;
	}

	float Synthesizer::getArpFrequency(int index)
	{
		if (arps.empty()) return 0.f;

		index = std::min(index, (int)(arps.size() - 1));

		//for (int i = index; i < arps.size(); i++) // original order that works in descending order
		for (int i = arps.size() - index - 1; i >= 0; i--) // reversed search order to make arpeggio ascending order, but I'm not sure this works well without error (there can be dummy notes in the vector array)
		{
			Note* note = getNoteById(arps.at(i));
			if (note != nullptr) return note->baseFrequency;
		}

		//return getNoteById(arps.at(0))->baseFrequency; //same theory as above
		return getNoteById(arps.at(arps.size() - 1))->baseFrequency;
	}

	int Synthesizer::getArpSpeedByTempo(float tempo)
	{
		if (tempo >= 400.f) return 4;
		if (tempo >= 200.f) return 8;
		if (tempo >= 120.f) return 16;
		if (tempo >= 50.f) return 32;
		if (tempo >= 40.f) return 64;

		return 128;
	}

	bool Synthesizer::isArpNote(Note* note)
	{
		return std::find(arps.begin(), arps.end(), note->id) != arps.end();
	}
}