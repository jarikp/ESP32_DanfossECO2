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

    char name[16];
    int batteryLevel;
    float ambientTemperature;

    uint8_t pinCode[4], encryptionKey[16];

    BLEClient *pClient;
    BLEAddress *pAddress;

    void onClientConnected(BLEClient *pclient);
    void onClientDisconnected(BLEClient *pclient);
    void swapEndianness(uint8_t *data, size_t length);
    bool decrypt(uint8_t *data, size_t length);

    class ClientCallbacks : public BLEClientCallbacks
    {
    private:
        DanfossECO2 *pDanfossClient;

    public:
        ClientCallbacks(DanfossECO2 *pDanfossClient)
        {
            this->pDanfossClient = pDanfossClient;
        }

        void onConnect(BLEClient *pclient)
        {
            pDanfossClient->onClientConnected(pclient);
        }

        void onDisconnect(BLEClient *pclient)
        {
            pDanfossClient->onClientDisconnected(pclient);
        }
    };
    ClientCallbacks *pClientCallbacks;

public:
    DanfossECO2(const char *macAddress, const uint8_t *keyData, const uint8_t *pinCode = nullptr);
    ~DanfossECO2();

    const char *getName();
    int getBatteryLevel();
    float getAmbientTemperature();

    void connect();
    void disconnect();

    void refreshValues();
};

#endif