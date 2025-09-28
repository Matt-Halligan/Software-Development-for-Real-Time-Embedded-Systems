// // Analog phototransistor detection test (recommended)
// const int irLedPin = 7;
// const int sensorA0 = A0;
// const int statusLed = LED_BUILTIN;

// const unsigned long SAMPLE_MS = 2;      // how long each measurement cycle lasts
// const int SAMPLE_COUNT = 40;              // how many analog samples per condition
// const int MIN_DIFF_ADC = 30;              // minimum ADC difference to declare detection (tweak)

// void setup() {
//   pinMode(irLedPin, OUTPUT);
//   pinMode(statusLed, OUTPUT);
//   digitalWrite(irLedPin, LOW);
//   Serial.begin(115200);
//   while (!Serial) { ; }
//   Serial.println("Analog phototransistor detection test");
//   delay(200);
// }

// // returns average analog reading (0-1023)
// int sampleAnalogAvg() {
//   long sum = 0;
//   for (int i = 0; i < SAMPLE_COUNT; ++i) {
//     sum += analogRead(sensorA0);
//     delay(SAMPLE_MS / SAMPLE_COUNT);
//   }
//   return (int)(sum / SAMPLE_COUNT);
// }

// void loop() {
//   // 1) ambient (IR OFF)
//   digitalWrite(irLedPin, LOW);
//   delay(50);
//   int ambient = sampleAnalogAvg();

//   // 2) IR ON
//   digitalWrite(irLedPin, HIGH);
//   delay(50);
//   int withIR = sampleAnalogAvg();

//   // 3) compute and decide
//   int diff = withIR - ambient; // positive if IR increases voltage at A0; may be negative depending on wiring
//   int absdiff = abs(diff);

//   Serial.print("Ambient: ");
//   Serial.print(ambient);
//   Serial.print(" | With IR: ");
//   Serial.print(withIR);
//   Serial.print(" | Diff: ");
//   Serial.println(diff);

//   bool detected = absdiff >= MIN_DIFF_ADC;

//   if (detected) {
//     Serial.println(">> DETECTION: phototransistor IS seeing the IR LED.");
//     // visual feedback: brightness-proportional blink
//     digitalWrite(statusLed, HIGH);
//     delay(120);
//     digitalWrite(statusLed, LOW);
//   } else {
//     Serial.println(">> NO DETECTION: change too small.");
//   }

//   Serial.println("------------------------------");
//   digitalWrite(irLedPin, LOW);
//   delay(800);
// }

////////////////////////////////////////////////////
// 
//  Matthew Halligan
//  Software Development for Real Time Embedded Systems, Fall 2025
//  Module 4/5 Project 3
//  Professor Doug Ferguson
// 
// Read and export IR Beam Breaks and convert to RPM for Electronic Speed Control Tuning
// Implements fast analog IR detection, break and recovery of signal caused by propeller interference 
// using hardware based interrupts and automated RPM calculation using timer based interrupts in a
// "Round Robin with interrupts Design" and serial export
// 
// To use, ensure serial monitor is attached via Arduino IDE 2.3.6 set to 115200 baud.
// Ensure circuitry is configured accorning to figure one in associated report.
// Provide power to esc module through external 11V power source.
// No further action is required by the user once power is provided to ESC and power and serial 
// are supplied to Arduino Uno in correct sequence. 
//
////////////////////////////////////////////////////
// Fast beam-break detection using ADC free-running + ISR
// For UNO- A0 sensor
// 60-second RPM collection window 
#include <Servo.h>
Servo esc;
const int escPin = 9;
const int irLedPin = 7;
const int sensorChannel = 0; // A0
const int statusLed = LED_BUILTIN;
int pw = 1100;

// detection tuning
const int MIN_DIFF_ADC = 60;     // expected drop in IR when broken
const int HYSTERESIS_ADC = 12;   // hysteresis band 
const uint8_t CONSECUTIVE_REQUIRED = 3; // require N consecutive samples beyond threshold to accept edge

