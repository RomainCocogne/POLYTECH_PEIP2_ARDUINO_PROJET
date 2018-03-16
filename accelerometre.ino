
float vref_x; float vref_y; 
void setup() {
  Serial.begin(9600);
  delay(2000);
  vref_x=analogRead(A0)/204.8;
  vref_y=analogRead(A1)/204.8;

}

void loop() {
  float v_x = analogRead(A0)/204.8;
  float v_y = analogRead(A1)/204.8;
  float g_x = (v_x-vref_x)/0.3;
  float g_y = (v_y-vref_y)/0.3;
  float a = atan2(g_x,g_y);
  Serial.print('$');Serial.print((a));Serial.println(';');
  delay (1000);
}
