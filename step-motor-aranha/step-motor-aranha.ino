#include "Ultrasonic.h"
#include <Stepper.h>

const int stepsPerRevolution = 500; 
const long maxStepsDistance = 55000; // +- 2,4m
const long minStepsDstance = 0;

int initialDistance = 0;
int triggerDistance = 0;
long currentStepDistance = 0;

unsigned long triggerDistanceTimer = 0;
int triggerDistanceTimeout = 1000;
int currentTriggerDistance = 0;

Stepper myStepper(stepsPerRevolution, 8,10,9,11); 
Ultrasonic ultrasonic(4, 5);

void setup() { 
  Serial.begin(9600);
  myStepper.setSpeed(60);
  
  initialDistance = readInitialTriggerDistance();
  triggerDistance = initialDistance - (initialDistance / 4);
} 
  
void loop() {
  bool spiderDown = shouldSpiderDown();
  moveSpider(spiderDown);
}

void moveSpider(bool spiderDown) {
  int spiderStep = spiderDown ? 50 : -50;
  bool stepOnLimit = false;

  if (spiderDown && currentStepDistance >= maxStepsDistance) {
    stepOnLimit = true;
  }

  if (!spiderDown && currentStepDistance <= minStepsDstance) {
    stepOnLimit = true;
  }

  if (stepOnLimit) {
    return;
  }

  myStepper.step(spiderStep);
  currentStepDistance = currentStepDistance + spiderStep;

  Serial.print("Current/Max/Min/Step: ");
  Serial.print(currentStepDistance);
  Serial.print("/");
  Serial.print(maxStepsDistance);
  Serial.print("/");
  Serial.print(minStepsDstance);
  Serial.print("/");
  Serial.println(spiderStep);
}

bool shouldSpiderDown() {
  int currentDistance = readCurrentTriggerDistance();

  return currentDistance <= triggerDistance;
}

int readCurrentTriggerDistance() {
  if (millis() - triggerDistanceTimer < triggerDistanceTimeout) {
    return currentTriggerDistance;
  }

  // Reset timer.
  triggerDistanceTimer = millis();

  const int samplesCount = 3;
  int samplesTotal = 0;
  for (int i = 0; i < samplesCount; i++) {
    samplesTotal = samplesTotal + ultrasonic.distanceRead();
  }

  currentTriggerDistance = samplesTotal / samplesCount;

  Serial.print("Distance current/initial/trigger: ");
  Serial.print(currentTriggerDistance);
  Serial.print("/");
  Serial.print(initialDistance);
  Serial.print("/");
  Serial.println(triggerDistance);

  return currentTriggerDistance;
}

int readInitialTriggerDistance() {
  // Get 5 samples.
  int samplesTotal = 0;
  for (int i = 0; i < 5; i++) {
    samplesTotal = samplesTotal + ultrasonic.distanceRead();
    delay(200);
  }

  return samplesTotal / 5;
}

