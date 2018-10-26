#include <Ultrasonic.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Settings.
#define ULTRASONIC_PIN_TRIGGER 4
#define ULTRASONIC_PIN_ECHO 5

#define RF_CE_PIN 7
#define RF_CSN_PIN 8
// RF connected to SPI ports:
// MOSI 11
// MISO 12
// SCK 13
const byte RX_ADDR[6] = "00001";

Ultrasonic ultrasonic(ULTRASONIC_PIN_TRIGGER, ULTRASONIC_PIN_ECHO);
RF24 radio(RF_CE_PIN, RF_CSN_PIN);

void setup() {
  radio.begin();
  radio.openWritingPipe(RX_ADDR);
  radio.stopListening();
}

void loop() {
  sendDistanceByRadio();
  delay(10);
}

void sendDistanceByRadio() {
  // Sends distance.
  unsigned int distance = getDistance();
   radio.write(&distance, sizeof(distance));
}

unsigned int getDistance() {
  // Reads ultrasonic sensor.
  float distance;
  long microsec = ultrasonic.timing();
  distance = ultrasonic.convert(microsec, Ultrasonic::CM);

  return (unsigned int) distance;
}

