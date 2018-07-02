#ifndef CUNPACK_INPUT_FILE_HEADER
#define CUNPACK_INPUT_FILE_HEADER
#include <vector>
#include <Rtypes.h>

namespace compass_unpack {

  class Event;
  
  class InputFile {
  public:
    InputFile() {}
    virtual ~InputFile() {}

    virtual Long64_t ReadEvent(Event&) = 0;
    virtual Long64_t GetEventNumber() const = 0;
    virtual Long64_t GetTotalEvents() const = 0;
  };
  
}


#endif
