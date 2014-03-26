const uint8_t echo_power_pin[6]={3,4,6,7,8,9};


void setup() {
  pinMode(13,OUTPUT);
  for(uint8_t i=0;i<6;i++){
     pinMode(echo_power_pin[i],OUTPUT);
     digitalWrite(echo_power_pin[i],LOW);
  }
}

void loop() {
  for(uint8_t i=0;i<6;i++){
     digitalWrite(13,HIGH);
     digitalWrite(echo_power_pin[i],HIGH);
     delay(2000);
     digitalWrite(13,LOW);
     digitalWrite(echo_power_pin[i],LOW);
     delay(1000);
  }

  
}

