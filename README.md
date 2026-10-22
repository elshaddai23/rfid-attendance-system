# RFID Attendance System

An Arduino-based attendance system. A student taps an RFID card or tag on the
reader, and the system identifies the student, reads the time from a real-time
clock, and shows the student's matric number and sign-in time on a 16x2 LCD.

## Features

- Reads the unique ID (UID) of MIFARE RFID cards and tags
- Links each card to a student's matric number through a lookup table
- Shows the matric number and sign-in time (HH:MM:SS DD/MM) on the LCD
- Rejects unregistered cards ("Unknown card")
- Blocks duplicate sign-ins: a second tap shows the original sign-in time
- Resets the sign-in list automatically when the date changes
- Keeps sign-ins in EEPROM, so they survive a power cut or reset

## Hardware

| Component | Role |
|---|---|
| Arduino Uno (or compatible) | Controller |
| RC522 RFID reader (MFRC522), 3.3V | Reads card UIDs over SPI |
| DS1302 real-time clock module with CR2032 | Keeps date and time |
| 16x2 LCD with I2C backpack | Displays results |
| RFID cards / tags (13.56 MHz) | One per student |

## Wiring

| Device | Pin connections |
|---|---|
| RC522 | SDA (SS) to 10, RST to 9, SCK to 13, MOSI to 11, MISO to 12, 3.3V, GND |
| DS1302 | CLK to 5, DAT to 4, RST to 2, VCC to 5V, GND |
| LCD (I2C) | SDA to A4, SCL to A5, VCC to 5V, GND |

The RC522 must be powered from 3.3V, not 5V.

## Libraries

- MFRC522
- Rtc by Makuna (RtcDS1302, ThreeWire, RtcDateTime)
- LiquidCrystal_I2C
- EEPROM (included with the Arduino IDE)

## How it works

1. Read the card's UID with the MFRC522 library.
2. Compare the UID size and every byte against the student table.
3. If the card is known and has not signed in today, store the time and show it.
4. If the card already signed in today, show the original time.
5. Save the sign-in to EEPROM, and clear all records when the day changes.

## Registering students

1. Upload the UID reader sketch and tap each card to read its UID.
2. Add the UID and matric number to the student tables in the main sketch.
3. Update `NUM_STUDENTS`, and change the EEPROM `MARKER` value so the saved layout stays valid.
