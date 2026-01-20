#ifndef _PC_AUDIO_RECORD_H_
#define _PC_AUDIO_RECORD_H_

#include <cstdint>

// Dummy values for PC (can be implemented later)
#define LINEIN_GAIN_MINDB 0
#define LINEIN_GAIN_MAXDB 0
#define MIC_GAIN_MINDB 0
#define MIC_GAIN_MAXDB 0

enum RecordSource { AllOff, LineIn, Mic, USBIn };

void Record(void *);
bool StartRecording(const char *filename, uint8_t threshold,
                    uint32_t milliseconds);
void StopRecording();
void StartMonitoring();
void StopMonitoring();
void SetInputSource(RecordSource source);
void SetLineInGain(uint8_t gainDb);
void SetMicGain(uint8_t gainDb);
bool IsRecordingActive();

#endif
