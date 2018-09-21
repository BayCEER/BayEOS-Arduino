/*
 * Low Power ADC-Board
 * is a variant of BayEOS Low Power Board
 * 
 * It has 12 screw clamps
 * 1,7:       GRD
 * 2,8:       VCC
 * 3-5,9-11:  A0-A2,A3-A5
 * 6,12;      MOSFET Power switch via D7
 * 
 * For EC05 soil moisture sensors solder a 2.5V LDO voltageregulator on the board.
 * Exitation time must be lower then 10ms
 * 
 * VWC = 0.00119 * mV - 0.400
 * 
 *  
 */

void setup(void){
  Serial.begin(9600);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(7,OUTPUT);
  analogReference(INTERNAL);
}

void loop(void){
  digitalWrite(LED_BUILTIN,HIGH);
  digitalWrite(7,HIGH);
  delay(5); //Add a delay to allow sensors to power up
  for(uint8_t i=0;i<6;i++){
    Serial.print(0.00119*1100.0/1023*analogRead(A0+i)-0.400);
    Serial.print("\t");
  }
  Serial.println();
  digitalWrite(LED_BUILTIN,LOW);
  digitalWrite(7,LOW);
  delay(2000);
  
}

