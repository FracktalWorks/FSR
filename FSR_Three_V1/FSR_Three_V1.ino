#define VERSION     1

// Define the pins that have LEDs on them. We have one LED for each of the three FSRs to indicate
// when each FSR is triggered. And one power/end stop LED that is on until any of the FSRs are
// triggered.
#define LED1        9
#define LED2        10
#define LED3        11

#define LEDTRIGGER  13
#define ENDSTOP     12

// Define the pins used for the analog inputs that have the FSRs attached. These have external
// 10K pull-up resistors.
#define FSR1        A0
#define FSR2        A1
#define FSR3        A2

// The end stop output
#define TRIGGER     03
#define TRIGGERED   LOW
#define UNTRIGGERED HIGH

//  SEN1    SEN2    Threshold
//  ----    ----    ---------
//   0       0          0.80
//   0       1          0.85
//   1       0          0.95
//   1       1          0.92
const float thresholds[] = { 0.80, 0.85, 0.95, 0.92 };

short fsrLeds[] = { LED1, LED2, LED3 };     // Pins for each of the LEDs next to the FSR inputs
short fsrPins[] = { FSR1, FSR2, FSR3 };     // Pins for each of the FSR analog inputs

#define SHORT_SIZE 8
#define LONG_SIZE 16
#define LONG_INTERVAL (2000 / LONG_SIZE)

unsigned long lastLongTime[3];              // Last time in millis that we captured a long-term sample
uint16_t longSamples[3][LONG_SIZE];         // Used to keep a long-term average
uint8_t longIndex[3] = {0, 0, 0};           // Index of the last long-term sample
uint16_t longAverage[3] = {0, 0, 0};

uint16_t shortSamples[3][SHORT_SIZE];       // Used to create an average of the most recent samples
uint8_t averageIndex[3] = {0, 0, 0};

//
// Set the triggered state based on the state of one FSR
//
void SetOutput(short fsr, bool state)
{
    static bool triggered[3] = {false};     // Keeps track of current FSR trigger state, initiall not triggered

    // Turns on the FSR LED when that FSR is triggered
    triggered[fsr] = state;
    digitalWrite(fsrLeds[fsr], state ? HIGH : LOW);

    // See if any of the FSRs are currently triggered
    bool any = false;
    for (uint8_t fsr = 0; fsr < 3; fsr++)
    {
        any |= triggered[fsr];
    }

    digitalWrite(LEDTRIGGER, any ? HIGH : LOW);
    digitalWrite(ENDSTOP, any ? LOW : HIGH);

}

void InitValues()
{
    for (uint8_t fsr = 0; fsr < 3; fsr++)
    {
        for (uint8_t i = 0; i < SHORT_SIZE; i++)
            shortSamples[fsr][i] = 0;

        for (uint8_t i = 0; i < LONG_SIZE; i++)
            longSamples[fsr][i] = 0;
    }

    for (uint8_t fsr = 0; fsr < 3; fsr++)
        lastLongTime[fsr] = millis();
}

//
// Briefly turns on FSR LEDs during startup to indicate the version number of the
// firmware.
//
void BlinkVersion(uint8_t version)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        digitalWrite(fsrLeds[i], (version & (1 << i)) ? HIGH : LOW);
    }
    delay(250);
    for (uint8_t i = 0; i < 3; i++)
    {
        digitalWrite(fsrLeds[i], LOW);
    }
}

//
// One-time setup for the various I/O ports
//
void setup()
{
    InitValues();

    for (uint8_t fsr = 0; fsr < 3; fsr++)
    {
        // Set the FSR LEDs for output and turn them off
        uint8_t pin = fsrLeds[fsr];
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);

        // Set the FSR lines for inputs without a pull-up since the board has external 10K
        // pull-up resistors.
        pin = fsrPins[fsr];
        pinMode(pin, INPUT);
    }

    // Set the green combined LED to on so it acts as a power-on indicator. We'll turn it
    // off whenever we trigger the end stop.
    pinMode(LEDTRIGGER, OUTPUT);
    digitalWrite(LEDTRIGGER, HIGH);

    // Set the endstop pin to be an output that is set for NC
    pinMode(ENDSTOP, OUTPUT);

    BlinkVersion(VERSION);
};

//
// Captures a new value once LONG_INTERVAL ms have passed since the last sample.
//
// Returns: The current long-range average
uint16_t UpdateLongSamples(short fsr, int avg)
{
    //
    // If enough time hasn't passed, just return the last value
    //
    unsigned long current = millis();
    if (current - lastLongTime[fsr] <= LONG_INTERVAL)
    {
        return longAverage[fsr];
    }

    //
    // Update the log sample with the new value, and then update the long average
    //
    longSamples[fsr][longIndex[fsr]++] = avg;
    if (longIndex[fsr] >= LONG_SIZE)
    {
        longIndex[fsr] = 0;
    }

    uint16_t total = 0;
    for (int i = 0; i < LONG_SIZE; i++)
    {
        total += longSamples[fsr][i];
    }
    
    longAverage[fsr] = total / LONG_SIZE;

    lastLongTime[fsr] = millis();
    return longAverage[fsr];
}

//
// Returns the current threshold to use, baed on jumpers installed
//
inline float GetThreshold()
{
    return 0.85;
}

//
// This method is called after every sample to see if the output trigger status should be changed.
// It will also the short sample buffer, and it may update the long-term samples.
//
void CheckIfTriggered(short fsr)
{
    //
    // Calculate the average of the most recent short-term samples
    //
    uint16_t total = 0;
    for (int i = 0; i < SHORT_SIZE; i++)
    {
        total += shortSamples[fsr][i];
    }
    uint16_t avg = total / SHORT_SIZE;
    uint16_t longAverage = UpdateLongSamples(fsr, avg);
    uint16_t threshold = GetThreshold() * longAverage;

    bool triggered = avg < threshold;
    SetOutput(fsr, triggered);
}

void loop()
{
    for (uint8_t fsr = 0; fsr < 3; fsr++)
    {
        int value = analogRead(fsrPins[fsr]);

        shortSamples[fsr][averageIndex[fsr]++] = value;
        if (averageIndex[fsr] >= SHORT_SIZE)
        {
            averageIndex[fsr] = 0;
        }
        CheckIfTriggered(fsr);
    }
};

