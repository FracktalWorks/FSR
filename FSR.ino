// Test 3 FSRs in parallel and getting their input in analog in

#define VERSION 1
#define LED 13
#define FSR A0
#define AVERAGE 8

int samples[AVERAGE];
int avgIndex = 0;
int i = 0, value = 0, avgValue = 0, threshold = 0, total = 0;

void initValues() {
  for(i = 0; i < AVERAGE; i++) {
    samples[i] = 0;
  }
}

void setup() {
  Serial.begin(9600);
  
  initValues();
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  pinMode(FSR, INPUT);
  
  // Calculating threshold
  total = 0;
  for(i = 0; i < AVERAGE; i++) {
    value = analogRead(FSR);
    samples[i] = value;
    total += samples[i];
  }
  threshold = (total / AVERAGE)*0.95;
}

void loop() {
  // Read analog and store samples
  value = analogRead(FSR);
  Serial.print(value);Serial.print(" ");Serial.println(threshold);
  samples[avgIndex++] = value;
  if(avgIndex >= AVERAGE) avgIndex = 0;
  
  // Averaging over samples
  total = 0;
  for(i = 0; i < AVERAGE; i++)
    total += samples[i];
  avgValue = total / AVERAGE;
  
  // Update LED
  bool state = avgValue < threshold;
  digitalWrite(LED, state ? HIGH : LOW);
}
