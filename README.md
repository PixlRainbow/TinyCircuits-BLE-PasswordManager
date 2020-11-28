# TinyCircuits BLE Hardware Password Manager
This project was made for the ICT1003 project assignment using TinyCircuits.  
The main code is in the PasswordManager folder.
The src folder contains our modified forks of other github repos.  
## Usage
On the TinyScreen buttons:
- `TSButtonUpperRight` tabs through available account passwords
- `TSButtonLowerRight` will type out the respective password on the connected device. **Only active if a device is actually connected.**
- `TSButtonLowerLeft` will delete all paired devices. **Does not notify previously paired devices that they should pair again, so you will have to delete pairings on the other side manually.**
- `TSButtonUpperLeft` will temporarily enable discoverable mode for 180 seconds. **Only active if not currently connected.**  
***Note:*** if there are no paired devices, the TinyCircuit will remain in constant discoverable mode. If there is at least one device paired, the TinyCircuit will not be discoverable when idle unless this button is pressed.

Over Bluetooth serial:  
BLE Serial packets only permit a maximum of 20 bytes per message. The expected message format is:  
```c
struct msg {
    char command; // 'G' for get or 'S' for set
    char space; // ignored; a single space character
    char username[9]; // right-padded username string. NOT null terminated!
    char password[9]; // right-padded password string. NOT null terminated! Ignored in 'G' operation.
}
```
### GET operation
*Deprecated behaviour:* Retrieves the password for the specified existing account.
### SET operation
Sets the password for the specified existing account.
## Files
### PasswordManager.ino
Contains the setup and main loop
### Database.ino
Contains operations for managing stored user accounts and the main UI and display code
### DummyData.h
Holds predefined user accounts for demo mode.
### StatusDisplay.ino
Extract from TinyCircuits Smart Watch example code. Renders battery level and bluetooth connection status indicators.
### BLESetup.ino
*Extract from TinyCircuits BLE UART example code.*  
Contains functions to set up the ST BLE TinyShield (BlueNRG).
### UART.ino
*Extract from TinyCircuits BLE UART example code.*  
Allows passwords to be configured over a Bluetooth serial connection.
### HID.ino
*Heavily modified based on ST X-CUBE suite example code.*  
Implements the Human Interface Device (HID) Bluetooth service, simulating a keyboard.
### Keyboard_types.h
*Extract from Mbed OS source code.*  
Contains definitions for keyboard button keycode mappings.
### USBHID_Types.h
*Extract from Mbed OS source code.*  
Contains definitions for the HID protocol, common between USB and Bluetooth.
### BLEHID.h
*Extract from Mbed OS source code.*  
Contains definitions for BLE UUIDS and characteristic structures.
