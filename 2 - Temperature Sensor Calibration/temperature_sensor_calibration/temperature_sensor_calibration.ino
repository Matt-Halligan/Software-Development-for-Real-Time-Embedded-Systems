////////////////////////////////////////////////////
// 
//  Matthew Halligan
//  Software Development for Real Time Embedded Systems, Fall 2025
//  Module 3/4 Project 2
//  Professor Doug Ferguson
// 
// Read and export Farenheight-Converted Temperature Readings from DHT11 Sensor
// Implements celcius to farenheight conversion using timer based interrupts
// "Round Robin with interrupts Design" and serial export
// 
// To use, ensure serial monitor is attached via Arduino IDE 2.3.6 set to 9600 baud
// No further action is required by the user once power and serial are supplied to Arduino Uno
//
////////////////////////////////////////////////////

#include "DHTesp.h"
DHTesp dht;

const uint8_t DHT_PIN = 3;              // digital pin 3 for DHT data 
const uint8_t SAMPLE_INTERVAL_SECONDS = 10; // sample every 10 seconds
const unsigned long HEARTBEAT_MS = 500; // task code background artificial interval

volatile unsigned long sampleCount = 0;      // number of samples requested by ISR
unsigned long processedSamples = 0;          // number of samples processed in main loop

volatile uint8_t secCounter = 0;             // seconds counter incremented by ISR

// --- Configure Timer1 for 1-second Clear Timer on Compare (CTC) ticks ---
void setupTimer1_1s() {
  cli();                // disable global interrupts while configuring
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  // Clear Timer on Compare mode (WGM12 = 1)
  TCCR1B |= (1 << WGM12);
  // Prescaler 1024: CS12 = 1, CS11 = 0, CS10 = 1
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // OCR1A for 1 second: ticks = F_CPU / prescaler = 16,000,000 / 1024 = 15625 -> OCR1A = 15624
  OCR1A = 15624;
  // Enable Timer1 compare A interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();                // enable interrupts
}

// Timer1 Compare Match A ISR at approx. once per second
ISR(TIMER1_COMPA_vect) {
  ++secCounter;
  if (secCounter >= SAMPLE_INTERVAL_SECONDS) {
    secCounter = 0;
    ++sampleCount;    // signal main loop to take one sample
  }
}

unsigned long lastHeartbeat = 0;
void doTaskCode() {
  unsigned long now = millis();
  if (now - lastHeartbeat >= HEARTBEAT_MS) {
    lastHeartbeat = now;
    // Serial.print("."); 
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  // DHT11 initialization
  dht.setup(DHT_PIN, DHTesp::DHT11);

  // Timer setup
  setupTimer1_1s();

  // CSV header
  Serial.println("timestamp_ms,sample_index,temp_F,humidity_pct");
}

void takeOneSampleAndPrint(unsigned long sampleIndex) {
  float hum = dht.getHumidity();
  float tempC = dht.getTemperature();

  unsigned long ts = millis(); // ms time stamp since start 

  if (isnan(tempC) || isnan(hum)) {
    Serial.print(ts); Serial.print(",");
    Serial.print(sampleIndex); Serial.print(",");
    Serial.print("NaN,NaN,ERROR_READ");
    Serial.println();
    return;
  }

  float tempF = tempC * 1.8f + 32.0f;

  // CSV: timestamp_ms,sample_index,temp_F,humidity_pct,event
  Serial.print(ts); Serial.print(",");
  Serial.print(sampleIndex); Serial.print(",");
  Serial.print(tempF, 2); Serial.print(",");
  Serial.print(hum, 1);
  Serial.println();
}

void loop() {
  // artificial task code that would otherwise be interrupted.
  doTaskCode();

  
  unsigned long pending = sampleCount - processedSamples;
  

  if (pending > 0) {
    ++processedSamples;
    unsigned long sampleIndex = processedSamples;
    noInterrupts();
    takeOneSampleAndPrint(sampleIndex);
    interrupts();
    
  }

  delay(10);
}