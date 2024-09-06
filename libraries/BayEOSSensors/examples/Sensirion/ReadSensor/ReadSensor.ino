#include <Sensirion.h>
#define LED 5

long mtime;

// Sensirion(dataPin, sclkPin, address, noholdmaster);
// SHT1x address = 0x00  SHT2x address = 0x40  SHT3x address = 0x44 or 0x45
Sensirion sht1 = Sensirion(2, 3, 0x40);

float temperature;
float humidity;
float dewpoint;
float humidity37oC;

int ret, mret = 1234;

void setup()
{
  // put your setup code here, to run once:

  pinMode(LED, OUTPUT); // initialize digital pin LED as an output
  Serial.begin(9600);
  delay(1000);
  Serial.println("started");
}

void loop()
{
  // put your main code here, to run repeatedly:

  digitalWrite(LED, (((millis() >> 6) & 0x5) == 0)); // double flash led rapidly

  // all parameters are optionals if user don't need it
  logError(ret = sht1.measure(&temperature, &humidity, &dewpoint, 37, &humidity37oC)); // Measurement

  if (ret != mret) // only for debug purpose
  {
    Serial.print("(");
    Serial.print(millis() - mtime);
    mtime = millis();
    Serial.print("mS)  ");
    if (ret == 0)
    {
      Serial.println();
      Serial.println();
    }

    Serial.print("[");
    Serial.print(ret);
    Serial.print("] ");
  }

  if (ret == S_Meas_Rdy) // A new measurement is available
  {
    Serial.println();
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.print(" oC, Humidity = ");
    Serial.print(humidity);
    Serial.print(" %, Dewpoint = ");
    Serial.print(dewpoint);
    Serial.print(" oC, Humidity @37oC = ");
    Serial.print(humidity37oC);
    Serial.print(" % ");
  }

  mret = ret;
}

// The following code is only used with error checking enabled
void logError(int error)
{
  if (error >= 0) // No error
    return;

  Serial.println();
  Serial.print("Error: ");
  Serial.print(error);

  switch (error)
  {
  case S_Err_Param:
    Serial.print(" - Parameter error in function call! ");
    break;
  case S_Err_NoACK:
    Serial.print(" - No response (ACK) from sensor! ");
    break;
  case S_Err_CRC:
    Serial.print(" - CRC mismatch! ");
    break;
  case S_Err_TO:
    Serial.print(" - Measurement timeout! ");
    break;
  default:
    Serial.print(" - Unknown error received! ");
    break;
  }
}
