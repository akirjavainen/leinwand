/*
******************************************************************************************************************************************************************
*
* Leinwand Motorized 433.92MHz Projection Screen
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* Since I only have one of these products, I've been unable to verify if the remotes have unique IDs or if they all send identical
* commands. This code is command-compatible with the rc-switch library, so you can capture your remote either with that or use an
* oscillator. You can download rc-switch here: https://github.com/sui77/rc-switch
* 
* 
* HOW TO USE
* 
* Capture the binary commands from your remote with rc-switch and copy paste the 24 bit commands to the #defines below. Then you
* can control your projection screen with sendLeinwandCommand(LEINWAND_DOWN), sendLeinwandCommand(LEINWAND_UP) etc.
* 
* 
* PROTOCOL DESCRIPTION
* 
* Tri-state bits are used.
* A single command is: 25 command bits + radio silence (no AGC)
*
* All sample counts below listed with a sample rate of 44100 Hz (sample count / 44100 = microseconds).
* 
* Pulse length:
* SHORT: approx. 17 samples = 385 us
* LONG: approx. 45 samples = 1020 us
* 
* Data bits:
* Data 0 = short HIGH, long LOW (wire 100)
* Data 1 = long HIGH, short LOW (wire 110)
* 
* Command is as follows:
* 21 bits for (possibly unique) remote control ID (hard coded in remotes?)
* 3 bits for command: DOWN = 100, UP = 001, STOP = 010
* 1 trailing bit (0, LOW) automatically added for rc-switch command compatibility
* = 25 bits in total
* 
* End with LOW radio silence of (minimum) 438 samples = 9932 us
* 
* 
* HOW THIS WAS STARTED
* 
* Commands were captured by a "poor man's oscillator": 433.92MHz receiver unit (data pin) -> 10K Ohm resistor -> USB sound card line-in.
* Try that at your own risk. Power to the 433.92MHz receiver unit was provided by Arduino (connected to 5V and GND).
*
* To view the waveform Arduino is transmitting (and debugging timings etc.), I found it easiest to directly connect the digital pin (13)
* from Arduino -> 10K Ohm resistor -> USB sound card line-in. This way the waveform was very clear.
* 
******************************************************************************************************************************************************************
*/



// Capture your remote with the rc-switch library and paste commands as binary here:
#define LEINWAND_DOWN   "000000000000000000000100"
#define LEINWAND STOP   "000000000000000000000010"
#define LEINWAND_UP     "000000000000000000000001"


// Timings in microseconds (us). Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
// Sample counts listed below with a sample rate of 44100 Hz:
#define LEINWAND_RADIO_SILENCE            9920   // 438 samples
#define LEINWAND_PULSE_SHORT              390    // 17 samples
#define LEINWAND_PULSE_LONG               1030   // 45 samples (approx. PULSE_SHORT * 3)
#define LEINWAND_COMMAND_BIT_ARRAY_SIZE   25     // Command bit count + ending zero (implemented like this for rcswitch compatibility)


#define TRANSMIT_PIN                      13     // We'll use digital 13 for transmitting
#define REPEAT_COMMAND                    10     // How many times to repeat the same command: original remote repeats 10 times
#define DEBUG                             false  // Some extra info on serial


// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600); // Used for error messages even with DEBUG set to false
  if (DEBUG) Serial.println("Starting up...");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  sendLeinwandCommand(LEINWAND_DOWN);
  delay(3000);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendLeinwandCommand(String command) {
  // Prepare for transmitting and check for validity
  pinMode(TRANSMIT_PIN, OUTPUT); // Prepare the digital pin for output
  
  // rc-switch adds an ending zero, so we retain command
  // compatibility with the library by adding it here:
  command = command + "0";

  if (command.length() < LEINWAND_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendLeinwandCommand(): Invalid command (too short), cannot continue.");
    return;
  }
  if (command.length() > LEINWAND_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendLeinwandCommand(): Invalid command (too long), cannot continue.");
    return;
  }

  // Declare the array (int) of command bits
  int command_array[LEINWAND_COMMAND_BIT_ARRAY_SIZE];

  // Processing a string during transmit is just too slow,
  // let's convert it to an array of int first:
  convertStringToArrayOfInt(command, command_array, LEINWAND_COMMAND_BIT_ARRAY_SIZE);
  
  // Repeat the command:
  for (int i = 0; i < REPEAT_COMMAND; i++) {
    doLeinwandSend(command_array);
  }

  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // which will disrupt most appliances operating on the 433.92MHz band:
  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doLeinwandSend(int *command_array) {
  if (command_array == NULL) {
    errorLog("doLeinwandSend(): Array pointer was NULL, cannot continue.");
    return;
  }

  // Transmit command:
  for (int i = 0; i < LEINWAND_COMMAND_BIT_ARRAY_SIZE; i++) {

    // If current command bit is 0, transmit SHORT HIGH and LONG LOW (wire 100):
    if (command_array[i] == 0) {
      transmitHigh(LEINWAND_PULSE_SHORT);
      transmitLow(LEINWAND_PULSE_LONG);
    }

    // If current command bit is 1, transmit LONG HIGH and SHORT LOW (wire 110):
    if (command_array[i] == 1) {
      transmitHigh(LEINWAND_PULSE_LONG);
      transmitLow(LEINWAND_PULSE_SHORT);
    }
   }

  // Radio silence at the end.
  // It's better to rather go a bit over than under required length.
  transmitLow(LEINWAND_RADIO_SILENCE);
  
  if (DEBUG) {
    Serial.println();
    Serial.print("Transmitted ");
    Serial.print(LEINWAND_COMMAND_BIT_ARRAY_SIZE);
    Serial.println(" bits.");
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitHigh(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, HIGH);
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitLow(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, LOW);
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
int convertStringToInt(String s) {
  char carray[2];
  int i = 0;
  
  s.toCharArray(carray, sizeof(carray));
  i = atoi(carray);

  return i;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void convertStringToArrayOfInt(String command, int *int_array, int command_array_size) {
  String c = "";

  if (int_array == NULL) {
    errorLog("convertStringToArrayOfInt(): Array pointer was NULL, cannot continue.");
    return;
  }
 
  for (int i = 0; i < command_array_size; i++) {
      c = command.substring(i, i + 1);

      if (c == "0" || c == "1") {
        int_array[i] = convertStringToInt(c);
      } else {
        errorLog("convertStringToArrayOfInt(): Invalid character " + c + " in command.");
        return;
      }
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void errorLog(String message) {
  Serial.println(message);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
