/*
MIT License

Copyright (c) 2021 Satoru Sato

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <M5Atom.h>
#include <hiduniversal.h>
#include <NimBLEDevice.h>
#include "HIDKeyboardTypes.h"
#include "HIDTypes.h"
#include <EEPROM.h>

#define HID_KEYBOARD 0x03C1

// Report ID
#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02
#define MOUSE_ID 0x03

static const uint8_t reportMap[] = {
  USAGE_PAGE(1),      0x01,          // USAGE_PAGE (Generic Desktop)
  // ------------------------------------------------- Keyboard
  USAGE(1),           0x06,          // USAGE (Keyboard)
  COLLECTION(1),      0x01,          // COLLECTION (Application) Start Keyboard Collection
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  REPORT_ID(1),       KEYBOARD_ID,   //   REPORT_ID (1) Report ID = 1 (Keyboard)
  // ------------------------------------------------- Modifier
  USAGE_MINIMUM(1),   0xE0,          //   USAGE_MINIMUM (0xE0) Left Control
  USAGE_MAXIMUM(1),   0xE7,          //   USAGE_MAXIMUM (0xE7) Right GUI
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1) 8 bits (Modifier)
  REPORT_COUNT(1),    0x08,          //   REPORT_COUNT (8)
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // ------------------------------------------------- Reserved
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE (8) 1 byte (Reserved)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1)
  HIDINPUT(1),        0x01,          //   INPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // ------------------------------------------------- LEDs
  USAGE_PAGE(1),      0x08,          //   USAGE_PAGE (LEDs)
  REPORT_ID(1),       KEYBOARD_ID,   //   REPORT_ID (1) Report ID = 1 (Keyboard)
  USAGE_MINIMUM(1),   0x01,          //   USAGE_MINIMUM (0x01) Num Lock
  USAGE_MAXIMUM(1),   0x05,          //   USAGE_MAXIMUM (0x05) Kana
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1) 5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
  REPORT_COUNT(1),    0x05,          //   REPORT_COUNT (5)
  HIDOUTPUT(1),       0x02,          //   OUTPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  // ------------------------------------------------- 
  REPORT_SIZE(1),     0x03,          //   REPORT_SIZE (3) 3 bits (Padding)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1)
  HIDOUTPUT(1),       0x03,          //   OUTPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  // ------------------------------------------------- Keys
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0x00,          //   USAGE_MINIMUM (0)
  USAGE_MAXIMUM(1),   0x65,          //   USAGE_MAXIMUM (0x65)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM(0)
  LOGICAL_MAXIMUM(1), 0x65,          //   LOGICAL_MAXIMUM(0x65) ; 101 keys
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE(8) 6 bytes (Keys)
  REPORT_COUNT(1),    0x06,          //   REPORT_COUNT (6)
  HIDINPUT(1),        0x00,          //   INPUT (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // ------------------------------------------------- 
  END_COLLECTION(0),                 // END_COLLECTION
  // ------------------------------------------------- Media Keys
  USAGE_PAGE(1),      0x0C,          // USAGE_PAGE (Consumer)
  USAGE(1),           0x01,          // USAGE (Consumer Control)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  USAGE_PAGE(1),      0x0C,          //   USAGE_PAGE (Consumer)
  REPORT_ID(1),       MEDIA_KEYS_ID, //   REPORT_ID (2) Report ID = 2 (Media Keys)
  USAGE(1),           0xB5,          //   USAGE (Scan Next Track)     ; bit 0: 1
  USAGE(1),           0xB6,          //   USAGE (Scan Previous Track) ; bit 1: 2
  USAGE(1),           0xB7,          //   USAGE (Stop)                ; bit 2: 4
  USAGE(1),           0xCD,          //   USAGE (Play/Pause)          ; bit 3: 8
  USAGE(1),           0xE2,          //   USAGE (Mute)                ; bit 4: 16
  USAGE(1),           0xE9,          //   USAGE (Volume Increment)    ; bit 5: 32
  USAGE(1),           0xEA,          //   USAGE (Volume Decrement)    ; bit 6: 64
  USAGE(1),           0xB8,          //   Usage (Eject)               ; bit 7: 128
  USAGE(1),           0x70,          //   Usage (Brightness Down)     ; bit 0: 1
  USAGE(1),           0x6F,          //   Usage (Brightness Up)       ; bit 1: 2
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1) 2 bytes (Media Keys)
  REPORT_COUNT(1),    0x10,          //   REPORT_COUNT (16)
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // ------------------------------------------------- 
  END_COLLECTION(0),                 // END_COLLECTION
  // ------------------------------------------------- 
  USAGE_PAGE(1),      0x01,          // USAGE_PAGE (Generic Desktop)
  USAGE(1),           0x02,          // USAGE (Mouse)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  USAGE(1),           0x01,          //   USAGE (Pointer)
  COLLECTION(1),      0x00,          //   COLLECTION (Physical)
  REPORT_ID(1),       MOUSE_ID,      //     REPORT_ID (1) Report ID = 3 (Mouse Keys)
  // ------------------------------------------------- 5 Buttons
  USAGE_PAGE(1),      0x09,          //     USAGE_PAGE (Button)
  USAGE_MINIMUM(1),   0x01,          //     USAGE_MINIMUM (Button 1)
  USAGE_MAXIMUM(1),   0x05,          //     USAGE_MAXIMUM (Button 5)
  LOGICAL_MINIMUM(1), 0x00,          //     LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //     LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),     0x01,          //     REPORT_SIZE (1) 5 bits (Buttons)
  REPORT_COUNT(1),    0x05,          //     REPORT_COUNT (5)
  HIDINPUT(1),        0x02,          //     INPUT (Data, Variable, Absolute)
  // ------------------------------------------------- Padding
  REPORT_SIZE(1),     0x03,          //     REPORT_SIZE (3) 3 bits (Padding)
  REPORT_COUNT(1),    0x01,          //     REPORT_COUNT (1)
  HIDINPUT(1),        0x03,          //     INPUT (Constant, Variable, Absolute)
  // ------------------------------------------------- X, Y, Wheel
  USAGE_PAGE(1),      0x01,          //     USAGE_PAGE (Generic Desktop)
  USAGE(1),           0x30,          //     USAGE (X)
  USAGE(1),           0x31,          //     USAGE (Y)
  USAGE(1),           0x38,          //     USAGE (Wheel)
  LOGICAL_MINIMUM(1), 0x81,          //     LOGICAL_MINIMUM (-127)
  LOGICAL_MAXIMUM(1), 0x7f,          //     LOGICAL_MAXIMUM (127)
  REPORT_SIZE(1),     0x08,          //     REPORT_SIZE (8) 3 bytes (X, Y, Wheel)
  REPORT_COUNT(1),    0x03,          //     REPORT_COUNT (3)
  HIDINPUT(1),        0x06,          //     INPUT (Data, Variable, Relative)
  // ------------------------------------------------- Pan
  USAGE_PAGE(1),       0x0c,         //     USAGE PAGE (Consumer)
  USAGE(2),            0x38, 0x02,   //     USAGE (AC Pan)
  LOGICAL_MINIMUM(1),  0x81,         //     LOGICAL_MINIMUM (-127)
  LOGICAL_MAXIMUM(1),  0x7f,         //     LOGICAL_MAXIMUM (127)
  REPORT_SIZE(1),      0x08,         //     REPORT_SIZE (8) 1 byte (Pan)
  REPORT_COUNT(1),     0x01,         //     REPORT_COUNT (1)
  HIDINPUT(1),         0x06,         //     INPUT (Data, Var, Rel)
  END_COLLECTION(0),                 //   END_COLLECTION
  END_COLLECTION(0)                  // END_COLLECTION
};

static int target_connection = 0;
static uint16_t target_handle;
static boolean connection_established = false;
static char bluetooth_name[4][15] = {
  "Atom Preonic 1",
  "Atom Preonic 2",
  "Atom Preonic 3",
  "Atom Preonic 4"
};
static uint8_t org_mac[6] = {0};

static NimBLEServer* pServer;
static NimBLECharacteristic* pReport1;
static NimBLECharacteristic* pReport2;
static NimBLECharacteristic* pReport3;
static NimBLECharacteristic* pReport4;

class ServerCallbacks: public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer) {
  };
  
  void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
    target_handle = desc->conn_handle;
    connection_established = true;
    pServer->updateConnParams(desc->conn_handle, 0x10, 0x20, 0, 600);
  };
  
  void onDisconnect(NimBLEServer* pServer) {
    if (!connection_established) {
      NimBLEDevice::startAdvertising();
    }
    connection_established = false;
  };

  void onAuthenticationComplete(ble_gap_conn_desc* desc) {
    if (!desc->sec_state.encrypted) {
      Serial.println("*** Encrypt connection failed - disconnecting client ***");
      return;
    }
  };
};

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic) {
  };
};

static CharacteristicCallbacks chrCallbacks;

void bluetooth_deinit(void) {
  NimBLEDevice::deinit(true);
}

void bluetooth_init(int connection) {
  EEPROM.put(0, connection);
  EEPROM.commit();
  uint8_t new_mac[6];
  for (int i = 0; i < 6; i++) {
    new_mac[i] = org_mac[i];
  }
  new_mac[5] += (4 * target_connection);
  esp_base_mac_addr_set(new_mac);
  NimBLEDevice::init(bluetooth_name[target_connection]);
  NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND);
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // DeviceInfo Service
  NimBLEService* pDeviceInfoService = pServer->createService("180A");

  // DeviceInfo Service - pnp
  NimBLECharacteristic* pPnpCharacteristic = pDeviceInfoService->createCharacteristic("2A50", NIMBLE_PROPERTY::READ);
  uint8_t sig = 0x02;
  uint16_t vid = 0xFFED; // OLKB
  uint16_t pid = 0x6061; // Preonic
  uint16_t version = 0x0001; 
  uint8_t pnp[] = {sig, (uint8_t) (vid >> 8), (uint8_t) vid, (uint8_t) (pid >> 8), (uint8_t) pid, (uint8_t) (version >> 8), (uint8_t) version };
  pPnpCharacteristic->setValue(pnp, sizeof(pnp));

  // DeviceInfo Service - Manufacturer
  NimBLECharacteristic* pManufacturerCharacteristic = pDeviceInfoService->createCharacteristic("2A29", NIMBLE_PROPERTY::READ);
  pManufacturerCharacteristic->setValue("OLKB");

  // HID Service
  Serial.println("NimBLEDevice createService");
  NimBLEService* pHidService = pServer->createService(NimBLEUUID("1812"), 40);

  // HID Service - HID Information
  NimBLECharacteristic* pHidInfoCharacteristic = pHidService->createCharacteristic("2A4A", NIMBLE_PROPERTY::READ);
  uint8_t country = 0x00;
  uint8_t flags = 0x01;
  uint8_t info[] = {0x11, 0x1, country, flags};                                              
  pHidInfoCharacteristic->setValue(info, sizeof(info));

  // HID Service - Report Map
  NimBLECharacteristic* pReportMapCharacteristic = pHidService->createCharacteristic("2A4B", NIMBLE_PROPERTY::READ);
  pReportMapCharacteristic->setValue((uint8_t *)reportMap, sizeof(reportMap));

  // HID Service - HID Control Point
  pHidService->createCharacteristic("2A4C", NIMBLE_PROPERTY::WRITE_NR);

  // HID Service - Protocol Mode
  NimBLECharacteristic* pProtocolModeCharacteristic = pHidService->createCharacteristic("2A4E", NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::READ);
  const uint8_t pMode[] = {0x01}; // 0: Boot Protocol 1: Rport Protocol
  pProtocolModeCharacteristic->setValue((uint8_t *)pMode, 1);                    

  // HID Service - Report 1
  NimBLECharacteristic* pInputCharacteristic1 = pHidService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC);                                      
  pReport1 = pInputCharacteristic1;

  // Report Descriptor 1
  NimBLEDescriptor* pDesc1 = pInputCharacteristic1->createDescriptor("2908", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC, 20);
  uint8_t desc1_val[] = {KEYBOARD_ID, 0x01}; // Report ID 1 Input
  pDesc1->setValue((uint8_t*)desc1_val, 2);

  // HID Service - Report 2
  NimBLECharacteristic* pInputCharacteristic2 = pHidService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC);                                      
  pReport2 = pInputCharacteristic2;

  // Report Descriptor 2
  NimBLEDescriptor* pDesc2 = pInputCharacteristic2->createDescriptor("2908", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC, 20);
  uint8_t desc2_val[] = {MEDIA_KEYS_ID, 0x01}; // Report ID 2 Input
  pDesc2->setValue((uint8_t*) desc2_val, 2);

  // HID Service - Report 3
  NimBLECharacteristic* pOutputCharacteristic = pHidService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC);
  pOutputCharacteristic->setCallbacks(&chrCallbacks);
  pReport3 = pOutputCharacteristic;

  // Report Descriptor 3
  NimBLEDescriptor* pDesc3 = pOutputCharacteristic->createDescriptor("2908", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC, 20);
  uint8_t desc3_val[] = {KEYBOARD_ID, 0x02}; // Report ID 1 Output
  pDesc3->setValue((uint8_t*) desc3_val, 2);

  // HID Service - Report 4
  NimBLECharacteristic* pInputCharacteristic4 = pHidService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC);                                      
  pReport4 = pInputCharacteristic4;

  // Report Descriptor 4
  NimBLEDescriptor* pDesc4 = pInputCharacteristic4->createDescriptor("2908", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC, 20);
  uint8_t desc4_val[] = {MOUSE_ID, 0x01}; // Report ID 3 Input
  pDesc4->setValue((uint8_t*) desc4_val, 2);

  pDeviceInfoService->start();
  pHidService->start();
  pServer->advertiseOnDisconnect(true);
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(pHidService->getUUID());
  pAdvertising->setScanResponse(false);
  pAdvertising->start();
}

void change_bluetooth_connection(int connection) {
  target_connection = connection;
  if (connection_established) {
    connection_established = false;
    pServer->disconnect(target_handle);
  }
  bluetooth_deinit();
  bluetooth_init(connection);
}

void sendKey(uint8_t *key) {
  if (pServer->getConnectedCount() && connection_established) {
    pReport1->setValue(key, 8);
    pReport1->notify();
  }
}

void sendMediaKey(uint8_t *mediakey) {
  if (pServer->getConnectedCount() && connection_established) {
    pReport2->setValue(mediakey, 2);
    pReport2->notify();
  }
}

void sendMouseKey(uint8_t *mousekey) {
  if (pServer->getConnectedCount() && connection_established) {
    pReport4->setValue(mousekey, 5);
    pReport4->notify();
  }
}

void setup() {
  EEPROM.begin(4); // 4 bytes EEPROM data
  EEPROM.get(0, target_connection);
  if (target_connection < 0 || 3 < target_connection) {
    target_connection = 0;
  }

  M5.begin();
  esp_efuse_mac_get_default(org_mac);

  Serial1.begin(115200, SERIAL_8N1, 32, 26);
  while (!Serial1) {
    delay(10);
  }

  bluetooth_init(target_connection);
}

boolean convertKey(uint8_t buf[]) {
  // Logicool Flow suport
  static boolean flowflag = false;
  static unsigned long flowstart = 0;

  if (buf[0] == 0x05 && buf[2] == 0x00 && buf[3] == 0x00 && buf[4] == 0x00 && 
      buf[5] == 0x00 && buf[6] == 0x00 && buf[7] == 0x00) {
    // Left Option + Left Ctrl
    flowflag = true;
    flowstart = millis();
  }
  else if (flowflag) {
    if (buf[0] == 0x05 && (buf[2] != 0x00 || buf[3] != 0x00 || buf[4] != 0x00 || 
      buf[5] != 0x00 || buf[6] != 0x00 || buf[7] != 0x00)) {
      // flow flag clear
      flowflag = false;
    }
    else if (buf[0] == 0x00 && buf[2] == 0x00 && buf[3] == 0x00 && buf[4] == 0x00 && 
        buf[5] == 0x00 && buf[6] == 0x00 && buf[7] == 0x00) {
      // release
      flowflag = false;
      if (millis() - flowstart > 500) { // 500ms
        // send release all key
        sendKey(buf);
        if (target_connection == 0) {
          target_connection = 1;
        }
        else {
          target_connection = 0;
        }
        change_bluetooth_connection(target_connection);
        return true;
      }
    }
    else {
      return false;
    }
  }

  // Change Connection
  if (buf[0] == 0x08 && (buf[2] == 0x1e || buf[3] == 0x1e || buf[4] == 0x1e || 
      buf[5] == 0x1e || buf[6] == 0x1e || buf[7] == 0x1e)) {
    // cmd-1
    change_bluetooth_connection(0);
    return true;      
  }
  else if (buf[0] == 0x08 && (buf[2] == 0x1f || buf[3] == 0x1f || buf[4] == 0x1f || 
      buf[5] == 0x1f || buf[6] == 0x1f || buf[7] == 0x1f)) {
    // cmd-2
    change_bluetooth_connection(1);      
    return true;      
  }
  else if (buf[0] == 0x08 && (buf[2] == 0x20 || buf[3] == 0x20 || buf[4] == 0x20 || 
      buf[5] == 0x20 || buf[6] == 0x20 || buf[7] == 0x20)) {
    // cmd-3
    change_bluetooth_connection(2);      
    return true;      
  }
  else if (buf[0] == 0x08 && (buf[2] == 0x21 || buf[3] == 0x21 || buf[4] == 0x21 || 
      buf[5] == 0x21 || buf[6] == 0x21 || buf[7] == 0x21)) {
    // cmd-4
    change_bluetooth_connection(3);      
    return true;      
  }
  return false;      
}

void convertMediaKey(uint8_t orgkey) {
  uint8_t mediakey[2];
  
  if (orgkey == 0xB5) { // Next Track
    mediakey[0] = 0x01;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0xB6) { // Previous Track
    mediakey[0] = 0x02;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0xB7) { // Stop
    mediakey[0] = 0x04;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0xCD) { // Play/Pause
    mediakey[0] = 0x08;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0xE2) { // Mute
    mediakey[0] = 0x10;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0xE9) { // Volume Up
    mediakey[0] = 0x20;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0xEA) { // Volume Down
    mediakey[0] = 0x40;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0xB8) { // Eject
    mediakey[0] = 0x80;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0x70) { // Brightness Down
    mediakey[0] = 0x00;
    mediakey[1] = 0x01;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0x6f) { // Brightness Up
    mediakey[0] = 0x00;
    mediakey[1] = 0x02;
    sendMediaKey(mediakey);
  }
  else if (orgkey == 0x00) { // Release
    mediakey[0] = 0x00;
    mediakey[1] = 0x00;
    sendMediaKey(mediakey);
  }
}

uint8_t getChar(void) {
  while (!Serial1.available()) {
    ;
  }
  uint8_t c;
  c = Serial1.read();
  return c;
}

void loop() {
  uint8_t buf[8] = {0};
  uint8_t mediakey = 0;
  uint8_t mousekey[6] = {0};
  uint8_t c;

  while (Serial1.available()) {
    c = getChar();
    if (c == 0xfd) {
      // start
      c = getChar();
      // key, media ,mouse
      switch (c) {
      case 9: // key
        c = getChar(); // skip
        // modifier + reserved + keycode only 6kro support
        for (int i = 0; i < 8; i++) {
          buf[i] = getChar();
        }
        if (!convertKey(buf)) {
          sendKey(buf);
        }
        break;
      case 3: // media key
        c = getChar(); // skip
        // media key
        mediakey = getChar();
        c = getChar(); // skip (only 1 byte media key support)
        convertMediaKey(mediakey);
        break;
      case 0: // mouse key
        c = getChar(); // skip
        for (int i = 0; i < 6; i++) {
          mousekey[i] = getChar();
        }
        sendMouseKey(mousekey);
        break;
      }
    }
    else {
      Serial.print("Error:");
      Serial.println(c, HEX);
    }
  }

  // Poll interval
  delay(10);
}
