// Verzija: v.4 Date: 24.1.2016
// Radi slanje Tag-a na server u pravom obliku, upper case
// Koristena konverzija 4 karaktera u String da bi se dobila cijela rijec/tag.
// Verzija u kojoj su svi komentari i visak Cod-a koji treba ocistit. Kao i visak varijabli


#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>  // Library for Mifare RC522 Devices
#include <Ethernet.h>


#define COMMON_ANODE

#ifdef COMMON_ANODE
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif

#define redLed 6    // Set Led Pins
#define greenLed 7
#define blueLed 8
#define buzzer 9
#define relay 4     // Set Relay Pin
#define wipeB 3     // Button pin for WipeMode

boolean match = false;          // initialize card match to false
boolean programMode = false;  // initialize programming mode to false

int successRead;    // Variable integer to keep if we have Successful Read from Reader

byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM

//ethernet postavke
char inString[32]; // string for incoming serial data
int stringPos = 0; // string index counter
boolean startRead = false; // is reading?

String SkeniraniTagTemp;  // moja varijabla


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "reflect.ba";    // name address for Google (using DNS)
IPAddress ip(192,168,0,15);
EthernetClient client;
int led1 = 8;
int led2 = 9;
String result;
/*
  We need to define MFRC522's pins and create instance
  Pin layout should be as follows (on Arduino Uno):
  MOSI: Pin 11 / ICSP-4
  MISO: Pin 12 / ICSP-1
  SCK : Pin 13 / ICSP-3
  SS : Pin 10 (Configurable)
  RST : Pin 9 (Configurable)
  look MFRC522 Library for
  other Arduinos' pin configuration 
 */

