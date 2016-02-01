#define SESSION_LIMIT 20

#define B_REDPIN 3
#define B_GREENPIN 4
#define B_BLUEPIN 2

#define M_REDPIN 6
#define M_GREENPIN 7
#define M_BLUEPIN 5

#define T_REDPIN 9
#define T_GREENPIN 10
#define T_BLUEPIN 8

#define FADESPEED 5 // Make this higher to slow down.

// These constants won't change:
const int ledPin = 13;      // led connected to digital pin 13
const int topSensor = A0; // boven
const int midSensor = A1; // midden
const int botSensor = A2; // onder

const int threshold = 100;  // threshold value to decide when the detected sound is a knock or not

// MAIN PROPERTIES
bool DEBUG = true;

String state = "starting";
int simonArray [SESSION_LIMIT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Some voodoo array magic happening!
int currentSessionLength = 0;
int hitCount = 0;
int registeredInputModule = 0;

// Timings (1k = 1sec)
int timeToReact;
int timeInBetweenSessions;
int timeIndicatorActivated = 750;
int timeInBetweenIndications = 1000;
int currentWaitTime;

// Difficulty settings. These are optional. Leave them as is to not fuck shit up..!
float difficulty = 1.0;
int sessionLength = 0;

void setup() {
  // Default.
  Serial.begin(9600);
  // Create random object so we can extract random numbers.
  randomSeed(analogRead(0));
  // Output we started the game.
  serialOutput("booting", "Starting system...");
  serialOutput("gameModeSwitch", "Starting Simon Says");

  pinMode(T_REDPIN, OUTPUT);
  pinMode(T_GREENPIN, OUTPUT);
  pinMode(T_BLUEPIN, OUTPUT);

  pinMode(B_REDPIN, OUTPUT);
  pinMode(B_GREENPIN, OUTPUT);
  pinMode(B_BLUEPIN, OUTPUT);

  pinMode(M_REDPIN, OUTPUT);
  pinMode(M_GREENPIN, OUTPUT);
  pinMode(M_BLUEPIN, OUTPUT);

  setState("playing");
}

void loop() {
  playSimonSays();
}

void handleLights (int module, String color) {
  int rgb[3];

  if (color == "orange") {
    rgb[0] = 255;
    rgb[1] = 50;
    rgb[2] = 0;
  }
  else if (color == "red") {
    rgb[0] = 255;
    rgb[1] = 0;
    rgb[2] = 0;
  }
  else if (color == "green") {
    rgb[0] = 0;
    rgb[1] = 255;
    rgb[2] = 0;
  }
  else if (color == "blue") {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 255;
  }
  else {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
  }

  switch (module) {
    case 1:
      analogWrite(T_REDPIN, rgb[0]);
      analogWrite(T_GREENPIN, rgb[1]);
      analogWrite(T_BLUEPIN, rgb[2]);
      break;
    case 2:
      analogWrite(M_REDPIN, rgb[0]);
      analogWrite(M_GREENPIN, rgb[1]);
      analogWrite(M_BLUEPIN, rgb[2]);
      break;
    case 3:
      analogWrite(B_REDPIN, rgb[0]);
      analogWrite(B_GREENPIN, rgb[1]);
      analogWrite(B_BLUEPIN, rgb[2]);
      break;

    case 4:
      analogWrite(T_REDPIN, rgb[0]);
      analogWrite(T_GREENPIN, rgb[1]);
      analogWrite(T_BLUEPIN, rgb[2]);
      analogWrite(M_REDPIN, rgb[0]);
      analogWrite(M_GREENPIN, rgb[1]);
      analogWrite(M_BLUEPIN, rgb[2]);
      analogWrite(B_REDPIN, rgb[0]);
      analogWrite(B_GREENPIN, rgb[1]);
      analogWrite(B_BLUEPIN, rgb[2]);
      break;
  }
}

void playSimonSays() {
  nextSession();
  // Display current simon session.
  for (int i = 0; i < currentSessionLength; i++) {

    handleLights(simonArray[i], "orange");
    delay(timeIndicatorActivated);

    handleLights(simonArray[i], "");
    delay(timeInBetweenIndications);
  }

  setState("awaitingInput");

  while (state == "awaitingInput") {
    while (hitCount != currentSessionLength) {
      registerInput();
      if (registeredInputModule > 0) {
        serialOutput("registeredInputModule", translateModuleNumber(registeredInputModule));
        checkInput();
      }
    }
  }
}

bool checkInput() {
  if (registeredInputModule == simonArray[hitCount]) {

    handleLights(registeredInputModule, "green");
    delay(350);
    handleLights(registeredInputModule, "");

    serialOutput("checkInput", "MATCH");
    setHitCount(hitCount + 1); // Bad semantics putting it here, but works...
    setState("playing");
  } else {
    // Show red on all modules.
    handleLights(4, "red");
    delay(3000);
    handleLights(4, "");

    serialOutput("checkInput", "INVALID");
    resetGame();
  }
  setRegisteredInputModule(0);
}

String serialString = "";

void registerInput() {

  // read the sensor and store it in the variable sensorReading:
  int topSensorReading = analogRead(topSensor);
  int midSensorReading = analogRead(midSensor);
  int botSensorReading = analogRead(botSensor);

  Serial.println(String(topSensorReading) + " " + String(midSensorReading) + " " + String(botSensorReading));

  // if the sensor reading is greater than the threshold:
  if (    topSensorReading >= threshold
          ||  midSensorReading >= threshold
          ||  botSensorReading >= threshold) {
    if ((topSensorReading >= midSensorReading) && (topSensorReading >= botSensorReading)) {
      setRegisteredInputModule(1);
    }
    else if ((midSensorReading >= topSensorReading) && (midSensorReading >= botSensorReading)) {
      setRegisteredInputModule(2);
    }
    else if ((botSensorReading >= topSensorReading) && (botSensorReading >= midSensorReading)) {
      setRegisteredInputModule(3);
    }
    else {
      setRegisteredInputModule(0);
    }
  }

  //  while (Serial.available()) {
  //    char inChar = (char)Serial.read();
  //    if (inChar != '\n') {
  //      serialString += inChar;
  //    }
  //    if (inChar == '\n') {
  //      serialOutput("registerInput", serialString);
  //      setRegisteredInputModule(serialString.toInt());
  //      serialString = "";
  //    }
  //  }
}

void nextSession() {
  if (currentSessionLength < SESSION_LIMIT) {
    // Next round.
    handleLights(4, "green");
    delay(1700);
    handleLights(4, "");

    // TODO fix the top one.
    simonArray[currentSessionLength] = random(1, 4);
    Serial.println("Hit this one: " + translateModuleNumber(simonArray[currentSessionLength]));
    setCurrentSessionLength(currentSessionLength + 1);
    setHitCount(0);
  }
  else {
    // Game finished!
    Serial.println("Game is finished");
    // Blink fast in green to indicate you WON!
    for (int i = 0; i < 15; i++) {
      handleLights(4, "green");
      delay(100);
      handleLights(4, "");
      delay(100);
    }

    // Reset values and recall this function to start a new game.
    resetGame();
    nextSession();
  }
}

void resetGame() {
  serialOutput("Resetting", "Starting reset");
  setCurrentSessionLength(0);
  setHitCount(0);
  //  simonArray = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Some voodoo array magic happening!
  resetSimonArray();
  setState("playing");
  serialOutput("Resetting", "Reset finished");
}

void resetSimonArray() {
  for (int i = 0; i < SESSION_LIMIT; i++) {
    simonArray[i] = 0;
  }
}

String translateModuleNumber(int value) {
  switch (value) {
    case 1:
      return "Top";
      break;
    case 2:
      return "Mid";
      break;
    case 3:
      return "Bot";
      break;
    default:
      return "translateModuleNumber: Error: default switch reached!";
      break;
  }
}

void setState(String stateParameter) {
  state = stateParameter;
  serialOutput("Switched state", stateParameter);
}


void setHitCount(int count) {
  serialOutput("changeHitCounter", String(count));
  hitCount = count;
}

void setCurrentSessionLength(int sessionLength) {
  serialOutput("changeCurrentSessionLength", String(sessionLength));
  currentSessionLength = sessionLength;
}

void setRegisteredInputModule(int module) {
  // serialOutput("changeRegisteredInputModule", String(module));
  registeredInputModule = module;
}

void serialOutput(String event, String text) {

  String gameMode = "SimonSays"; // Temp!

  Serial.print("gameMode: ");
  Serial.print(gameMode);
  Serial.print("(");
  Serial.print(DEBUG);
  Serial.print(")");
  Serial.print(" | state: ");
  Serial.print(state);
  Serial.print(" | Event: ");
  Serial.print(event);
  Serial.print(" | message: ");
  Serial.println(text);
}


void setTimeToReact(int timeSpan) {
  timeToReact = timeSpan * difficulty;
}

void setTimeInBetweenSessions(int timeSpan) {
  timeInBetweenSessions = timeSpan * difficulty;
}

void setTimeIndicatorActivated(int timeSpan) {
  timeIndicatorActivated = timeSpan * difficulty;
}

void setTimeInBetweenIndications(int timeSpan) {
  timeInBetweenIndications = timeSpan * difficulty;
}
