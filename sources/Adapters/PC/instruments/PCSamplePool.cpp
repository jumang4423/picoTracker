#include "PCSamplePool.h"
#include "System/Console/Trace.h"
#include <cstring>
#include <iostream>

#define PC_SAMPLE_MEMORY_SIZE (16 * 1024 * 1024) // 16MB

PCSamplePool::PCSamplePool() : SamplePool() {
    sampleMemory_.resize(PC_SAMPLE_MEMORY_SIZE);
    currentOffset_ = 0;
}

PCSamplePool::~PCSamplePool() {}

void PCSamplePool::Reset() {
    count_ = 0;
    for (int i = 0; i < MAX_SAMPLES; i++) {
        wav_[i].Close();
        nameStore_[i][0] = '\0';
    }
    currentOffset_ = 0; // Reset memory pointer
    std::fill(sampleMemory_.begin(), sampleMemory_.end(), 0);
}

bool PCSamplePool::CheckSampleFits(int sampleSize) {
    return (currentOffset_ + sampleSize) <= PC_SAMPLE_MEMORY_SIZE;
}

uint32_t PCSamplePool::GetAvailableSampleStorageSpace() {
    return PC_SAMPLE_MEMORY_SIZE - currentOffset_;
}

bool PCSamplePool::unloadSample(uint32_t i) {
    // Implementing unload is complex with linear allocation, 
    // but for now return false or implement simple logic if needed.
    // Pico version returns false.
    return false;
}

bool PCSamplePool::loadSample(const char *name) {
    Trace::Log("SAMPLEPOOL", "Loading sample (PC): %s", name);
    
    if (count_ == MAX_SAMPLES) {
        Trace::Error("Max samples reached");
        return false;
    }
    
    if (!wav_[count_].Open(name)) {
        Trace::Error("Failed to open sample: %s", name);
        return false;
    }
    
    // Copy name
    strncpy(nameStore_[count_], name, MAX_INSTRUMENT_FILENAME_LENGTH);
    nameStore_[count_][MAX_INSTRUMENT_FILENAME_LENGTH] = '\0';
    
    // Load data into memory
    uint32_t size = wav_[count_].GetDiskSize(-1);
    
    if (!CheckSampleFits(size)) {
         Trace::Error("Not enough memory for sample: %s", name);
         wav_[count_].Close();
         return false;
    }
    
    // Align?
    // WAV uses shorts usually?
    
    // Set buffer pointer in WavFile
    uint8_t *dest = &sampleMemory_[currentOffset_];
    wav_[count_].SetSampleBuffer((short*)dest);
    
    // Read data
    uint32_t bytesRead = 0;
    wav_[count_].Rewind();
    
    // We need to read in chunks or full?
    // WavFile::Read takes a buffer.
    // We can read directly into destination?
    // WavFile Read method signature: Read(void *buffer, uint32_t size, uint32_t *bytesRead)
    
    wav_[count_].Read(dest, size, &bytesRead);
    
    if (bytesRead == 0 && size > 0) {
         Trace::Error("Read 0 bytes for sample: %s", name);
    }
    
    count_++;
    currentOffset_ += size;
    // Align offset?
    if (currentOffset_ % 2 != 0) currentOffset_++; 
    
    return true;
}
