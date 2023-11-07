#include "daisy_patch_sm.h"
#include "daisysp.h"
#include <cmath>

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;
using namespace std;

struct Sequence {
    std::vector<float> notes;
};

DaisyPatchSM hw;
Switch       toggle;
Switch       button;
Sequence     sequence;

//std::vector<int> Minor {0, 2, 3, 5, 7, 8, 10};
//std::vector<int> Major {0, 2, 4, 5, 6, 8, 10};
int          button_idx = 0;
int          counters[2] = {0, 0 };
int          divisions[2] = {4, 3 };
bool         rec_state[2] = {false, true };
bool         recording_armed = false;
int          pos = 0;
int          seq_length = 1;
float        led_voltage = 0.f;
float        octave = 1.f;
float        note_input_2 = 0.f;


void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        OUT_L[i] = IN_L[i]; /**< Copy the left input to the left output */
        OUT_R[i] = IN_R[i]; /**< Copy the right input to the right output */
    }
}

int GetOutputState(int counter, int state)
{
    if (counter == 0)
    {
        return state;
    }
    return 0;
}


int main() {
    /** Initialize the hardware */
    hw.Init();
    toggle.Init(daisy::patch_sm::DaisyPatchSM::B8);
    button.Init(daisy::patch_sm::DaisyPatchSM::B7);
    sequence.notes = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    /** Start Processing the audio */
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessAllControls();
        /** Update octave from knob CV_1 (0,1) mapped to (0,5) */
        octave = round(fmap(hw.GetAdcValue(CV_1),1,5));
        /** Update sequence length from knob CV_2 (0,1) mapped to (1,8) */
        seq_length = static_cast<int>(round(fmap(hw.GetAdcValue(CV_2),1,8)));

        // debounce switch and button
        toggle.Debounce();
        button.Debounce();

        // Set Divisions /2 with toggle
        bool toggle_state = toggle.Pressed();
        if (toggle_state)
        {
            divisions[0] = 2;
            divisions[1] = 3;
        }
        else
        {
            divisions[0] = 4;
            divisions[1] = 6;
        }

        // Arm Recording using the button (latching on/off)
        if (button.RisingEdge()) {
            button_idx += 1;
            button_idx = button_idx % 2;
        }
        // Show recording state via LED
        recording_armed = rec_state[button_idx];
        led_voltage = static_cast<float>(recording_armed) * 5.0f;

        // If we get a gate to gate_in 1 - Advance counters, output next CV value
        if (hw.gate_in_1.Trig()) {
            // Advance gate clock division counters
            for(int i = 0; i < 2; i++) {
                counters[i] += 1;
                counters[i] = counters[i] % divisions[i];
            }

            // advance position in the sequence
            pos += 1;
            pos = pos % seq_length;

            // Record new CV value if recording armed
            if (recording_armed) {
                // voltage in is -1 to 1, but we map to octaves
                float note_input = round(fmap(hw.GetAdcValue(CV_5), -12.f, 12.f));
                sequence.notes.at(pos) = note_input;
            }
        }

        // Get CV value for current step
        float voltage = (sequence.notes.at(pos) + (octave * 12.f)) * kOneTwelfth;
        // get secondary v/oct input from CV_6
        if (hw.GetAdcValue(CV_6) > kOneTwelfth) {
            note_input_2 = hw.GetAdcValue(CV_6);
        }
        else {
            note_input_2 = 0;
        }
        // Combine and output on CV_1
        hw.WriteCvOut(CV_OUT_1, voltage + note_input_2);
        // Write voltage showing record arm toggle to CV_2 (LED)
        hw.WriteCvOut(CV_OUT_2, led_voltage);

        /** Get the current gate in state */
        bool state1 = hw.gate_in_1.State();
        // Get the current gate out state based on the counters
        int OutputState1 = GetOutputState(counters[0], state1);
        int OutputState2 = GetOutputState(counters[1], state1);
        dsy_gpio_write(&hw.gate_out_1, OutputState1);
        dsy_gpio_write(&hw.gate_out_2, OutputState2);
    }
}