// runtime state (shared with ISR)
volatile bool beamPresent = true;      // current state (true = beam present)
volatile bool fallingEdgeFlag = false; // set by ADC ISR when beam broken detected
volatile bool risingEdgeFlag = false;  // set by ADC ISR when beam restored detected
volatile unsigned long edgeCount = 0;  // total number of detected breaks during current window

// internal ISR counters (kept small)
volatile uint8_t consecBelow = 0;
volatile uint8_t consecAbove = 0;

// Timer2 millisecond counter/state (for 60s window)
volatile unsigned long msCounter = 0UL;     // ms accumulator (0..14999)
volatile bool sampleReady = false;          // set true when 15s elapsed; main loop consumes it
const unsigned long MS_PER_WINDOW = 15000UL; // 15 seconds = 15000 ms

unsigned long processedWindows = 0;

// thresholds (computed at startup)
int baselineWithIR = 0;
int threshLow = 0;
int threshHigh = 0;

// main bookkeeping
unsigned long lastEdgeMicros = 0;
const unsigned int countsPerRevolution = 1; // set to pulses per one shaft revolution

// --- Configure Timer2 for ~1ms CTC ticks (OCR2A=249, prescaler=64) ---
void setupTimer2_1ms() {
  cli();                // disable global interrupts while configuring
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  // CTC mode: WGM21 = 1
  TCCR2A |= (1 << WGM21);
  // OCR2A for 1ms with prescaler 64: ticks_per_ms = 16000000/64/1000 = 250 -> OCR2A = 249
  OCR2A = 249;
  // Set prescaler to 64: CS22 = 1 (CS21=0, CS20=0)
  TCCR2B |= (1 << CS22);
  TCCR2B &= ~((1 << CS21) | (1 << CS20));
  // Enable Timer2 compare A interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();                // enable interrupts
}

// Timer2 Compare Match A ISR at approx. once per 1 ms
ISR(TIMER2_COMPA_vect) {
  msCounter++;
  if (msCounter >= MS_PER_WINDOW) {
    msCounter = 0;
    sampleReady = true; // signal main loop that the 60s window finished
  }
}

// ADC free-running setup
void setupAnalogInterrupt(){
  // Configure ADC for free-running conversions on A0 with interrupt on completion
  ADMUX = (1 << REFS0) | (sensorChannel & 0x07); // AVcc reference, channel A0

  // ADCSRA: ADEN | ADSC | ADATE | ADIE | ADPS2..0 (prescaler 128)
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) |
           (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

  ADCSRB = 0; // free running
  // Global interrupts enabled by caller (we'll ensure in setup)
}

// quick average using analogRead() for calibration only
int quickAnalogAvg(int samples) {
  long s = 0;
  for (int i = 0; i < samples; ++i) {
    s += analogRead(A0);
    delay(1);
  }
  return (int)(s / samples);
}

void computeThresholds() {
  threshLow = baselineWithIR - MIN_DIFF_ADC;
  if (threshLow < 0) threshLow = 0;
  threshHigh = threshLow + HYSTERESIS_ADC;
  if (threshHigh > 1023) threshHigh = 1023;
}

// ADC ISR: runs at free-running ADC rate (fast). Keep short.
ISR(ADC_vect) {
  uint16_t v = ADC; // 10-bit result

  // Beam gives HIGH reading normally, and breaks drop the ADC reading.
  if (v < threshLow) {
    consecBelow++;
    consecAbove = 0;
    if (consecBelow >= CONSECUTIVE_REQUIRED && beamPresent) {
      beamPresent = false;
      fallingEdgeFlag = true;
      consecBelow = 0;
    }
  } else if (v > threshHigh) {
    consecAbove++;
    consecBelow = 0;
    if (consecAbove >= CONSECUTIVE_REQUIRED && !beamPresent) {
      beamPresent = true;
      risingEdgeFlag = true;
      consecAbove = 0;
    }
  } else {
    // within hysteresis band -> reset counters
    consecBelow = 0;
    consecAbove = 0;
  }
}

