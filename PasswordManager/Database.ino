// Use the external SPI SD card as storage
#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>

#if USE_SDCARD
#include <SD.h>
#define SD_PIN 10  // SD Card CS pin15
#else
#include "DummyData.h" // fake preloaded data
#endif

#ifndef SerialMonitorInterface
#if defined (ARDUINO_ARCH_AVR)
#define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
#define SerialMonitorInterface SerialUSB
#endif
#define MAX_UART_BUFFER 128'
#endif

char key[10][10] = {0};
char value[10][10] = {0};
int Row = 0;
char filename[]="db.txt";
char receivedKey[] = "fan";
char receivedValue[] = "club";
int action = 0; //add:1,remove:2
int displayRow = 0;

#if USE_SDCARD
File myFile;
#endif

void setupDisplay(){
  Wire.begin();
  display.begin();
  display.setBrightness(10);
}

void setupDB(){
  SerialMonitorInterface.println(" Ini Parser begins:");
  SerialMonitorInterface.println();

  #if USE_SDCARD
  SerialMonitorInterface.println("Initializing SD card...");
  if (!SD.begin(SD_PIN)) {
    SerialMonitorInterface.println("No SD-card.");
    return;
  }else{
    SerialMonitorInterface.println("initialization done.");
  }

  myFile = SD.open(filename);
  if (!myFile) {
    myFile = SD.open(filename);
  }
  
  if (myFile){
    readIni();
  }else{
    SerialMonitorInterface.println("File(Read) not open");
  }

  myFile.close();
  #else
  SerialMonitorInterface.println("Skipping SD Card. Using in-memory volatile database.");
  for(int i = 0; i < dummy::len && i < (sizeof(key) / sizeof(key[0])); i++){
    strcpy(key[i], dummy::key[i]);
    strcpy(value[i], dummy::value[i]);
    Row++;
  }
  #endif
  
  printrow();
  printArray();
  
  writeText();
  

  
  
  #if USE_SDCARD
  //Adding to file 
  if (action == 1){
      myFile = SD.open(filename, FILE_WRITE);
      if (myFile){
        SerialMonitorInterface.println("Adding a password");
        SerialMonitorInterface.println("==================");
        if (receivedKey && receivedValue){
          addIni(receivedKey,receivedValue); 
        }
        myFile.close();
      }else{
        SerialMonitorInterface.println("File(foradding) not open");
      }
      Row++;
      printrow();
      printArray();  
  }
  
  //Remove from file
  if (action == 2){
    SerialMonitorInterface.println("Removing a password");
    SerialMonitorInterface.println("==================");
    if (receivedKey){
        removeIni(receivedKey);  
    }
    printrow();
    printArray();
  }
  #endif
}
void db_loop(){
  static unsigned long startTime = millis();
  static bool was_pressed = false;
  if(millis() - startTime > 200){
    startTime = millis();
    if(display.getButtons(TSButtonUpperRight|TSButtonLowerRight|TSButtonLowerLeft)
      && !was_pressed){
      // buttonDown
      was_pressed = true;
      buttonLoop();
    }
    else if(!display.getButtons(TSButtonUpperRight|TSButtonLowerRight) && was_pressed){
      // buttonUp
      was_pressed = false;
    }
  }
}

// BEGIN SDCARD OPERATIONS
#if USE_SDCARD
//Read from file and add key/value pair to arrays
void readIni(){
        int equal = 0;
        int line = 0;
        int col = 0;
     while (myFile.available()) {
        char c = myFile.read();
        if (c == '='){
          //SerialMonitorInterface.println("equal");
          equal = 1;
          col=0;
          continue;
        }

        if (c == '\n'){
          //SerialMonitorInterface.println("New Line");
          key[line][col] = c;
          equal = 0;
          line++;
          col = 0;
          Row++;
          continue;
        }

        if(equal == 0){
           key[line][col] = c;
        }

        if(equal == 1){
            value[line][col] = c;
        }
        col++;
          
  }
}

//Add new key and value to text file
void addIni(char inputkey[], char inputvalue[]){
  char finalinput[20];
  strcpy(finalinput, inputkey);
  strcat(finalinput,"=");
  strcat(finalinput,inputvalue);
  myFile.write(finalinput);      //Insert word(Key + '=' + value) into file
  myFile.write('\n');
  strcpy(key[Row],inputkey);     //update the key array
  strcpy(value[Row],inputvalue); //update the value array
}


