/*
  DanfossECO2 library for ESP32 (Arduino) providing interaction with Danfoss ECO2 eTRV devices 
  using Bluetooth Low Energy radio.

  For documentation visit: https://github.com/jarikp/ESP32_DanfossECO2
  
  The library is licensed under the MIT License
  
  Copyright (c) 2020 Jarik Poplavski 
  
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

#ifndef DANFOSSECO2
#define DANFOSSECO2

#include <BLEDevice.h>

class DanfossECO2
{
private:
    static constexpr const char *MAIN_SERVICE_UUID = "10020000-2749-0001-0000-00805F9B042F";
    static constexpr const char *MS_PIN_CHARACTERISTICS_UUID = "10020001-2749-0001-0000-00805F9B042F";
    static constexpr const char *MS_KEY_CHARACTERISTICS_UUID = "1002000B-2749-0001-0000-00805F9B042F";
    static constexpr const char *MS_TEMP_CHARACTERISTICS_UUID = "10020005-2749-0001-0000-00805F9B042F";
    static constexpr const char *MS_NAME_CHARACTERISTICS_UUID = "10020006-2749-0001-0000-00805F9B042F";

    static constexpr const char *BATTERY_SERVICE_UUID = "180F";
    static constexpr const char *BS_LEVEL_CHARACTERISTICS_UUID = "2A19";

    String name;
    int batteryLevel;
    float roomTemperature;
    bool dataReceived;

    uint8_t pinCode[4], encryptionKey[16];

    BLEClient *pClient;
    BLEAddress *pAddress;

    void swapEndianness(uint8_t *data, size_t length);
    bool decrypt(uint8_t *data, size_t length);

public:
    DanfossECO2(const char *macAddress, const uint8_t *keyData, const uint8_t *pinCode = nullptr);
    ~DanfossECO2();

    String getName();
    int getBatteryLevel();
    float getRoomTemperature();

    void connect();
    void disconnect();

    void refreshValues();

    String toString();
};

#endif