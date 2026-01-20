#ifndef _PC_AUDIO_H_
#define _PC_AUDIO_H_

#include "Services/Audio/Audio.h"

class PCAudio : public Audio {
public:
    PCAudio(AudioSettings &settings);
    virtual ~PCAudio();
    
    virtual void Init() override;
    virtual void Close() override;
    virtual int GetSampleRate() override;
};
#endif