//Remove key and value from array and file
void removeIni(char inputkey[]){
  int new_position = 0;
  //Remove and replace array with values from the bottom of array
  for (int i = 0; i < Row; i++) {
      if (strstr(key[i], inputkey) != 0) {
          // Do nothing. and continue
          continue;       
      }
      if (new_position != i) {
        strcpy(key[new_position],key[i]);      //Replace the removed values in key array with values one level below 
        strcpy(value[new_position],value[i]);  //Replace the removed values in value array with values one level below 
      }
      new_position++;
   }
   Row--;
   strcpy(key[Row],""); //NUll terminate the previous array row
   strcpy(value[Row],""); //NUll terminate the previous array row 

   //Remove db.txt
   SerialMonitorInterface.println("Removing db.txt.");
   SD.remove(filename);
   if (SD.exists(filename)) {
      SerialMonitorInterface.println("db.txt removed and recreated!!");
   } else {
      SerialMonitorInterface.println("db.txt doesn't exist.");
   }

   //Add new array to newly created text file
   int r = Row; 
   myFile = SD.open(filename, FILE_WRITE);
   if (myFile){
      for (int i = 0; i < r; i++) {
          addIni(key[i],value[i]); //add to file function
      }
      myFile.close();
   }else{
      SerialMonitorInterface.println("File(foradding) not open");
   }
   SerialMonitorInterface.print("Removed key:");
   SerialMonitorInterface.println(inputkey);
}
#endif
// END SDCARD OPERATIONS

//Print all values in key array and value array
void printArray() {
    SerialMonitorInterface.println("Keys:");
    SerialMonitorInterface.println("==================");
   // loop through array's rows
   for ( int i = 0; i < Row; i++ ) {
        SerialMonitorInterface.print(i+1);
        SerialMonitorInterface.print(".");
        SerialMonitorInterface.println(key[i]);
   }
   SerialMonitorInterface.println("Values:");
   SerialMonitorInterface.println("==================");
   // loop through array's rows
   for ( int i = 0; i < Row; i++ ) {
      SerialMonitorInterface.print(i+1);
      SerialMonitorInterface.print(".");
      SerialMonitorInterface.println(value[i]);
   }
}

void printrow() {
   SerialMonitorInterface.print("Number of Rows:");
   SerialMonitorInterface.println(Row);
}

void writeText(){
  display.clearScreen();
  writeTextCustom(key[displayRow], liberationSans_16ptFontInfo, 0xFF, 0xFF, TS_8b_Green, TS_8b_Black);
}

void writeTextCustom(char* text, FONT_INFO font, uint8_t x, uint8_t y, uint8_t fg, uint8_t bg){
  display.setFont(font); //Set Font
  display.fontColor(fg,bg); //sets text and background color
  int width=display.getPrintWidth(text); //get the pixel print width of a string
  if(x == 0xFF && y == 0xFF) // flag default coordinates
    display.setCursor(48-(width/2),25);
  else
    display.setCursor(x,y);
  display.print(text);
}


void buttonLoop() {
  static bool delete_confirmation = false;
  if (display.getButtons(TSButtonUpperRight)){
      if(delete_confirmation)
        delete_confirmation = false;
      else displayRow++;
      if (displayRow == Row){
         displayRow = 0;
      }
      display.clearScreen();
      SerialMonitorInterface.println("Button clicked");
      SerialMonitorInterface.println(displayRow);
      writeText();
      //delay(2000);
  }
  else if (getConnectionState() && display.getButtons(TSButtonLowerRight)){
      if(!delete_confirmation){
        char* warning = "Entering Password!";
        SerialMonitorInterface.println(warning);
        int width=display.getPrintWidth(warning); //get the pixel print width of a string
        writeTextCustom(warning, liberationSans_10ptFontInfo, 48-(width/2), 32, TS_8b_Green, TS_8b_Black);
        delay(50);
        const int len = strlen(value[displayRow]);
        for(int i = 0; i < len; i++){
          pressKey(value[displayRow][i]);
        }
      }
      else delete_confirmation = false;
      writeText();
  }
  else if (display.getButtons(TSButtonLowerLeft)){
    // clear pairings/bondings
    if(delete_confirmation){
      aci_gap_clear_security_database();
      char* warning = "Reboot Required";
      display.clearScreen();
      writeTextCustom(warning, liberationSans_10ptFontInfo, 0xFF, 0xFF, TS_8b_Gray, TS_8b_Black);
    }
    else{
      char* warning = "Clear Pairings?";
      SerialMonitorInterface.println(warning);
      display.clearScreen();
      writeTextCustom(warning, liberationSans_10ptFontInfo, 0xFF, 0xFF, TS_8b_Green, TS_8b_Black);
      // print options
      writeTextCustom("< YES", liberationSans_10ptFontInfo, 4, 42, TS_8b_Gray, TS_8b_Black);
      writeTextCustom("NO >", liberationSans_10ptFontInfo, 64, 42, TS_8b_Gray, TS_8b_Black);
    }
    delete_confirmation = !delete_confirmation;
  }
}
