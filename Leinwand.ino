/*
******************************************************************************************************************************************************************
*
* Leinwand Motorized 433.92MHz Projection Screen
* PSAA*
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* Since I only have one of these products, I'm not able to tell if the remotes have unique IDs or if they all send identical
* commands. This code is command-compatible with the rc-switch library, so you can capture your remote either with that or use an
* oscilloscope. You can download rc-switch here: https://github.com/sui77/rc-switch
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
* A single command is: (no AGC/preamble) 24 command bits + radio silence
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
* 1 trailing bit (0, LOW), add to the end of commands if necessary
* = 25 bits in total
* 
* End with LOW radio silence of (minimum) 438 samples = 9932 us
* 
******************************************************************************************************************************************************************
*/



// Example commands (capture your own remotes with the rc-switch library):
#define LEINWAND_DOWN     "000001010101110011110100"
#define LEINWAND_STOP     "000001010101110011110010"
#define LEINWAND_UP       "000001010101110011110001"



// Timings in microseconds (us). Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
// Sample counts listed below with a sample rate of 44100 Hz:
#define LEINWAND_RADIO_SILENCE            9920   // 438 samples
#define LEINWAND_PULSE_SHORT              390    // 17 samples
#define LEINWAND_PULSE_LONG               1030   // 45 samples (approx. PULSE_SHORT * 3)
#define LEINWAND_COMMAND_BIT_ARRAY_SIZE   24     // Command bit count


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
  delay(10000);
  sendLeinwandCommand(LEINWAND_UP);
  delay(10000);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendLeinwandCommand(char* command) {

  if (command == NULL) {
    errorLog("sendLeinwandCommand(): Command array pointer was NULL, cannot continue.");
    return;
  }

  // Prepare for transmitting and check for validity
  pinMode(TRANSMIT_PIN, OUTPUT); // Prepare the digital pin for output
  
  if (strlen(command) < LEINWAND_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendLeinwandCommand(): Invalid command (too short), cannot continue.");
    return;
  }
  if (strlen(command) > LEINWAND_COMMAND_BIT_ARRAY_SIZE) {
    errorLog("sendLeinwandCommand(): Invalid command (too long), cannot continue.");
    return;
  }
  
  // Repeat the command:
  for (int i = 0; i < REPEAT_COMMAND; i++) {
    doLeinwandSend(command);
  }

  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // which will disrupt most appliances operating on the 433.92MHz band:
  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doLeinwandSend(char* command) {

  // Transmit command:
  for (int i = 0; i < LEINWAND_COMMAND_BIT_ARRAY_SIZE; i++) {

    // If current command bit is 0, transmit HIGH-LOW-LOW (wire 100):
    if (command[i] == '0') {
      transmitHigh(LEINWAND_PULSE_SHORT);
      transmitLow(LEINWAND_PULSE_LONG);
    }

    // If current command bit is 1, transmit HIGH-HIGH-LOW (wire 110):
    if (command[i] == '1') {
      transmitHigh(LEINWAND_PULSE_LONG);
      transmitLow(LEINWAND_PULSE_SHORT);
    }
   }

  // rc-switch doesn't record the trailing "0",
  // so we add it here:
  transmitHigh(LEINWAND_PULSE_SHORT);
  transmitLow(LEINWAND_PULSE_LONG);

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
  //PORTB = PORTB D13high; // If you wish to use faster PORTB calls instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitLow(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, LOW);
  //PORTB = PORTB D13low; // If you wish to use faster PORTB calls instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void errorLog(String message) {
  Serial.println(message);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
