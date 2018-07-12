#include <fstream>
#include <iostream>
#include "Event.hpp"
#include "InputFile.hpp"

namespace cu = compass_unpack;

cu::Event::Event():
  fBoard(0),
  fChannel(0),
  fEnergy(0),
  fEnergyShort(0),
  fTimestamp(0),
	fFlags(0),
  fWaveform(0)
{  }

cu::Event::~Event()
{  }

bool cu::Event::operator< (const cu::Event& rhs) const
{
  const Event& lhs = *this;
  if(lhs.fTimestamp != rhs.fTimestamp)
    { return lhs.fTimestamp < rhs.fTimestamp; }
  if(lhs.fBoard != rhs.fBoard)
    { return lhs.fBoard < rhs.fBoard; }
  if(lhs.fChannel != rhs.fChannel)
    { return lhs.fChannel < rhs.fChannel; }
  return false;
}


void cu::Event::Print() const
{
	std::cout << "EVENT: (board, channel, energy, energy short, " <<
		"timestamp, flags, no. of waves): " <<
		fBoard << ", " << fChannel << ", " << fEnergy << ", " << fEnergyShort << ", " <<
		fTimestamp << ", " << fFlags << ", " << fWaveform.size() << "\n";
}
