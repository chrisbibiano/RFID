#include "SSD1306.h"    // Oled library
#include "MFRC522.h"    // Rfid library

/* wiring the MFRC522 to ESP8266 (ESP-12)
RST     = A0
SDA(SS) = D8    // Slave Select CONNEcT TO SDA ON MFRC522
MOSI    = D7
MISO    = D6
SCK     = D5
GND     = GND
3.3V    = 3.3V
*/
#define RST_PIN	A0  // RST-PIN
#define SS_PIN	D8  // SS-PIN
MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance

// Initialize the OLED display using i2c
// SDA     = D2
// SCL     = D1
#define SCL_PIN	D1  // SCL-PIN
#define SDA_PIN	D2  // SDA-PIN
SSD1306  display(0x3c, SDA_PIN, SCL_PIN);

//Initialize led output
#define greenLed D3

//Initialize switch input pins
#define switch1 A0
#define switch2 D0
#define switch3 D4

// List of matching tag IDs
char* allowedTags[] = {
  "61D2F052",           // Tag 0
  "6470621A",           // Tag 1
  "6ECCBE23",           // Tag 2
  "A5F3FFB0",           // Tag 3
};

// List of names to associate with the matching tag IDs
char* tagName[] = {
  "Jared Paskins",      // Name 0
  "Ben Smith",          // Name 1
  "John Dawson",        // Name 2
  "David Lee",          // Name 3
};

// List of plant areas
char* areas[] = {
  "Admin",              // Area 0
  "Receiving",          // Area 1
  "Store Room",         // Area 2
  "Mixing",             // Area 3
  "Filling",            // Area 4
  "Packing",            // Area 5
  "Stock",              // Area 6
  "Delivery",           // Area 7
};

// Access List to plant areas
byte access[] = {
  B11111111,            // Access list 0
  B11100010,            // Access list 1
  B00100011,            // Access list 2
  B00011100,            // Access list 3
};

// Check the number of tags defined
int numberOfTags = sizeof(allowedTags)/sizeof(allowedTags[0]);

char tagValue[9]; // 8 characters of HEX value + end string character

int read_switch(void);
void dump_byte_array(byte *buffer, byte bufferSize);
int findTag(void);
// byte read_name(void);

                               

void setup() {
  pinMode(switch1, INPUT);
  pinMode(switch2, INPUT);
  pinMode(switch3, INPUT);

  pinMode(greenLed, OUTPUT);                        //Declaring greenled as output
  digitalWrite(greenLed, HIGH);

  // Initialize serial communications
  Serial.begin(9600);
  delay(250);
  Serial.println(F("Booting...."));

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);

  // Initialising the rfid reader
  SPI.begin();	         // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522

  // Print to serial the ready status
  Serial.println(F("Ready!"));
  Serial.println(F("======================================================"));
  Serial.println(F("Scan for Card and print UID:"));
}

void loop() {

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // Show some details of the PICC (that is: the tag/card

  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);

  int tagId = findTag();
  if (tagId>=0){
    int area = read_switch();
    if (area>=0){
      if (bitRead(access[tagId], area)){
        Serial.print(F("Access Granted "));
        display.drawString(0, 48, "Access Granted");
        digitalWrite(greenLed, LOW);
      }
      else{
        Serial.print(F("Access Denied "));
        display.drawString(0, 48, "Access Denied");
      }

      display.display(); 
      delay(8000);
      digitalWrite(greenLed, HIGH);
      Serial.println(F(""));
      display.clear();
      display.display();
    }
  }

}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  String content= "";
  display.drawString(0, 0, "Card read:");
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    content.concat(String(buffer[i] < 0x10 ? "0" : ""));
    content.concat(String(buffer[i], HEX));
  }
  Serial.println(F(""));
  content.toUpperCase();
  content.toCharArray(tagValue, 9);
}


int read_switch(void){
  int value=0;
  bitWrite (value,0,digitalRead(switch3));
  bitWrite (value,1,digitalRead(switch2));
  //bitWrite (value,2,digitalRead(switch1));
  
  if(analogRead(switch1)>500)
    bitWrite (value,2, 1 );
  else 
   bitWrite (value,2, 0 );
  
  if (value<8){
    display.drawString(0, 16, areas[value]);
    Serial.print(F("Location: "));
    Serial.println(areas[value]);
    return value;
  }
  else{
    display.drawString(0, 16, "Not a valid area");
    Serial.print(F("Location: "));
    Serial.println(F("Not a valid area"));
    return -1;
  }
}

/**
 * Search for a specific tag in the database
 */
int findTag(void) {
  for (int thisCard = 0; thisCard < numberOfTags; thisCard++) {
    // Check if the tag value matches this row in the tag database
    if(strcmp(tagValue, allowedTags[thisCard]) == 0){
      Serial.print(F("Name: "));
      Serial.println(tagName[thisCard]);
      display.drawString(0, 32, tagName[thisCard]);
      return(thisCard);
    }
  }
  // If we don't find the tag return a tag ID of -1 to show there was no match
  return -1;
}