void calcAndPrintRPM() {
  // compute RPM using the 60-second window
  // RPM = (edgeCount pulses / countsPerRevolution) * (60 / windowSeconds)
  // Since windowSeconds = 60, RPM = edgeCount / countsPerRevolution
  unsigned long counts = 0;
  counts = edgeCount;
  edgeCount = 0; // reset for next window
  unsigned long rpm = counts*60/15;
  Serial.print(processedWindows + 1);
  Serial.print(',');
  Serial.print(counts);
  Serial.print(',');
  Serial.print(rpm);
  Serial.println();

  processedWindows++;
  if (processedWindows % 4 == 0){
    pw = pw + 100;
  }  
}

void setup() {
  pinMode(irLedPin, OUTPUT);
  pinMode(statusLed, OUTPUT);
  digitalWrite(irLedPin, HIGH);
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("Fast ADC free-run beam-break detection, 15s collection window size");

  // Turn IR ON and measure baseline (simple calibration)
  digitalWrite(irLedPin, HIGH);
  delay(200);
  baselineWithIR = quickAnalogAvg(40);
  computeThresholds();
  Serial.print("Baseline: ");
  Serial.print(baselineWithIR);
  Serial.print("  threshLow=");
  Serial.print(threshLow);
  Serial.print("  threshHigh=");
  Serial.println(threshHigh);

  // Setup ADC free-running + ISR
  setupAnalogInterrupt();

  // Attach and arm ESC
  esc.attach(escPin);
  Serial.println("Arming ESC...");
  esc.writeMicroseconds(1000); // min throttle
  delay(2000);
  // optional calibration pulse — comment out if not desired
  // esc.writeMicroseconds(2000); delay(2000);
  esc.writeMicroseconds(1000);
  delay(2000);
  Serial.println("ESC armed. Ready.");
  Serial.print("Collection Window ID, Beam Break Counts, Approximate RPM");
  Serial.println();
  // Configure Timer2 for ms ticks
  setupTimer2_1ms();
}

void loop() {
  // Keep motor from overspinning during testing
  if (pw <= 1300) {
    esc.writeMicroseconds(pw); 
  }

  // Process edge events flagged by ADC ISR
  if (fallingEdgeFlag) {
    fallingEdgeFlag = false;
    unsigned long now = micros();
    unsigned long dt = (lastEdgeMicros == 0) ? 0 : (now - lastEdgeMicros);
    lastEdgeMicros = now;

    edgeCount++;
    digitalWrite(statusLed, HIGH);

    // Uncomment for debugging of IR beam break detection
    // Serial.print("[BROKEN] t=");
    // Serial.print(now);
    // Serial.print(" us  dt=");
    // Serial.print(dt);
    // Serial.print(" us  total=");
    // Serial.println(edgeCount);
  }

  if (risingEdgeFlag) {
    risingEdgeFlag = false;
    unsigned long now = micros();
    digitalWrite(statusLed, LOW);
    // Uncomment for beam detection restoration debugging
    // Serial.print("[RESTORED] t=");
    // Serial.print(now);
    // Serial.println(" us");
  }

  // When 60s window completes, sampleReady is true: compute and print RPM
  if (sampleReady) {
    sampleReady = false;
    calcAndPrintRPM();
  }

  // occasional status print for IR detection baseline and thresholds
  static unsigned long lastPrint = 0;
  // if (millis() - lastPrint > 30000) {
  //   Serial.print("Baseline=");
  //   Serial.print(baselineWithIR);
  //   Serial.print("  threshLow=");
  //   Serial.print(threshLow);
  //   Serial.print("  threshHigh=");
  //   Serial.println(threshHigh);
  //   lastPrint = millis();
  // }

  delay(2);
}
