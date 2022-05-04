//
// Copyright 2022
//
// This file is intended for the single-tube test casette design
//
// The intended behavior of the code is as follows:
// As the test casette is inserted into the device, the software detects
// a downward trend in Arduino voltage values, then detects an upward trend,
// and finally uploads the average value of 100 measurements, if and only if
// the sample deviation of those 100 measurements is below 2. This way, the
// O.C.E.A.N. device only uploads when a sample is being measured.
//
// Input: inserting the test casette (sample tray) into the device
// Output: comma-separated output as follows:
// Arduino voltage average, standard deviation, number of measurement points per single reading
//
// To obtain the Arduino voltage value, divide the measured digital value by 1023,
// and then multiply the voltage input to the device by this ratio.
//
// Example: Arduino output is 500 when the voltage input is 5 V.
// The voltage potential measured at the sensor pin
// is (500 / 1023) * 5 Volts = 2.44 Volts
//

// Initializing variables
int sensorPin = A0;
int sensorValue = 0;
double largeAverage = 0;
double miniAverage = 0;
double lastMiniAverage = 0;
double lastMiniSD = 0;
int counter = 0;
double sumStd = 0;
double miniSD = 0;
double delayVal = 1000;
double totalTime = 2;

const int ELEMENT_COUNT_MAX = 100;
const int MINI_AVERAGE_COUNT = 10;
double values[ELEMENT_COUNT_MAX];
double miniValues[MINI_AVERAGE_COUNT];

const int NUMBER_OF_EXTREMES = 4;
double extremes[NUMBER_OF_EXTREMES];
double times[NUMBER_OF_EXTREMES];

// percentage threshold which allows the board
// to detect an upward or downward increase in values
const double monitoringThreshold = 0.03;
double thresholdValue = 0;
int maxValue = 0;
int minValue = 1000;
int printStatus = 0;
double lastAverage = 0;
double lastSD = 0;
int lastMax = 0;
// If the collected N measurements have a standard deviation
// below the autoUploadThreshold value, the data will be
// automatically uploaded to the Serial monitor.
const double autoUploadThreshold = 2.0;

bool resetTracker = true;
int currentIndex = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (int i = 0; i < ELEMENT_COUNT_MAX; i++) {
   values[i] = 0;
  }
  for (int i = 0; i < MINI_AVERAGE_COUNT; i++) {
   miniValues[i] = 0;
  }
  delayVal = delayVal / (ELEMENT_COUNT_MAX/totalTime);
}

void loop() {
  sensorValue = analogRead(sensorPin);
  delay(delayVal);
  counter = counter + 1;

  // CSV output, uncomment for debugging purposes
  /*
  // CSV output
  Serial.print(sensorValue);
  Serial.print(",");
  */

  miniAverage = miniAverage + sensorValue;
  miniValues[counter % MINI_AVERAGE_COUNT] = sensorValue;

  if ((counter % MINI_AVERAGE_COUNT == 0) && (counter != 0)) {
      // calculate the average
      miniAverage = miniAverage / MINI_AVERAGE_COUNT;

      // calculate the standard deviation
      for (int i = 0; i < MINI_AVERAGE_COUNT; i++) {
        miniSD = miniSD + (miniValues[i] - miniAverage)*(miniValues[i] - miniAverage);
      }
      miniSD = miniSD / (MINI_AVERAGE_COUNT - 1);
      miniSD = sqrt(miniSD);

      /*
      // CSV output
      Serial.print(miniAverage);
      Serial.print(",");
      Serial.print(miniSD);
      Serial.print(",");
      */

      // reset the average
      lastMiniAverage = miniAverage;
      lastMiniSD = miniSD;
      miniAverage = 0;
      miniSD = 0;
  }
  /*
  else {
      Serial.print(",,");
  }
  */
  
  largeAverage = largeAverage + sensorValue;
  values[counter - 1] = sensorValue;
  if (counter == ELEMENT_COUNT_MAX) {
    largeAverage = largeAverage / counter;
    counter = 0;
    for (int i = 0; i < ELEMENT_COUNT_MAX; i++) {
      sumStd = sumStd + (values[i] - largeAverage)*(values[i] - largeAverage);
    }
    // calculate the standard deviation
    sumStd = sumStd / (ELEMENT_COUNT_MAX - 1);
    sumStd = sqrt(sumStd);

    /*
    // CSV output
    Serial.print(largeAverage);
    Serial.print(",");
    Serial.print(sumStd);
    */
    
    lastAverage = largeAverage;
    lastSD = sumStd;
    largeAverage = 0;
    sumStd = 0;
  }
  else {
      /*
      // CSV output
      Serial.print(",");
      */
  }

  /*
  // CSV output
  Serial.println();
  */

  switch (printStatus) {
      case 0:
          if (sensorValue > maxValue) {
              maxValue = sensorValue;
          }
          thresholdValue = (maxValue - 0) * monitoringThreshold;
          if (sensorValue < thresholdValue) {
              lastMax = maxValue;
              printStatus = 1;
          }
          break;
      case 1:
          // the sensorValue crossed the threshold, going from high to low
          if (sensorValue > thresholdValue) {
              printStatus = 2;
          }
          break;
      case 2:
          if (lastMiniSD <= autoUploadThreshold) {
              counter = 0;
              for (int i = 0; i < ELEMENT_COUNT_MAX; i++) {
               values[i] = 0;
              }
              for (int i = 0; i < MINI_AVERAGE_COUNT; i++) {
               miniValues[i] = 0;
              }
              printStatus = 3;
          }
          break;
      case 3:
          // The sensorValue crossed the threshold, going from low to high
          if ((lastAverage > thresholdValue) && (lastSD <= autoUploadThreshold)) {
            if (lastAverage <= ((1.0 - monitoringThreshold) * lastMax)) {
                  printToSerial();

                  printStatus = 0;
                  lastAverage = 0;
                  lastMiniAverage = 0;
                  largeAverage = 0;
                  sumStd = 0;
                  lastSD = 100;
                  miniSD = 100;
                  miniAverage = 0;
                  counter = 0;
                  for (int i = 0; i < ELEMENT_COUNT_MAX; i++) {
                   values[i] = 0;
                  }
                  for (int i = 0; i < MINI_AVERAGE_COUNT; i++) {
                   miniValues[i] = 0;
                  }
            }
          }
          break;
      default:
        // do nothing
        break;
  }
}

void printToSerial() {
    // Excel accepts comma separated values
   Serial.print(lastAverage);
   Serial.print(",");
   Serial.print(lastSD);
   Serial.print(",");
   Serial.println(ELEMENT_COUNT_MAX);
}
