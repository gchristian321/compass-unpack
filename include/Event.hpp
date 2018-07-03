#ifndef COMPASS_UNPACK_EVENT_HEADER
#define COMPASS_UNPACK_EVENT_HEADER
#include <atomic>
#include <vector>
#include <Rtypes.h>


namespace compass_unpack {
  
class InputFileBin;
  
class Event {
public:
	Event();
	~Event();
		
	UShort_t  GetBoard()       const { return fBoard;       }
	UShort_t  GetChannel()     const { return fChannel;     }
	UShort_t  GetEnergy()      const { return fEnergy;      }
	UShort_t  GetEnergyShort() const { return fEnergyShort; }
	Long64_t  GetTimestamp()   const { return fTimestamp;   }
	UInt_t    GetFlags()       const { return fFlags;       }
	const std::vector<UShort_t>& GetWaveform() const { return fWaveform; }
		
	bool operator< (const Event& rhs) const;
		
private:
	UShort_t  fBoard;       
	UShort_t  fChannel;
	UShort_t  fEnergy;
	UShort_t  fEnergyShort;
	Long64_t  fTimestamp;
	UInt_t    fFlags;
	std::vector<UShort_t> fWaveform;

public:
	static void SetMatchWindow(Long64_t Window) { kMatchWindow = Window; }
	static Long64_t GetMatchWindow() { return kMatchWindow;   } 
private:
	static std::atomic<Long64_t> kMatchWindow;

	friend class compass_unpack::InputFileBin;
};
  
}


#endif
