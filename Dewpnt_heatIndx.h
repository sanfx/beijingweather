// Dewpnt_heatIndx.h

namespace Funcs
{
    class Dewpnt_heatIndx
    {
    public:
        // Returns dew point
        static double dewPointFast(double celsius, double humidity);

        // Returns heat index
        static double heatIndex(double tempF, double humidity);
    };
} 