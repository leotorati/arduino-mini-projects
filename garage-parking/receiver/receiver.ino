
#include <Adafruit_NeoPixel.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Fix for NeoPixel.
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Settings.
#define DISTANCE_MAX_CM 200
#define DISTANCE_MIN_CM 20

#define WLED_STRIP_PIN 9
#define WLED_STRIP_NUM_LEDS 7
#define WLED_STRIP_INTENSITY 210
#define WLED_STRIP_COLOR_WHEEL_INITIAL 5
#define WLED_STRIP_COLOR_WHEEL_FINAL 200

#define RF_CE_PIN 7
#define RF_CSN_PIN 8
// RF connected to SPI ports:
// MOSI 11
// MISO 12
// SCK 13
const byte RX_ADDR[6] = "00001";

#define BUZZER_PIN 10
#define BUZZER_FREQUENCE 2000

#define ALERT_ON_DURATION 100
#define ALERT_OFF_MAX_DURATION 2000
#define ALERT_OFF_MIN_DURATION 10

#define COMMUNICATION_TIMEOUT_LIMIT 1000

Adafruit_NeoPixel wLedStrip = Adafruit_NeoPixel(WLED_STRIP_NUM_LEDS, WLED_STRIP_PIN, NEO_GRB + NEO_KHZ800);
RF24 radio(RF_CE_PIN, RF_CSN_PIN);

// Controls.
unsigned int distance = -1;
bool alertOn = false;
bool alertMinDistance = false;
unsigned long alertTimer = 0;
unsigned long communicationTimeoutTimer = 0;

void setup() {
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  pinMode(BUZZER_PIN,OUTPUT);

  wLedStrip.begin();
  wLedStrip.setBrightness(WLED_STRIP_INTENSITY);

  radio.begin();
  radio.openReadingPipe(0, RX_ADDR);
  radio.startListening();
}

void loop() {
  updateDistanceByRadio();
  delay(50);

  if (distance == -1) {
    setAlertOff();
    setWLedStripInitialValue();
    return;
  }
  
  updateAlert();
}

void updateDistanceByRadio() {
  if (radio.available()) {
    unsigned int newDistance;
    radio.read(&newDistance, sizeof(newDistance));
    updateDistance(newDistance);

    // Resets timer.
    communicationTimeoutTimer = millis();
    return;
  }

  // Communication timeout.
  if (millis() - communicationTimeoutTimer > COMMUNICATION_TIMEOUT_LIMIT) {
    distance = -1;
  }
}

void updateAlert() {
  if (distance < DISTANCE_MIN_CM) {
    if (alertOn) {
      if (!alertMinDistance) {
        turnOnWLedStrip();
        alertMinDistance = true;
      }
      
      return;
    }

    setAlertOn();
  }

  alertMinDistance = false;
  
  unsigned long alertDuration = getAlertDuration();

  if (millis() - alertTimer < alertDuration) {
    return;
  }

  toggleAlert();
}

void toggleAlert() {
  alertOn = !alertOn;

  // Reset alertTimer.
  alertTimer = millis();

  if (alertOn) {
    setAlertOn();
    return;
  }

  setAlertOff();
}

void setAlertOn() {
  turnOnBuzzer();
  turnOnWLedStrip();
}

void setAlertOff() {
  turnOffBuzzer();
  turnOffWLedStrip();
}

void turnOnWLedStrip() {
  uint32_t color = getColor();
  
  for (int i = 0; i <wLedStrip.numPixels(); i++) {
    wLedStrip.setPixelColor(i, color);
  }

  wLedStrip.show();
}

void turnOffWLedStrip() {
  // Black color is equal off pixel.
  uint32_t color = wLedStrip.Color(0, 0, 0);

  // Keeps first pixel.
  for (int i = 1; i <wLedStrip.numPixels(); i++) {
    wLedStrip.setPixelColor(i, color);
  }

  wLedStrip.show();
}

void setWLedStripInitialValue() {
  wLedStrip.setPixelColor(0, wLedStrip.Color(255,255,255));
  wLedStrip.show();
}

void turnOnBuzzer() {
  tone(BUZZER_PIN, BUZZER_FREQUENCE);
}

void turnOffBuzzer() {
  noTone(BUZZER_PIN);
}

unsigned long getAlertDuration() {
  if (alertOn) {
    return ALERT_ON_DURATION;
  }

  return map(
    distance,
    DISTANCE_MIN_CM,
    DISTANCE_MAX_CM,
    ALERT_OFF_MIN_DURATION,
    ALERT_OFF_MAX_DURATION
  );
}

unsigned int updateDistance(unsigned int newDistance) {
  if (newDistance > DISTANCE_MAX_CM) {
    newDistance = DISTANCE_MAX_CM;
  }

  distance = newDistance;
}

uint32_t getColor() {
  if (distance < DISTANCE_MIN_CM) {
    return wLedStrip.Color(255, 0, 0);
  }

  byte wheelPosition = map(
    distance,
    DISTANCE_MIN_CM,
    DISTANCE_MAX_CM,
    WLED_STRIP_COLOR_WHEEL_INITIAL,
    WLED_STRIP_COLOR_WHEEL_FINAL
  );

  return colorWheel(wheelPosition);
}

uint32_t colorWheel(byte wheelPosition) {
  wheelPosition = 255 - wheelPosition;
  if(wheelPosition < 85) {
    return wLedStrip.Color(255 - wheelPosition * 3, 0, wheelPosition * 3);
  }
  if(wheelPosition < 170) {
    wheelPosition -= 85;
    return wLedStrip.Color(0, wheelPosition * 3, 255 - wheelPosition * 3);
  }
  wheelPosition -= 170;
  return wLedStrip.Color(wheelPosition * 3, 255 - wheelPosition * 3, 0);
}

