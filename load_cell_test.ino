/*
file: load_cell_test.ino

The MIT License (MIT)

Copyright (c) 2013 Mark Heywood

Load Cell Calibration & Processing output
written by Mark Heywood
Website: http://airtripper.com/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
_________________________________
ABOUT

This software takes an analog reading from a load cell circuit attached to the arduino.
Each new analog reading is put into a buffer of many readings where the newest
analog reading replaces the oldest analog reading. All the readings in the buffer
are added together and then divided by the number of readings held in the buffer to
yield an average analog value. The buffer is used as a method of smoothing the analog
readings taken from the arduino analog pin.

The software has two modes of operation where one operation is used for calibration and
testing while the other is used for outputting to a graphing application such as
Processing.


*/

// Set the software mode and set the analog pin
int calibrate = 1; // 0 = Output to Processing Application, 1 = Calibration mode
int analogPin = 0;  // Arduino analog pin to read

// LOAD CELL CALIBRATION
// Low end of the test load values
static long loadLow = 0; // measured load in grammes on good scales
static int analogLow = 80; // analog reading from load cell for low end test load

// High end of the test load values
static long loadHigh = 5103; // measured load in grammes on good scales
static int analogHigh = 1008; // analog reading from load cell for high end test load

// This is used when you change the load cell platform to something else that weighs
// different and the load is no longer on zero. Add an offset to set to zero.
int loadAdjustment = 164;  // Adjust non loaded load cell to 0




// Values for the analog sample buffer for running average to smooth analogue reads
static int samplesBuffer = 100; // Should be the same as the number of samples
int analogSamples[100] = {0}; // Number of analog samples to average and set to 0
int lastSampleIndex = 0; // Last analog sample
long sampleBufferTotal = 0; // The sum of all the samples held in the buffer
long analogSamplesAverage = 0; // Running average of the sum of the buffer

// Results plot or display frequency
long time = 0;  // Set time to mark start of delay
int plotDelay = 10; // Time gap between plots for display or graph

void setup() {
  if (calibrate) {
    Serial.begin(9600);  // Set a slower boadrate for calibration
    plotDelay = 1000;  // Slow the readings output to display for calibration
  } else {
    Serial.begin(115200);  // Set a faster baudrate for Processing Application
  }
}

void loop() {
  int analogValue = analogRead(analogPin);  // Get analog read
  
  // Add new analog reading to buffer and return oldest reading
  int oldestSample = addSample(analogValue);
  // Get running average, pass the latest and oldest analog sample reading
  analogSamplesAverage = runningAverage(analogValue, oldestSample);
  
  if(millis() > time + plotDelay){
    // Convert analog value to load in grams
    int loadGrams = map(analogSamplesAverage, analogLow, analogHigh, loadLow, loadHigh);
    loadGrams = loadGrams - loadAdjustment;  // Shift the load to measure from 0 load
    if (calibrate) {// print test results
      Serial.print("analog value: ");Serial.println(analogValue);
      Serial.print("average value: ");Serial.println(analogSamplesAverage);
      Serial.print("load in grams: ");Serial.println(loadGrams);
      Serial.println(" ");
    } else {  // Output to Processing as such
      Serial.println(loadGrams);
    }
    
    time = millis();  // Set time to mark start of delay
  }
}

// Function - running average
long runningAverage(long newSample, long oldSample) {
  // Add new sample and subtract old sample from the samples buffer total
  // then return sample average
  sampleBufferTotal += newSample;  // Add new analog sample
  sampleBufferTotal -= oldSample;  // Subtract oldest analog sample
  return sampleBufferTotal / samplesBuffer;  // Return average analog sample reading
}

// Function - add and remove analog samples from ring buffer, return oldest sample
// The ring buffer is used to keep track of the oldest sample.
int addSample(int newSample) {
  // Add new analog read sample to the ring buffer and return the oldest analog reading
  int oldSample;
  if (lastSampleIndex == samplesBuffer - 1 ) { // Check if end off buffer reached
    oldSample = analogSamples[0];  // Get old analog sample from start of buffer
    analogSamples[0] = newSample;  // Add new analog sample to start of buffer
    lastSampleIndex = 0;  // Record sample index currently working on
  } else {
    lastSampleIndex ++;  // Move to next index
    oldSample = analogSamples[lastSampleIndex];  // Get old analog sample
    analogSamples[lastSampleIndex] = newSample;  // Add new analog sample
  }
  return oldSample;  // Return old analog sample
}

