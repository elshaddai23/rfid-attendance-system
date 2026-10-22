#include<SPI.h> 
#include<MFRC522.h>


MFRC522 rfid(10, 9);

void printHexByte(byte number){
  if(number < 0x10){
    Serial.print("0");
  }
  Serial.print(number, HEX);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init(); // start the reader using the pinsfrom the constructor 
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  rfid.PCD_DumpVersionToSerial(); // prints the chip version. A normal number means wiring works. 0x00 0r 0xFF means wiring is wrong 
  Serial.println("Scan a card..."); // prints scan a card
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!rfid.PICC_IsNewCardPresent()){
    return;
  } // check if a new card has entered the field, if no new card has return it starts again 

  if(!rfid.PICC_ReadCardSerial()){
    return;
  } //check if a card has been called(i.e select a card and fills uid. true= success)

  Serial.print("UID size: ");
  Serial.println(rfid.uid.size); // prints the uid size (uid.size means how many byte are real )

  Serial.print("UID: ");
  for(byte i=0; i < (rfid.uid.size); i++){
    printHexByte(rfid.uid.uidByte[i]); // prints the ID bytes 
    Serial.print(" ");
  }
  Serial.println();

  Serial.println(rfid.PICC_GetTypeName(rfid.PICC_GetType(rfid.uid.sak)));


  rfid.PICC_HaltA(); // tells the card to go quiet so it is not read again staright away
  rfid.PCD_StopCrypto1(); // ends the secure session
}
