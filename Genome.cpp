#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;
//using namespace std;

DaisyPatchSM hw;

//std::vector<int> Minor {0, 2, 3, 5, 7, 8, 10};
//std::vector<int> Major {0, 2, 4, 5, 6, 8, 10};

int main(void)
{
    /** Initialize the hardware */
    hw.Init();
    while(1)
    {
        /** Delay half a second */
        hw.Delay(500);
        /** Get a truly random float from the hardware */
        float voltage = hw.GetRandomValue() / (4294967296/ 8);
        float float_voltage = hw.GetRandomFloat(0.f, 10.f);

        /** Write it to both CV Outputs */
        hw.WriteCvOut(CV_OUT_1, voltage);
        hw.WriteCvOut(CV_OUT_2, voltage);
    }
}
