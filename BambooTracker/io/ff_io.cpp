/*
 * Copyright (C) 2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ff_io.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include "instrument.hpp"
#include "file_io_error.hpp"

FfIO::FfIO() : AbstractBankIO("ff", "PMD FF", true, false) {}

AbstractBank* FfIO::load(const BinaryContainer& ctr) const
{
	size_t csr = 0;
	// File size check
	size_t ctrSize = ctr.size();
	if (!ctrSize || ctrSize & 0x1f || ctrSize > 0x2000)
		throw FileCorruptionError(FileIO::FileType::Bank, csr);

	std::vector<int> ids;
	std::vector<std::string> names;
	std::vector<BinaryContainer> ctrs;
	int max = static_cast<int>(ctrSize / 0x20);
	for (int i = 0; i < max; ++i) {
		BinaryContainer block = ctr.getSubcontainer(csr, 25);
		csr += 25;
		std::string name = "";
		for (size_t j = 0; j < 7; ++j) {
			if (char c = ctr.readChar(csr + j)) name += c;
			else break;
		}
		csr += 7;

		// Empty
		if (std::all_of(block.getPointer(), block.getPointer() + 25,
						[](const char c) { return c == 0; }) && name.empty())
			continue;

		ids.push_back(i);
		names.push_back(name);
		ctrs.push_back(block);
	}

	return new FfBank(std::move(ids), std::move(names), std::move(ctrs));
}

AbstractInstrument* FfIO::loadInstrument(const BinaryContainer& instCtr,
										 const std::string& name,
										 std::weak_ptr<InstrumentsManager> instMan,
										 int instNum)
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Bank, 0);

	InstrumentFM* fm = new InstrumentFM(instNum, name, instManLocked.get());
	fm->setEnvelopeNumber(envIdx);

	size_t csr = 0;
	uint8_t tmp = instCtr.readUint8(csr++);
	uint8_t ssgeg1 = (tmp & 0x80) ? 8 : 0;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, (tmp & 0x70) >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	uint8_t ssgeg3 = (tmp & 0x80) ? 8 : 0;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, (tmp & 0x70) >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	uint8_t ssgeg2 = (tmp & 0x80) ? 8 : 0;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, (tmp & 0x70) >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	uint8_t ssgeg4 = (tmp & 0x80) ? 8 : 0;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, (tmp & 0x70) >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	if (tmp & 0x80) ssgeg1 |= 4;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, tmp & 0x7f);
	tmp = instCtr.readUint8(csr++);
	if (tmp & 0x80) ssgeg3 |= 4;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, tmp & 0x7f);
	tmp = instCtr.readUint8(csr++);
	if (tmp & 0x80) ssgeg2 |= 4;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, tmp & 0x7f);
	tmp = instCtr.readUint8(csr++);
	if (tmp & 0x80) ssgeg4 |= 4;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, tmp & 0x7f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	ssgeg1 |= ((tmp & 0x60) >> 5);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1,
										  (ssgeg1 & 8) ? ssgeg1 & 7 : -1);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	ssgeg3 |= ((tmp & 0x60) >> 5);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3,
										  (ssgeg3 & 8) ? ssgeg3 & 7 : -1);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	ssgeg2 |= ((tmp & 0x60) >> 5);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2,
										  (ssgeg2 & 8) ? ssgeg2 & 7 : -1);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	ssgeg4 |= ((tmp & 0x60) >> 5);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4,
										  (ssgeg4 & 8) ? ssgeg4 & 7 : -1);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, tmp & 0x1f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, tmp);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, tmp & 0x0f);
	tmp = instCtr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, tmp >> 3);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, tmp & 0x07);

	return fm;
}
