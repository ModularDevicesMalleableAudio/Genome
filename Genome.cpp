#include "daisy_patch_sm.h"
#include "daisysp.h"
#include <cmath>

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;
//using namespace std;

struct Sequence {
    std::vector<float> notes;
};

DaisyPatchSM hw;
Switch       toggle;
Sequence     sequence;

//std::vector<int> Minor {0, 2, 3, 5, 7, 8, 10};
//std::vector<int> Major {0, 2, 4, 5, 6, 8, 10};
int          counters[2] = {0, 0 };
int          divisions[2] = {4, 3 };
float        led_voltage = 0.f;
float        octave = 1.f;
float        note_input_2 = 0.f;

int main() {
    /** Initialize the hardware */
    hw.Init();
    toggle.Init(DaisyPatchSM::B8);
    sequence.notes = {0.f, 2.f, 5.f, 8.f};

    for(;;)
    {
        hw.ProcessAllControls();

        /** Update octave from knob CV_1 (0, 1) */
        octave = round(fmap(hw.GetAdcValue(CV_1),1,5));

        if (hw.GetAdcValue(CV_6) > kOneTwelfth) {
            note_input_2 = hw.GetAdcValue(CV_6);
        }
        else {
            note_input_2 = 0;
        };


        // Get recording armed state from the switch
        toggle.Debounce();
        bool recording_armed = toggle.Pressed();

        // Show recording state via LED
        led_voltage = static_cast<float>(recording_armed) * 5.0f;
        hw.WriteCvOut(CV_OUT_2, led_voltage);

//        float led_gate_voltage = static_cast<float>(hw.gate_in_1.State());
//        hw.WriteCvOut(CV_OUT_2, led_gate_voltage);

        // Advance counters, output next CV value
        if (hw.gate_in_1.Trig()) {

            /** Advance counters, move forward a step */
            counters[0] += 1;
            counters[0] = counters[0] % divisions[0];

            // Record new CV value if recording armed
            if (recording_armed) {
                // voltage in is -1 to 1, but we map to octaves
                float note_input = round(fmap(hw.GetAdcValue(CV_5), -12.f, 12.f));
                sequence.notes.at(counters[0]) = note_input;
            }

            // Get CV value for current step and output
            float voltage = (sequence.notes.at(counters[0]) + (octave * 12.f)) * kOneTwelfth;
            hw.WriteCvOut(CV_OUT_1, voltage + note_input_2);

        }
    }
}
