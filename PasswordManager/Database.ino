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
  if(millis() - startTime > 2000){
    buttonLoop();
    startTime = millis();
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
  display.setFont(liberationSans_16ptFontInfo); //Set Font
  int width=display.getPrintWidth(key[displayRow]); //get the pixel print width of a string
  display.setCursor(48-(width/2),25);  //set text cursor position to (x,y)
  display.fontColor(TS_8b_Green,TS_8b_Black); //sets text and background color
  display.print(key[displayRow]);
}


void buttonLoop() {
  if (display.getButtons(TSButtonUpperRight)){
      displayRow++;
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
      char* warning = "Entering Password!";
      SerialMonitorInterface.println(warning);
      int width=display.getPrintWidth(warning); //get the pixel print width of a string
      display.setCursor(48-(width/2),32);  //set text cursor position to (x,y)
      display.print(warning);
      delay(50);
      const int len = strlen(value[displayRow]);
      for(int i = 0; i < len; i++){
        pressKey(value[displayRow][i]);
      }
      writeText();
  }
}
