#ifndef HYT2X_H
#define HYT2X_H

#include <inttypes.h>

typedef enum {
	HYT221_ADDR = 0x28,
} HYT_SENSOR_T;


class HYT2xClass
{
  private:
	unsigned int valuetemp;
	unsigned int valuecap;

  public:
    uint8_t isPresent(void);
    float GetHumidity(void);
    float GetTemperature(void);
    void triggerMeasurement(void);
    uint8_t readMeasurement(void);
    uint8_t measure(void);
};

extern HYT2xClass HYT2x;

#endif
