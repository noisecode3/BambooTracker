#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include "abstract_instrument_property.hpp"

class WaveformADPCM : public AbstractInstrumentProperty
{
public:
	explicit WaveformADPCM(int num);
	WaveformADPCM(const WaveformADPCM& other);

	friend bool operator==(const WaveformADPCM& a, const WaveformADPCM& b);
	friend bool operator!=(const WaveformADPCM& a, const WaveformADPCM& b) { return !(a == b); }

	std::unique_ptr<WaveformADPCM> clone();

	void setRootKeyNumber(int n);
	int getRootKeyNumber() const;
	void setRootDeltaN(int dn);
	int getRootDeltaN() const;
	void setRepeatEnabled(bool enabled);
	bool isRepeatable() const;
	void storeSamples(std::vector<uint8_t> samples);
	std::vector<uint8_t> getSamples() const;

	bool isEdited() const;

private:
	int rootKeyNum_, rootDeltaN_;
	bool isRepeated_;
	std::vector<uint8_t> samples_;

	static constexpr int DEF_RT_KEY_ = 60;	// C5
	static constexpr int DEF_RT_DELTAN_ = 0x49cd;	// 16000Hz
	static constexpr bool DEF_REPET_ = false;
};
