#include <fstream>
#include <iostream>
#include "Event.hpp"
#include "InputFile.hpp"

namespace cu = compass_unpack;

std::atomic<Long64_t> cu::Event::kMatchWindow (10e6);

cu::Event::Event():
  fBoard(0),
  fChannel(0),
  fEnergy(0),
  fEnergyShort(0),
  fTimestamp(0),
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
