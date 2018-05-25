#define POWER_PIN 7

void setup()
{
  pinMode(POWER_PIN,OUTPUT);
  Serial.begin(9600);  
}


void loop() {
    digitalWrite(POWER_PIN,HIGH);
    delay(15);
    for(uint8_t i=0;i<5;i++){
      Serial.print(analogRead(A0+i));
      Serial.print("\t");
    }
    digitalWrite(POWER_PIN,LOW);
    Serial.println();
    
    delay(1000);
 }