#define SS_PIN 53
#define RST_PIN 46
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
  // ethernet postavke LEDICA
   Ethernet.begin(mac);
   // Ovo sam kopirao za pokretanje etherneta
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);
  //Arduino Pin Configuration
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(wipeB, INPUT_PULLUP);   // Enable pin's pull up resistor
  pinMode(relay, OUTPUT);
  //Be careful how relay circuit behave on while resetting or power-cycling your Arduino
  digitalWrite(relay, HIGH);    // Make sure door is locked
  digitalWrite(buzzer, HIGH);    // Make sure door is locked
  digitalWrite(redLed, LED_OFF);  // Make sure led is off
  digitalWrite(greenLed, LED_OFF);  // Make sure led is off
  digitalWrite(blueLed, LED_OFF); // Make sure led is off

  //Protocol Configuration
  Serial.begin(9600);  // Initialize serial communications with PC
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware
  
  //If you set Antenna Gain to Max it will increase reading distance
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  
  Serial.println(F("Access Control v3.3"));   // For debugging purposes
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details

  //Wipe Code if Button Pressed while setup run (powered on) it wipes EEPROM
  if (digitalRead(wipeB) == LOW) {  // when button pressed pin should get low, button connected to ground
    digitalWrite(redLed, LED_ON); // Red Led stays on to inform user we are going to wipe
    Serial.println(F("Wipe Button Pressed"));
    Serial.println(F("You have 5 seconds to Cancel"));
    Serial.println(F("This will be remove all records and cannot be undone"));
    delay(5000);                        // Give user enough time to cancel operation
    if (digitalRead(wipeB) == LOW) {    // If button still be pressed, wipe EEPROM
      Serial.println(F("Starting Wiping EEPROM"));
      for (int x = 0; x < EEPROM.length(); x = x + 1) {    //Loop end of EEPROM address
        if (EEPROM.read(x) == 0) {              //If EEPROM address 0
          // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
        }
        else {
          EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
        }
      }
      Serial.println(F("EEPROM Successfully Wiped"));
      digitalWrite(redLed, LED_OFF);  // visualize successful wipe
      delay(200);
      digitalWrite(redLed, LED_ON);
      delay(200);
      digitalWrite(redLed, LED_OFF);
      delay(200);
      digitalWrite(redLed, LED_ON);
      delay(200);
      digitalWrite(redLed, LED_OFF);
    }
    else {
      Serial.println(F("Wiping Cancelled"));
      digitalWrite(redLed, LED_OFF);
    }
  }
  // Check if master card defined, if not let user choose a master card
  // This also useful to just redefine Master Card
  // You can keep other EEPROM records just write other than 143 to EEPROM address 1
  // EEPROM address 1 should hold magical number which is '143'
  if (EEPROM.read(1) != 143) {      
    Serial.println(F("No Master Card Defined"));
    Serial.println(F("Scan A PICC to Define as Master Card"));
    do {
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      digitalWrite(blueLed, LED_ON);    // Visualize Master Card need to be defined
      delay(200);
      digitalWrite(blueLed, LED_OFF);
      delay(200);
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( int j = 0; j < 4; j++ ) {        // Loop 4 times
      EEPROM.write( 2 + j, readCard[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
    Serial.println(F("Master Card Defined"));
  }
  Serial.println(F("-------------------"));
  Serial.println(F("Master Card's UID"));
  for ( int i = 0; i < 4; i++ ) {          // Read Master Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Everything Ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
  cycleLeds();    // Everything ready lets give user some feedback by cycling leds


  //nastavak za ethernet kontrolu////////////////////////////////// E T H E R N E T ////////////////////////////////////////////////////////////////////////////////
   
}



// spajanje na server i provjera skeniranog tokena  MARKO  SkeniraniTagTemp
String connectAndRead(String Tag){
  //connect to the server
  Serial.println("\nSkenirani Tag: "+Tag+"\n");
 // Serial.println(Tag);
////////////////////////////////////////////////  Serial.println("connecting...");

  //port 80 is typical of a www page
  if (client.connect(server, 80)) {
   //////////////////////////////////////////// Serial.println("connected");
    // Make your API request:
  client.println("GET /arduino/index.php?device=DFTK35&userkey="+Tag+" HTTP/1.0");
    //MojaVari1
// client.print(Tag);
//client.print(" HTTP/1.0");
 //   client.print( "GET /testserver/arduino_temperatures/add_data.php?");
    client.print( "Host: " );
    client.println(server);
    client.println("Connection: close");
    client.println();

    //Connected - Read the page
    return readPage(); //go and read the output

  }else{
    return "connection failed";
  }

}
String readPage(){
  //read the page, and capture & return everything between '<' and '>'

  stringPos = 0;
  memset( &inString, 0, 32 ); //clear inString memory

  while(true){

    if (client.available()) {
      char c = client.read();

      if (c == '#' ) { //'<' is our begining character
        startRead = true; //Ready to start reading the part 
      }else if(startRead){

        if(c != '&'){ //'>' is our ending character
          inString[stringPos] = c;
          stringPos ++;
        }else{
          //got what we need here! We can disconnect now
          startRead = false;
          client.stop();
          client.flush();
        ///////////////////////////////  Serial.println("disconnecting.");
          return inString;

        }

      }
     
    }

  }

}



///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
//pozivanje funkcije za provjeri ethernet-a

//kraj pozivanja funkcije


  
  do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
    if (programMode) {
      cycleLeds();              // Program Mode cycles through RGB waiting to read a new card
    }
    else {
      normalModeOn();     // Normal mode, blue Power LED is on, all others are off
    }
  }
  while (!successRead);   //the program will not go further while you not get a successful read
  if (programMode) {
    if ( isMaster(readCard) ) { //If master card scanned again exit program mode
      Serial.println(F("Master Card Scanned"));
      Serial.println(F("Exiting Program Mode"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      ///////// Marko
      masterkartica_exit();
      ///////// Marko
      return;
      
    }
    else {
      if ( findID(readCard) ) { // If scanned card is known delete it
        Serial.println(F("I know this PICC, removing..."));
        deleteID(readCard);
        Serial.println("-----------------------------");
      }
      else {                    // If scanned card is not known add it
        Serial.println(F("I do not know this PICC, adding..."));
        writeID(readCard);
        Serial.println(F("-----------------------------"));
      }
    }
  }
  else {
    if ( isMaster(readCard) ) {   // If scanned card's ID matches Master Card's ID enter program mode
      programMode = true;
      ///////// Marko
      masterkartica_enter();
      ///////// Marko
      Serial.println(F("Hello Master - Entered Program Mode"));
      int count = EEPROM.read(0);   // Read the first Byte of EEPROM that
      Serial.print(F("I have "));     // stores the number of ID's in EEPROM
      Serial.print(count);
      Serial.print(F(" record(s) on EEPROM"));
      Serial.println("");
      Serial.println(F("Scan a PICC to ADD or REMOVE"));
      Serial.println(F("-----------------------------"));
    }
    else {
      //provjera da li je kartica u epromu vec memorirana
      /////////////////////////////
      ////////////////////////////////
      ///////////////////////////////////
      if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
        Serial.println(F("Welcome, You shall pass"));
        
        granted(5000);          // Open the door lock for 300 ms
      }
      else {      // If not, show that the ID was not valid
        Serial.println(F("You shall not pass"));
        denied();
      }
    }
  }
//  ispisProvjera();
}

/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted (int setDelay) {
  digitalWrite(blueLed, LED_OFF);   // Turn off blue LED
  digitalWrite(redLed, LED_OFF);  // Turn off red LED
  digitalWrite(greenLed, LED_ON);   // Turn on green LED
  digitalWrite(relay, LOW);     // Unlock door!
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(80);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(setDelay);          // Hold door lock open for given seconds
  digitalWrite(relay, HIGH);    // Relock door
  delay(1000);            // Hold green LED on for a second
  ///////////////////////////////////////////////////
  
      ////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_ON);   // Turn on red LED
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(400);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(1000);
}


///////////////////////////////////////// Get PICC's UID /////////////////////////////////// SOLOMUN
int getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for (int i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    // Serial.print(readCard[i], HEX);
  }
  Serial.println("Gore je komentirano da se ne ispisuje skenirani tag");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    while(true);  // do not go further
  }
}

///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
void cycleLeds() {
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
}

//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, LED_ON);  // Blue LED ON and ready to read card
  digitalWrite(redLed, LED_OFF);  // Make sure Red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure Green LED is off
  digitalWrite(relay, HIGH);    // Make sure Door is Locked
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( int number ) {
  int start = (number * 4 ) + 2;    // Figure out starting position
  for ( int i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    int num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    int start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( int j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    successWrite();
  Serial.println(F("Succesfully added ID record to EEPROM"));
 ////// Marko
  potvrda_upisa(); //buzzer
  ///// Marko
  }
  else {
    failedWrite();
  Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    failedWrite();      // If not
  Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  odbijanje();
  }
  else {
    int num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    int slot;       // Figure out the slot number of the card
    int start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    int looping;    // The number of times the loop repeats
    int j;
    int count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( int k = 0; k < 4; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
  Serial.println(F("Succesfully removed ID record from EEPROM"));
 potvrda_brisanja();
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != NULL )       // Make sure there is something in the array first
    match = true;       // Assume they match at first
  for ( int k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] )     // IF a != b then set match = false, one fails, all fail
      match = false;
  }
  if ( match ) {      // Check to see if if match is still true
    return true;      // Return true
  }
  else  {
    return false;       // Return false
  }
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
int findIDSLOT( byte find[] ) {
  int count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( int i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
      break;          // Stop looking we found it
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   /////////////////////////////////// MARKO
// ODradio sam da se upisuje u bazu svako skeniranje. Ujedno u PHP ima definiran broj TAGA-a pa php provjeri jel tag postoji i vrati true ako postoju, false ako ne postoji.
// jos imam da odradim query na tabelu i da vidim da li tag postoji u tabeli. Moram php-u proslijediti variablu.
// moram u connectAndRead ubaciti variablu koja ce biti skenirani tag. Funkcija ce na poziv ocekivati vrijednost za primanje a to ce biti broj TAG-a. Samo ce ga proslijediti u linku na php koji ce dalje provjeravati
boolean findID( byte find[] ) {

// ********************  for (int i = 0; i < 4; i++) {  //
    readCard[0] = (mfrc522.uid.uidByte[0]);
    readCard[1] = (mfrc522.uid.uidByte[1]);
    readCard[2] = (mfrc522.uid.uidByte[2]);
    readCard[3] = (mfrc522.uid.uidByte[3]);
    readCard[4] = (mfrc522.uid.uidByte[4]);
 
                         // Serial.println(readCard[i]); 
   //                     String str0;
    //                    char cstr[18];
   //                     str = String(readCard[i], HEX);
   //                       str.toCharArray(cstr,18);
   //                       Serial.print(cstr); 
    //                    SkeniraniTagTemp = cstr;


            
             
            
           char A[5];  
           char B[5]; 
           char C[5]; 
           char D[5]; 
           char E[5];           
           String strA; 
           String strB;
           String strC;
           String strD;
           String strE;           
           strA=String(readCard[0], HEX);
           strB=String(readCard[1], HEX);
           strC=String(readCard[2], HEX);
           strD=String(readCard[3], HEX);
           strE=String(readCard[4], HEX);
            
          strA.toCharArray(A,5);
          strB.toCharArray(B,5);
          strC.toCharArray(C,5);
          strD.toCharArray(D,5);
          strE.toCharArray(E,5);

char myBigArray[128];
//myBigArray[0] = '\';
strcat(myBigArray, A);
strcat(myBigArray, B);
strcat(myBigArray, C);
strcat(myBigArray, D);
//strcat(myBigArray, E);      
       
      SkeniraniTagTemp = myBigArray;
      SkeniraniTagTemp.toUpperCase();
//Serial.print("ISPIS: ");
//Serial.print(myBigArray);
           

            
 // da sprint odradim svaku variablo zasebno i onda ih na kraju sastavim sa + Val + Val + Val...  kao karaktere


///Ovdje moram skontat kako da niz stavim u jednu variablu String 
//Petlja se ponovi 4 puta. Po dva slova svaki put se ispisu. Samo ih trebam nadodavat. 
 //******************* }
  
 Serial.print("ISPIS SASTAVLJENIH U STRING: ");  //ispisuje sve variable, samo ih sada trebam sastavit u komad
  Serial.print(SkeniraniTagTemp); 
// Serial.print(MojaVari1, HEX);
//Serial.print(MojaVari2, HEX);
 // Serial.print(MojaVari3, HEX);
 // Serial.print(MojaVari4, HEX);
// Serial.print(MojaVari5, HEX);


//  Serial.print("Server response: " + MojaVari1 + MojaVari2 + MojaVari3 + MojaVari4 + MojaVari2);
 String pageValue = connectAndRead(SkeniraniTagTemp); //connect to the server and read the output
Serial.print("Server response: ");
  Serial.println(pageValue); //print out the findings.
      if (pageValue == "D")
      {
       return true;
         
      } else {
        return false;
      }
  //int count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  //for ( int i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
   // readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
   // if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
    //  return true;
    //  break;  // Stop looking we found it
   // }
   // else {    // If not, return false
  //  }
  //}
  //return false;
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Flashes the green LED 3 times to indicate a successful write to EEPROM
void successWrite() {
  //potvrda_upisa();
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
}

///////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
// Flashes the red LED 3 times to indicate a failed write to EEPROM
void failedWrite() {
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
}

///////////////////////////////////////// Success Remove UID From EEPROM  ///////////////////////////////////
// Flashes the blue LED 3 times to indicate a success delete to EEPROM
void successDelete() {
  //potvrda_brisanja ();
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
boolean isMaster( byte test[] ) {
  if ( checkTwo( test, masterCard ) )
    return true;
  else
    return false;
}

//////////////////////////////////////// Buzzer potvrda  ///////////////////////////////////
void potvrda_upisa () {
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, HIGH);     // Unlock door!
}

//////////////////////////////////////// Buzzer potvrda //////////////////////////////

//////////////////////////////////////// Buzzer brisanje  ///////////////////////////////////
void potvrda_brisanja () {
 digitalWrite(buzzer, LOW);     // Unlock door!
  delay(600);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(400);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(600);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(400); 
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(600);
  digitalWrite(buzzer, HIGH);     // Unlock door!
}

//////////////////////////////////////// Buzzer brisanje //////////////////////////////


//////////////////////////////////////// Buzzer odbijanje  ///////////////////////////////////
void odbijanje () {
 digitalWrite(buzzer, LOW);     // Unlock door!
  delay(600);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(100);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(600);
  digitalWrite(buzzer, HIGH);     // Unlock door!
}

//////////////////////////////////////// Buzzer odbijanje //////////////////////////////
void masterkartica_enter() {
 digitalWrite(buzzer, LOW);     // Unlock door!
  delay(700);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(250);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, HIGH);     // Unlock door!
}

void masterkartica_exit() {
 digitalWrite(buzzer, LOW);     // Unlock door!
  delay(50);
  digitalWrite(buzzer, HIGH);     // Unlock door!
  delay(250);
  digitalWrite(buzzer, LOW);     // Unlock door!
  delay(700);
  digitalWrite(buzzer, HIGH);     // Unlock door!
}
