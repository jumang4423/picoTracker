#include "record.h"
#include <cstdio>

void Record(void *) {}

bool StartRecording(const char *filename, uint8_t threshold, uint32_t milliseconds) {
    return false;
}

void StopRecording() {}

void StartMonitoring() {}

void StopMonitoring() {}

void SetInputSource(RecordSource source) {}

void SetLineInGain(uint8_t gainDb) {}

void SetMicGain(uint8_t gainDb) {}

bool IsRecordingActive() {
    return false;
}
