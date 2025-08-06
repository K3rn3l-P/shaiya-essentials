#pragma once
#include <vector>
#include <windows.h>
#include "include/shaiya/include/CTexture.h"

// Classe per gestire una sequenza di CTexture* come animazione tipo GIF
class GifAnimator {
public:
    GifAnimator(int frameTimeMs = 100) : frameTimeMs_(frameTimeMs) {}

    void setFrames(const std::vector<shaiya::CTexture*>& frames) {
        frames_ = frames;
    }

    void addFrame(shaiya::CTexture* frame) {
        frames_.push_back(frame);
    }

    void clear() {
        frames_.clear();
    }

    int frameCount() const {
        return (int)frames_.size();
    }

    // Restituisce il frame corrente in base al tempo
    shaiya::CTexture* getCurrentFrame() const {
        if (frames_.empty()) return nullptr;
        DWORD now = GetTickCount();
        int idx = (now / frameTimeMs_) % frames_.size();
        return frames_[idx];
    }

    void setFrameTime(int ms) { frameTimeMs_ = ms; }
    int getFrameTime() const { return frameTimeMs_; }

private:
    std::vector<shaiya::CTexture*> frames_;
    int frameTimeMs_;
};