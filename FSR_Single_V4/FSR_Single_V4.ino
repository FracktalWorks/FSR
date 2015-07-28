#define VERSION 2

#define LED 9
#define SIG 12
#define FSR A0
#define THRESHOLD 0.93

#define SHORT_SIZE 8
#define LONG_SIZE 16
#define LONG_INTERVAL (2000/LONG_SIZE)

unsigned long lastTime = 0;

int longIndex = 0, shortIndex = 0, longAvg = 0, shortAvg = 0;
int shortSamples[SHORT_SIZE], longSamples[LONG_SIZE];
int i = 0, total = 0;

void setup() {
  Serial.begin(9600);
  
  for(i = 0; i < SHORT_SIZE; i++) shortSamples[i] = 0;
  for(i = 0; i < LONG_SIZE; i++) longSamples[i] = 0;
    
  lastTime = millis();
  
  pinMode(LED, OUTPUT);
  pinMode(SIG, OUTPUT);
  digitalWrite(LED, LOW);
  digitalWrite(SIG, HIGH);
  pinMode(FSR, INPUT);
}

void loop() {
  shortSamples[shortIndex++] = analogRead(FSR);
  if(shortIndex >= SHORT_SIZE) shortIndex = 0;
  
  total = 0;
  for(i = 0; i < SHORT_SIZE; i++) total += shortSamples[i];
  shortAvg = total / SHORT_SIZE;
  
  if(millis() - lastTime > LONG_INTERVAL) {
    longSamples[longIndex++] = shortAvg;
    if(longIndex >= LONG_SIZE) longIndex = 0;
    
    total = 0;
    for(i = 0; i < LONG_SIZE; i++) total += longSamples[i];
    longAvg = total / LONG_SIZE;
    
    lastTime = millis();
  }
  
  if(shortAvg < longAvg*THRESHOLD) {
    digitalWrite(LED, HIGH);
    digitalWrite(SIG, LOW);
  }
  else {
    digitalWrite(LED, LOW);
    digitalWrite(SIG, HIGH);
  }
  
  Serial.println(shortAvg);
  
}
