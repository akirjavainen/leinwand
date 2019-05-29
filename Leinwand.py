#!/usr/bin/python

"""
* Leinwand Motorized 433.92MHz Projection Screen
*
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
*
* This is a Python implementation for the PSAA* motorized screens, for
* the Raspberry Pi. Plug your transmitter to BOARD PIN 16 (BCM/GPIO23).
*
* HOW TO USE
* ./Leinwand.py [24-bit_binary_string]
*
* More info on the protocol in Leinwand.ino and RemoteCapture.ino here:
* https://github.com/akirjavainen/axel
*
"""

import time
import sys
import os
import RPi.GPIO as GPIO


TRANSMIT_PIN = 16  # BCM PIN 23 (GPIO23, BOARD PIN 16)
REPEAT_COMMAND = 10


# Microseconds (us) converted to seconds for time.sleep() function:
LEINWAND_RADIO_SILENCE = 0.00992

LEINWAND_SHORT = 0.00039
LEINWAND_LONG = 0.00103

LEINWAND_COMMAND_BIT_ARRAY_SIZE = 24


# ------------------------------------------------------------------
def sendLeinwandCommand(command):

    if len(str(command)) is not LEINWAND_COMMAND_BIT_ARRAY_SIZE:
        print "Your (invalid) command was", len(str(command)), "bits long."
        print ""
        printUsage()

    # Prepare:
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(TRANSMIT_PIN, GPIO.OUT)

    # Send command:
    for t in range(REPEAT_COMMAND):
        doLeinwandTribitSend(command)

    # Radio silence at the end of last command:
    transmitLow(LEINWAND_RADIO_SILENCE)

    # Disable output to transmitter and clean up:
    exitProgram()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def doLeinwandTribitSend(command):

    # There is no AGC/preamble

    for i in command:

        if i == '0':  # HIGH-LOW-LOW
            transmitHigh(LEINWAND_SHORT)
            transmitLow(LEINWAND_LONG)

        elif i == '1':  # HIGH-HIGH-LOW
            transmitHigh(LEINWAND_LONG)
            transmitLow(LEINWAND_SHORT)

        else:
            print "Invalid character", i, "in command! Exiting..."
            exitProgram()

    # rc-switch doesn't record the trailing "0",,
    # so we add it here:
    transmitHigh(LEINWAND_SHORT)
    transmitLow(LEINWAND_LONG)

    # Radio silence:
    transmitLow(LEINWAND_RADIO_SILENCE)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitHigh(delay):
    GPIO.output(TRANSMIT_PIN, GPIO.HIGH)
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitLow(delay):
    GPIO.output(TRANSMIT_PIN, GPIO.LOW)
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def printUsage():
    print "Usage:"
    print os.path.basename(sys.argv[0]), "[command_string]"
    print
    print "Correct command length is", LEINWAND_COMMAND_BIT_ARRAY_SIZE, "bits."
    print
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def exitProgram():
    # Disable output to transmitter and clean up:
    GPIO.output(TRANSMIT_PIN, GPIO.LOW)
    GPIO.cleanup()
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
# Main program:
# ------------------------------------------------------------------
if len(sys.argv) < 2:
    printUsage()

sendLeinwandCommand(sys.argv[1])
