#include <SPI.h>
#include <STBLE.h>
#include <TinyScreen.h>


//Debug output adds extra flash and memory requirements!
#ifndef BLE_DEBUG
#define BLE_DEBUG true
#endif

//SD card -- Cannot be used together with BLE tinyshield!
#ifndef USE_SDCARD
#define USE_SDCARD false
#endif

#if defined (ARDUINO_ARCH_AVR)
#define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
#define SerialMonitorInterface SerialUSB
#endif
// BLE standard defines a max payload size of 20 bytes
#define MAX_UART_BUFFER 20

uint8_t ble_rx_buffer[MAX_UART_BUFFER+1];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_tx_buffer[MAX_UART_BUFFER+1];
uint8_t ble_tx_buffer_len = 0;
uint8_t ble_connection_state = false;
#define PIPE_UART_OVER_BTLE_UART_TX_TX 0

TinyScreen display = TinyScreen(TinyScreenDefault);
bool disable_buttons = false;

void setup() {
  SerialMonitorInterface.begin(9600);
  #if BLE_DEBUG
  while (!SerialMonitorInterface); //This line will block until a serial monitor is opened with TinyScreen+!
  #else
  delay(250); //in case we need to wait for peripheral startup
  #endif
  BLEsetup();
  setupDisplay();
  setupDB();
  //aci_gap_clear_security_database();
}

void loop() {
  aci_loop();//Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.
  if (ble_rx_buffer_len) {//Check if data is available
    SerialMonitorInterface.print(ble_rx_buffer_len);
    SerialMonitorInterface.print(" : ");
    SerialMonitorInterface.println((char*)ble_rx_buffer);
    if(ble_rx_buffer_len > 11){
      int attempt = 0;
      //char sendBuffer[MAX_UART_BUFFER+1];
      int sendLength = 0;
      switch(toupper(ble_rx_buffer[0])){
        case 'G': // GET
        {
          char* toSend = selectByName((char*)&ble_rx_buffer[2]);
          if(toSend)
            sendLength = strlen(toSend);
          else
            toSend = "";
          while(attempt++ < 3 && Write_UART_TX(toSend, sendLength));
        }
        break;
        case 'S': // SET
        {
          char* toSet = selectByName((char*)&ble_rx_buffer[2]);
          if(toSet)
            strncpy(toSet, (char*)&ble_rx_buffer[11], 9);
        }
        break;
      }
    }
    ble_rx_buffer_len = 0;//clear afer reading
  }
//  if(ble_connection_state){
//    if(display.getButtons(TSButtonUpperLeft) && millis() - startTime > 2000){
//      pressKey('A');
//      startTime = millis();
//    }
//  }
  if(!disable_buttons)
    db_loop();
}

uint8_t getConnectionState(){
  return ble_connection_state;
}
