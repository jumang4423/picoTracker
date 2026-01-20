#ifndef _PC_SAMPLE_POOL_H_
#define _PC_SAMPLE_POOL_H_

#include "Application/Instruments/SamplePool.h"
#include <vector>

class PCSamplePool : public SamplePool {
public:
    PCSamplePool();
    virtual ~PCSamplePool();
    
    virtual void Reset();
    virtual bool CheckSampleFits(int sampleSize);
    virtual uint32_t GetAvailableSampleStorageSpace();
    virtual bool unloadSample(uint32_t i);
    
protected:
    virtual bool loadSample(const char *name);
    
private:
    std::vector<uint8_t> sampleMemory_; 
    size_t currentOffset_;
};

#endif
