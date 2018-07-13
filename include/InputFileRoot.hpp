#ifndef CU_INPUT_FILE_ROOT_HEADER
#define CU_INPUT_FILE_ROOT_HEADER
#include <set>
#include <string>
#include <memory>
#include "ComparePointer.hpp"
#include "InputFile.hpp"

class TChain;

namespace compass_unpack {

  class Event;
  
  class InputFileRoot : public InputFile {
  public:
    InputFileRoot(const std::string& run_directory);
    virtual ~InputFileRoot();

    virtual Long64_t ReadEvent(std::shared_ptr<compass_unpack::Event>& event);
    virtual Long64_t GetTotalEvents() const;
    virtual Long64_t GetEventNumber() const { return fEventNo; }
		virtual bool Good() const { return (fChain.get() != nullptr); }
		
  private:
		std::shared_ptr<TChain> fChain;
    UShort_t  fBoard;
    UShort_t  fChannel;
    UShort_t  fEnergy;
    UShort_t  fEnergyShort;
    ULong64_t fTimestampUnsigned;
    UInt_t    fFlags;
    Long64_t  fEventNo;
		Long64_t  fReturnedEvents;

		std::set<std::shared_ptr<Event>, CompareSharedPointer<Event> > fLocalBuffer;
  };

}


#endif
