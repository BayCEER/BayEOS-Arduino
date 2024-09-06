#include <QuickStats.h>
float bat_voltage;
float t2[TMEAN2];
uint8_t current_t1i;
uint8_t current_t2i;
float t_mean_t2, t_sum_t1, temp0;
uint8_t t_mode = 0;
float y_cur[MITTELUNG];
uint8_t current_i = 0;
uint8_t calc_mode = 0;
QuickStats stats; // initialize an instance of this class
long adc[1];

float rainsum = 0;
float current = 0;
float current_mean = 0;


#ifndef WITH_RAINSUM
#define WITH_RAINSUM 0
#endif


#ifdef BayEOSLOGGER_H
#if WITH_RAINSUM
#define NUMBER_OF_CHANNELS 9
#else
#define NUMBER_OF_CHANNELS 5
#endif

float values[NUMBER_OF_CHANNELS];
uint8_t counted;

void delayBoard(unsigned long d) {
  if (client.connected) {
    unsigned long s = millis();
    while ((millis() - s) < d) {
      myLogger.handleCommand();
      myLogger.sendBinaryDump();
    }
  } else {
    delayLCB(d);
  }
}
#else
void delayBoard(unsigned long d){
    delayLCB(d);
}
#endif

#ifndef BayRF24_h
#define WITH_CHECKSUM 0
#endif

void handleAction0(void)
{
    UNSET_ACTION(0); // clear the flag
#ifdef BayEOSLOGGER_H
    if (myLogger._logged_flag)
    {
        myLogger._logged_flag = 0;
        counted = 0;
        for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++)
        {
            values[i] = 0;
        }
    }
#endif

    digitalWrite(POWER_PIN, HIGH); // power up modem AND power divider (for battery voltage read)
    analogReference(DEFAULT);
    delayBoard(100);
    bat_voltage = 3.3 * BAT_DIVIDER / 1023 * analogRead(A7); // Read battery voltage
    digitalWrite(POWER_PIN, LOW);                                  // power down modem AND power divider

    ntc.readResistance();
    temp0 = ntc.getTemp(0);
    t_sum_t1 += temp0;
    current_t1i++;
    if (current_t1i == TMEAN1)
    {
        t2[current_t2i] = t_sum_t1 / TMEAN1;
        t_mean_t2 = stats.average(t2, TMEAN2);
        current_t2i++;
        if (current_t2i == TMEAN2)
        {
            current_t2i = 0;
            t_mode = 1;
        }
        current_t1i = 0;
        t_sum_t1 = 0;
    }
    scale.power_up();
    scale.read_average(adc, 2);
    // scale.read_average(adc);
    uint8_t counts[1];
    while (scale.read_average_with_filter(adc, 3000, counts) == 0xf0000000)
    {
        delayBoard(30);
    }
    scale.power_down();
    client.startDataFrame(BayEOS_Float32le,WITH_CHECKSUM);          // Start the data frame with float values
    client.addChannelValue(millis()); // add uptime (ms) als 4 byte float
#ifdef BayEOSLOGGER_H
    counted++;
    myLogger._bat = bat_voltage * 1000;
    values[0] += bat_voltage;
    values[1] += temp0;
    values[2] += adc[0];
    values[3] += cal0.getWeight(adc[0], 20, 0);
#else
    client.addChannelValue(bat_voltage); // add battery voltage
    client.addChannelValue(temp0);
#ifdef BayRF24_h
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    sendOrBufferLCB();
    client.startDataFrame(BayEOS_Float32le,WITH_CHECKSUM); // Start the data frame with float values
#endif
    client.addChannelValue(adc[0], 4);
    client.addChannelValue(cal0.getWeight(adc[0], 20, 0));
#endif

    if (t_mode == 1)
    {
        uint8_t newest_t2i = current_t2i - 1;
        if (newest_t2i > TMEAN2)
            newest_t2i = TMEAN2 - 1;
        float t_mean = (t_mean_t2 * TMEAN2 * TMEAN1 - t2[current_t2i] * current_t1i + t_sum_t1) / TMEAN1 / TMEAN2;
        float dt = t_mean - ((t_mean_t2 * TMEAN2 * TMEAN1 - t2[current_t2i] * current_t1i - t2[newest_t2i] * (TMEAN1 - current_t1i)) / TMEAN1 / (TMEAN2 - 1));
        y_cur[current_i] = cal0.getWeight(adc[0], t_mean, dt);
#ifdef BayEOSLOGGER_H
        values[4] += y_cur[current_i];
#else
        client.addChannelValue(y_cur[current_i]);
#ifdef BayRF24_h
#if WITH_CHECKSUM
        client.addChecksum();
#endif
        client.sendOrBuffer();
        client.startDataFrame(BayEOS_Float32le,WITH_CHECKSUM); // Start the data frame with float values
#endif

#endif
        if (calc_mode >= 1)
            current_mean = stats.average(y_cur, MITTELUNG);

        if (calc_mode == 3 && abs(current_mean - y_cur[current_i]) < MAX_DEVIATION / 5)
        {
            calc_mode = 2; // Back in the limit
            current = current_mean;
        }

        if (calc_mode == 2 && abs(current_mean - y_cur[current_i]) > MAX_DEVIATION)
            calc_mode = 3; // To much deviation!

        current_i++;
        if (current_i == MITTELUNG)
        {
            if (calc_mode == 0)
            {
                current = stats.average(y_cur, MITTELUNG);
                calc_mode = 1;
            }
            current_i = 0;
        }
#if WITH_RAINSUM
        if (calc_mode == 1 && current_i == 1)
            calc_mode = 2; // mean of MITTELUNG values available + next value
        if (calc_mode == 2)
        {
            float diff = current_mean - current;
#ifdef BayEOSLOGGER_H
            values[5] += current;
            values[6] += diff * 3600 / SAMPLING_INT;
#else
            client.addChannelValue(current, 7);
            client.addChannelValue(diff * 3600 / SAMPLING_INT);
#ifdef BayRF24_h
#if WITH_CHECKSUM
            client.addChecksum();
#endif
            client.sendOrBuffer();
            client.startDataFrame(BayEOS_Float32le,WITH_CHECKSUM); // Start the data frame with float values
#endif
#endif
            float max_evap = -1 * MAX_EVAP * exp(17.62 * temp0 / (243.12 + temp0)) / exp(17.62 * 20 / (243.12 + 20)) * SAMPLING_INT / 3600;
            if (diff < max_evap)
                diff = max_evap; //
            current += diff;     // Mittelungswert anpassen
            if (diff > 0 && diff < (MIN_SLOPE * SAMPLING_INT / 3600))
                diff = 0;
            if (diff > 0)
                rainsum += diff;
            if (diff < 0)
                diff = 0;
#ifdef BayEOSLOGGER_H
            values[7] += diff * 3600 / SAMPLING_INT;
            values[8] += rainsum;
#else
            client.addChannelValue(diff * 3600 / SAMPLING_INT, 9);
            client.addChannelValue(rainsum);
#endif
        }
#endif
    }
#ifdef BayEOSLOGGER_H
    for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++)
    {
        client.addChannelValue(values[i] / counted);
    }
#else
#ifdef BayRF24_h
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    client.sendOrBuffer();
#endif

#endif
}