#include "DSparser.h"

#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <stdexcept>

namespace DS {
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <stdexcept>

    using namespace std;

    float noteNameToMidi(const string& noteName) {
        static map<string, int> noteMap = {
            {"C", 0}, {"C#", 1}, {"Db", 1},
            {"D", 2}, {"D#", 3}, {"Eb", 3},
            {"E", 4}, {"Fb", 4}, {"E#", 5},
            {"F", 5}, {"F#", 6}, {"Gb", 6},
            {"G", 7}, {"G#", 8}, {"Ab", 8},
            {"A", 9}, {"A#", 10}, {"Bb", 10},
            {"B", 11}, {"Cb", 11}, {"B#", 0}
        };

        size_t i = 0;
        while (i < noteName.size() && (noteName[i] < '0' || noteName[i] > '9') && noteName[i] != '-') {
            ++i;
        }
        string notePart = noteName.substr(0, i);
        string octavePart = noteName.substr(i);

        auto it = noteMap.find(notePart);
        if (it == noteMap.end()) {
            throw invalid_argument("Invalid note name: " + noteName);
        }

        int pitchClass = it->second;
        int octave = stoi(octavePart);
        int midiNumber = (octave + 1) * 12 + pitchClass;
        return static_cast<float>(midiNumber);
    }

    vector<float> processMidiWithRest(const vector<string>& notes) {
        int n = notes.size();
        vector<float> midiValues(n, -1.0f);

        // 初始化MIDI值
        for (int i = 0; i < n; ++i) {
            if (notes[i] != "rest") {
                midiValues[i] = noteNameToMidi(notes[i]);
            }
        }

        vector<float> prevMidi(n, -1.0f);
        vector<float> nextMidi(n, -1.0f);

        // 前向遍历记录最近有效MIDI
        float currentPrev = -1.0f;
        for (int i = 0; i < n; ++i) {
            if (notes[i] == "rest") {
                prevMidi[i] = currentPrev;
            }
            else {
                prevMidi[i] = currentPrev;
                currentPrev = midiValues[i];
            }
        }

        // 后向遍历记录最近有效MIDI
        float currentNext = -1.0f;
        for (int i = n - 1; i >= 0; --i) {
            if (notes[i] == "rest") {
                nextMidi[i] = currentNext;
            }
            else {
                nextMidi[i] = currentNext;
                currentNext = midiValues[i];
            }
        }

        // 计算默认MIDI值（对应160Hz的MIDI）
        const float defaultMidi = 69.0f + 12.0f * log2(160.0f / 440.0f);

        // 处理休止符
        for (int i = 0; i < n; ++i) {
            if (notes[i] == "rest") {
                if (prevMidi[i] != -1.0f) {
                    midiValues[i] = prevMidi[i];
                }
                else if (nextMidi[i] != -1.0f) {
                    midiValues[i] = nextMidi[i];
                }
                else {
                    midiValues[i] = defaultMidi;
                }
            }
        }

        return midiValues;
    }

    std::vector<float> parser::P_F_conversion(
        const std::vector<std::string>& notes
    )const {
        vector<float> midiValues = processMidiWithRest(notes);
        vector<float> frequencies;
        frequencies.reserve(midiValues.size());

        for (float midi : midiValues) {
            float freq = 440.0f * pow(2.0f, (midi - 69.0f) / 12.0f);
            frequencies.push_back(freq);
        }
        return frequencies;
    }

    std::vector<float> parser::P_M_conversion(
        const std::vector<std::string>& notes
    )const {
        std::vector<float> midi_float = processMidiWithRest(notes);
        for (int index = 0;index < midi_float.size();++index) {
            midi_float[index] = std::round(midi_float[index]);
        }
        return std::move(midi_float);
    }
}