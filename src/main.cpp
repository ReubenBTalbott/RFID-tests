#include <Arduino.h>          //inlcude the Arduino library, requred to use arduino on Platform io.
#include <MFRC522.h>          //The libary for the RC-522 RFID reader
#include <SPI.h>              //SPI libary used to comunicate with the RC-522
#include <Wire.h>             //I2C libarry used to comunicate with the SSD1306
#include <Adafruit_GFX.h>     //Graphics libary.
#include <Adafruit_SSD1306.h> //SSD1306 OLED Screen library

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 //the OLED reset pin, mine does not have one so it's set to -1 to share the arduino's reset pin, change to the pin yours is attached to if your SSD1306 has a reset pin
#define RST_PIN 9     //The reset pin used by the RC522
#define SS_PIN 10     //The SPI SlaveSelect pin used by the RC522

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Initilise the SSD1306 with the pins and size's defind earier and specify that the screen is using I2C.
MFRC522 mfrc522(SS_PIN, RST_PIN);                                         // Inislize the RC-522 using the pins defined eariler

int booting = 0;      //bootup screen
int instructions = 0; //Prompt to scan card
int code = 0;         //has the user started entering the code yet?
int lock = 0;         //is the system locked still?

static const unsigned char PROGMEM nuke[] = { //Nuke OS logo in bitmap
                                              // 'nuke os', 128x64px
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xc3, 0x80, 0x00, 0xc0, 0x00, 0x00, 0xff, 0x07, 0xc0, 0xfc, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xc3, 0x80, 0x00, 0xc0, 0x00, 0x01, 0x83, 0x08, 0x01, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xe3, 0x80, 0x00, 0xc0, 0x00, 0x03, 0x83, 0x08, 0x02, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xf3, 0x8c, 0x38, 0xc7, 0x3c, 0x03, 0x03, 0x88, 0x04, 0x78, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xb3, 0x8c, 0x38, 0xcc, 0x7f, 0x07, 0x00, 0x8c, 0x04, 0xc0, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xbb, 0x8c, 0x38, 0xf8, 0xe3, 0x87, 0x00, 0x87, 0x84, 0x80, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x9b, 0x8c, 0x38, 0xf8, 0xc3, 0x87, 0x00, 0x87, 0x84, 0x80, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x8b, 0x8c, 0x38, 0xf0, 0xc3, 0x87, 0x00, 0x80, 0xc4, 0x80, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x8f, 0x8c, 0x38, 0xf0, 0xff, 0x07, 0x00, 0x80, 0x74, 0xc0, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x87, 0x8c, 0x38, 0xf8, 0xe0, 0x03, 0x01, 0x80, 0x74, 0x78, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x87, 0x8e, 0x38, 0xf8, 0xc0, 0x03, 0x03, 0x80, 0x74, 0x78, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x87, 0x8f, 0x78, 0xcc, 0xc3, 0x83, 0x87, 0x00, 0x73, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x83, 0x87, 0xf8, 0xc7, 0x7f, 0x80, 0xfc, 0x0f, 0xc0, 0xfc, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xf8, 0x00, 0x03, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0xfc, 0x00, 0x07, 0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0xfc, 0x00, 0x07, 0xf8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xfe, 0x00, 0x07, 0xf8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0xff, 0x00, 0x0f, 0xfc, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0xff, 0x00, 0x0f, 0xff, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0xff, 0x80, 0x1f, 0xff, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x5f, 0xff, 0x80, 0x3f, 0xff, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xff, 0x80, 0x3f, 0xff, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xc0, 0x7f, 0xff, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xe0, 0xff, 0xff, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xf0, 0xff, 0xff, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xf0, 0xff, 0xff, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xc4, 0x7f, 0xff, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0e, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0e, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x04, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0e, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x1f, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x3f, 0x80, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x3f, 0x80, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x7f, 0xc0, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x7f, 0xc0, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x7f, 0xc0, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0xff, 0xf0, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x01, 0xff, 0xf8, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x03, 0xff, 0xf8, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x03, 0xff, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x07, 0xff, 0xfc, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, 0xff, 0xfc, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0xff, 0xfc, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0xff, 0xfc, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0xff, 0xf0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void setup() //setup loop
{

  Serial.begin(9600);                  //start the serial monitor
  Serial.print("Start");               //Prints "Start" in the serial monitor so I know the code has started to run
  SPI.begin();                         //Start the SPI protocol
  mfrc522.PCD_Init();                  //Initalise the RC-522
  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             //set the cursor to the top left pixel which is 0, 0 on the X Y grid             // Start at top-left corner
  display.cp437(true);                 // Set the font to cp437

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //Connect to the LCD and tell it to stepdown the volage to 3.3v
  {
    Serial.println(F("SSD1306 allocation failed")); //If you see this message in the serial monitor your microcontroller has run out of RAM.
    for (;;)
      ; // Don't proceed, loop forever
  }
  if (booting == 0) //booting screen
  {
    for (int x = 0; x < 3; x++) //This will loop the code in it 3 times
    {
      display.clearDisplay();   //clear the display of any text that was on it previously //clear the display of any text that was on it previously
      display.setCursor(0, 0);  //set the cursor to the top left pixel which is 0, 0 on the X Y grid //set the cursor to the top left pixel which is 0, 0 on the X Y grid
      display.write("Booting"); //Write the text "Booting" to the buffer
      display.display();        //Write all the data in the buffer to the screen  //Write all the data in the buffer to the screen  //Write all the data in the buffer to the screen
      delay(500);               //Wait 500ms or 1/2 of a second //Wait 500ms or 1/2 of a second

      display.clearDisplay();    //clear the display of any text that was on it previously //clear the display of any text that was on it previously
      display.setCursor(0, 0);   //set the cursor to the top left pixel which is 0, 0 on the X Y grid
      display.write("Booting."); //Write the text "Booting." to the buffer
      display.display();         //Write all the data in the buffer to the screen
      delay(500);                //Wait 500ms or 1/2 of a second

      display.clearDisplay();     //clear the display of any text that was on it previously //clear the display of any text that was on it previously
      display.setCursor(0, 0);    //set the cursor to the top left pixel which is 0, 0 on the X Y grid
      display.write("Booting.."); //Write the text "Booting.." to the buffer
      display.display();          //Write all the data in the buffer to the screen
      delay(500);                 //Wait 500ms or 1/2 of a second

      display.clearDisplay();      //clear the display of any text that was on it previously //clear the display of any text that was on it previously
      display.setCursor(0, 0);     //set the cursor to the top left pixel which is 0, 0 on the X Y grid
      display.write("Booting..."); //Write the text "Booting..." to the buffer
      display.display();           //Write all the data in the buffer to the screen
      delay(500);                  //Wait 500ms or 1/2 of a second
    }
    display.clearDisplay();                     //clear the display of any text that was on it previously //clear the display of any text that was on it previously
    display.setCursor(0, 0);                    //set the cursor to the top left pixel which is 0, 0 on the X Y grid
    display.drawBitmap(0, 0, nuke, 128, 64, 1); //Write the bitmap 'nuke' to the buffer
    display.display();                          //Write all the data in the buffer to the screen
    delay(5000);                                //Wait 5000ms or 5 seconds
  }
}

void loop()
{ //any code written in here will loop forever

  while (lock == 0) //if lock is set to 0 this loops until lock is set to 1.
  {
    if (instructions == 0) //check if this text is already written to the display, if set to 0 it will run but if it's set to 1 it will skip
    {
      display.clearDisplay();         //clear the display of any text that was on it previously
      display.setCursor(0, 0);        //set the cursor to the top left pixel which is 0, 0 on the X Y grid
      display.write("Please place");  //write "Please place" to the screen buffer"
      Serial.println("Please Place"); //write "Please place" to the serial monitor
      display.setCursor(0, 14);       //set the Cursor to X0 Y14 which will make any display commands start there instead of what was set previously
      display.write("your card on");  //write "your card on" to the screen buffer
      Serial.println("your card on"); //write "your card on" to the serial monitor
      display.setCursor(0, 28);       //set the Cursor to X0 Y28 which will make any display commands start there instead of what was set previously
      display.write("the reader");    //write "the reader" to the screen buffer
      Serial.println("the reader");   //write "the reader" to the serial monitor
      display.display();              //Write all the data in the buffer to the screen
      delay(1000);                    //Wait 1000ms or 1 second
      instructions = 1;               //set 'instrucions' to 1 which will end the loop
    }

    if (mfrc522.PICC_IsNewCardPresent() == true) //if a RFID card is near the reader it will run the code inside the if statment
    {

      if (!mfrc522.PICC_ReadCardSerial()) //requred to read from the card, not sure what it does yet
      {
        return; //terminate the if statment so the next bit of code can run
      }
      display.clearDisplay();                     //clear the display of any text that was on it previously
      display.setCursor(0, 0);                    //set the cursor to the top left pixel which is 0, 0 on the X Y grid
      display.write("Autherizing");               //write "Autherizing" to the screen buffer
      display.display();                          //Write all the data in the buffer to the screen
      delay(2000);                                //Wait 2000ms or 2 seconds
      Serial.print("UID tag :");                  //prints "UID tag" to the serial monitor
      String content = "";                        //Initilize an empty string
      for (byte i = 0; i < mfrc522.uid.size; i++) //Run this 'for' the same ammount as the length of the RFID tag's UID
      {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");           //Prints the UID to the serial monitor
        Serial.print(mfrc522.uid.uidByte[i], HEX);                          //prints the UID to the serial monitor
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ")); //Write the UID to the String
        content.concat(String(mfrc522.uid.uidByte[i], HEX));                //Write the UID to the string
      }
      Serial.println();                          //move down a line in the serial monitor
      content.toUpperCase();                     //make the UID all uppercase
      if (content.substring(1) == "A2 B8 5A 1C") //Check if the UID is the same as the UID in the parentheses
      {
        display.clearDisplay();   //clear the display of any text that was on it previously
        display.setCursor(0, 0);  //set the cursor to the top left pixel which is 0, 0 on the X Y grid
        display.write("Access");  //Write "Access" to the buffer
        display.setCursor(0, 14); //set the Cursor to X0 Y14 which will make any display commands start there instead of what was set previously
        display.write("Granted"); //Write "Granted" to the buffer
        display.display();        //Write all the data in the buffer to the screen
        instructions = 0;         //added for development
        delay(10000);             //wait 10000ms or 10 seconds
        //lock = 1; disabled for development, if set to 1 it would end this intire while loop so the next section of code could run.
      }
      else //if the UID is incorrect this else statement will run
      {
        display.clearDisplay();   //clear the display of any text that was on it previously
        display.setCursor(0, 0);  //set the cursor to the top left pixel which is 0, 0 on the X Y grid
        display.write("Access");  //write "Access" to the buffer
        display.setCursor(0, 14); //set the Cursor to X0 Y14 which will make any display commands start there instead of what was set previously
        display.write("Denied");  //write "Denied" to the buffer
        display.display();        //Write all the data in the buffer to the screen
        instructions = 0;         //Set 'instrucions' to 0 it can run again
        delay(10000);             //wait 10000ms or 10 seconds
      }
    }
  }
}
