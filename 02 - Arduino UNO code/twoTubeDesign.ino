//
// Copyright 2022
//
// publish date: 03 May 2022, Tuesday, at 11:41 AM
// version 1 of code
// to do: add timestamp to output
//
// this code:
// outputs peak values for the two-tube design
//

int sensorPin = A0;
int sensorValue = 0;
double valuesAverage = 0;
int counter = 0;
double delayVal = 1000;
// total time for testing
double totalTime = 0.1;

const int ELEMENT_COUNT_MAX = 100;
// the sampling time is totalTime / ELEMENT_COUNT_MAX
// for example, this code takes measurements every 0.1/100 = 0.001 seconds
double values[ELEMENT_COUNT_MAX];

const int NUMBER_OF_EXTREMES = 4;
double extremes[NUMBER_OF_EXTREMES];
double times[NUMBER_OF_EXTREMES];

// this double sets the threshold between
const double monitoringThreshold = 0.03;
double thresholdValue = 0;
int maxValue = 0;
int numberOfPeak = 0;
double lastAverage = 0;
bool resetTracker = true;
int currentIndex = 0;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < ELEMENT_COUNT_MAX; i++) {
   values[i] = 0;
  }
  delayVal = delayVal / (ELEMENT_COUNT_MAX/totalTime);
}

void loop() {
  sensorValue = analogRead(sensorPin);
  delay(delayVal);
  counter = counter + 1;
  // debugging purposes: prints out the sensor value
  // Serial.println(sensorValue);

  valuesAverage = valuesAverage + sensorValue;
  values[counter - 1] = sensorValue;

  // once the values array is full,
  // this calculates the average of its values
  if (counter == ELEMENT_COUNT_MAX) {
    valuesAverage = valuesAverage / counter;
    lastAverage = valuesAverage;
    valuesAverage = 0;
    counter = 0;
  }

// switch statement that coordinates the logic for saving peak values
  switch (numberOfPeak) {
      case 0:
          if (sensorValue > maxValue) {
              maxValue = sensorValue;
          }
          thresholdValue = (maxValue - 0) * monitoringThreshold;
          if (sensorValue < thresholdValue) {
              if ((lastAverage - sensorValue) > 0) {
                  numberOfPeak = 1;
              }
          }
          break;
      case 1:
          // Serial.println("You crossed the threshold, going from high to low");
          if (resetTracker) {
              maxValue = 0;
              resetTracker = false;
          }
          if (sensorValue > thresholdValue) {
              if ((lastAverage - sensorValue) < 0) {
                  numberOfPeak = 2;
              }
          }
          break;
      case 2:
          // Serial.println("You crossed the threshold, going from low to high");
          if (sensorValue > maxValue) {
              maxValue = sensorValue;
          }
          if (sensorValue < thresholdValue) {
              if ((lastAverage - sensorValue) > 0) {
                  if (currentIndex == 4) {
                      currentIndex = 0;
                      resetTracker = true;
                      numberOfPeak = 0;
                  } else {
                      extremes[currentIndex] = maxValue;
                      currentIndex++;
                      maxValue = 0;
                  }
                  numberOfPeak = 1;
              }
          }
          break;
      default:
        // do nothing
        break;
  }
  // if the peaks array is full,
  // then print the output and reset the array
  if (extremes[3] != 0) {
    printToSerial();
    for (int i = 0; i < NUMBER_OF_EXTREMES; i++) {
      extremes[i] = 0;
    }
  }
}

void printToSerial() {
// debugging code: prints all four peaks
 /*
  for (int i = 0; i < NUMBER_OF_EXTREMES; i++) {
      Serial.print(extremes[i]);
      if (i != 3) {
        Serial.print(",");
      }
    }
 */
  // Serial.println();
  // output has format: (1) buffer max (2) sample max
  Serial.print(max(extremes[0],extremes[3]));
  Serial.print(",");
  Serial.print(max(extremes[1],extremes[2]));
  Serial.println();
}
