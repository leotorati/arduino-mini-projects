#include <Adafruit_NeoPixel.h>
#include <DS3232RTC.h>

// Fix for NeoPixel.
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIXEL_PIN 6
int numPixels = 7;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

String colors[] = {
    "#0000ff", // 0
    "#0000ff",
    "#0020ff",
    "#0020ff",
    "#0040ff",
    "#0040ff",
    "#00aaff", // 6
    "#00aaff",
    "#38d0ff",
    "#7aff8e",
    "#faffe3",
    "#f9f8b2",
    "#ffffff", // 12
    "#fffa94",
    "#fff58a",
    "#fff070",
    "#ff9500",
    "#ff9520",
    "#ff5050", // 18
    "#ff50aa",
    "#ff00cc",
    "#cc00ff",
    "#5000ff",
    "#1000ff"
  };

int brights[] = {
    15, // 0
    20,
    20,
    30,
    30,
    40,
    70, // 6
    100,
    120,
    150,
    200,
    215,
    230, // 12
    215,
    215,
    200,
    200,
    170,
    100, // 18
    50,
    40,
    30,
    30,
    20,
  };

int currentFloat = 0;

void setup() {
  strip.begin();
  strip.setBrightness(50);
  updateAllPixelsColor("#ffffff");
  strip.show();

  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");

  Serial.println("Waiting for commands");
}

void loop() {
  updateByHour();

  if (stringComplete) {
    Serial.print("Received commands: ");
    Serial.println(stringComplete);
    
    // Translate string and change color.
    convertCommands(inputString.c_str());
    
    inputString = "";
    stringComplete = false;
  }

  strip.show();
  delay(5000);
}

void updateByHour() {
  int currentHour = hour();

  Serial.print("Time: ");
  Serial.print(hour());
  Serial.print(":");
  Serial.println(minute());

  String color = colors[currentHour];
  int bright = brights[currentHour];

  Serial.print("Color/Bright: ");
  Serial.print(color);
  Serial.print("/");
  Serial.println(bright);

  updateBrightness(bright);
  updateAllPixelsColor(color);

  String floatColor = "#ff0000";

  if (currentHour > 19 || currentHour < 8) {
    floatColor = "#0000ff";
  }

  updatePixelColor(currentFloat, floatColor);

  currentFloat++;

  if (currentFloat > 6) {
    currentFloat = 0;
  }
}

void convertCommands(char commands[]) {
  char *token = NULL;

  // Example of commands: 01#ffffff;02#ff0011
  token = strtok(commands, ";");

  while (token != NULL) {
    executeCommand(token);
    token = strtok(NULL, ";");
  }
}

void executeCommand(char c_command[]) {
  String command = String(c_command);

  Serial.print("Command: ");
  Serial.println(command);

  if (command.length() < 3) {
    return;
  }

  // Get pixel number/bright and color.
  String pixelNumber = command.substring(0, 2);
  String color = command.substring(2);

  Serial.println("("+pixelNumber+")"+color);

  if (pixelNumber == "--") {
    // Update brightness.
    updateBrightness(color.toInt());

    return;
  }

  if (pixelNumber == "AA") {
    // Update all pixels.
    updateAllPixelsColor(color);

    return;
  }

  if (pixelNumber == "TT") {
    // Update time
    updateTime(color);

    return;
  }

  int pixel = pixelNumber.toInt();
  updatePixelColor(pixel, color);
}

void updateTime(String value) {
  // Format: hhmmssddmmaaaa

  Serial.print("Set time from: ");
  Serial.println(value);
  
  if (value.length() < 14) {
    Serial.println("Time wrong format.");
    return;
  }

  int vhh = value.substring(0,2).toInt();
  int vmi = value.substring(2,4).toInt();
  int vss = value.substring(4,6).toInt();
  int vdd = value.substring(6,8).toInt();
  int vmo = value.substring(8,10).toInt();
  int vaaaa = value.substring(10,14).toInt();

  setTime(vhh, vmi, vss, vdd, vmo, vaaaa);
  RTC.set(now());
}

void updatePixelColor(int pixel, String hex) {
  Serial.print("Set pixel color: ");
  Serial.print(pixel);
  Serial.print(" - ");
  Serial.println(hex);

  // TODO: convert in a function.
  long number = (long) strtol( &hex[1], NULL, 16);
  long r = number >> 16;
  long g = number >> 8 & 0xFF;
  long b = number & 0xFF;

  strip.setPixelColor(pixel, r, g, b);
  // strip.show();
}

void updateBrightness(int brightness) {
  Serial.print("Update brightness: ");
  Serial.println(brightness);

  strip.setBrightness(brightness);
  // strip.show();
}

void updateAllPixelsColor(String hex) {
  Serial.print("Update all colors: ");
  Serial.println(hex);

  long number = (long) strtol( &hex[1], NULL, 16);
  long r = number >> 16;
  long g = number >> 8 & 0xFF;
  long b = number & 0xFF;

  Serial.print("Color: ");
  Serial.print(number);
  Serial.print(", R: ");
  Serial.print(r);
  Serial.print(", G: ");
  Serial.print(g);
  Serial.print(", B: ");
  Serial.println(b);

  for (int i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  
  // strip.show();
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n' || inChar == 'x') {
      stringComplete = true;
      return;
    }

    // add it to the inputString:
    inputString += inChar;
  }
}

