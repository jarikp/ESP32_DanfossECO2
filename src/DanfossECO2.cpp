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

#include <Arduino.h>
#include <xxtea-lib.h>
#include <BLEDevice.h>
#include "DanfossECO2.h"

DanfossECO2::DanfossECO2(const char *macAddress, const uint8_t *keyData, const uint8_t *pinCode)
{
    memcpy(encryptionKey, keyData, 16);

    pAddress = new BLEAddress(std::string(macAddress));
    pClient = BLEDevice::createClient();
    roomTemperature = 0;
    batteryLevel = 0;
    name = "";
}

DanfossECO2::~DanfossECO2()
{
    disconnect();
    delete pClient;
    delete pAddress;
}

String DanfossECO2::getName()
{
    return name;
}

int DanfossECO2::getBatteryLevel()
{
    return batteryLevel;
}

float DanfossECO2::getRoomTemperature()
{
    return roomTemperature;
}

void DanfossECO2::refreshValues()
{
    BLERemoteService *pMainService = pClient->getService(this->MAIN_SERVICE_UUID);

    BLERemoteCharacteristic *pNameCharacteristic = pMainService->getCharacteristic(this->MS_NAME_CHARACTERISTICS_UUID);
    if (pNameCharacteristic != nullptr && pNameCharacteristic->canRead())
    {
        std::string nameEncryted = pNameCharacteristic->readValue();
        uint8_t data[nameEncryted.length()];
        memcpy(data, nameEncryted.c_str(), nameEncryted.length());
        decrypt(data, nameEncryted.length());
        name = String((char *)data);
    }

    BLERemoteCharacteristic *pTemperatureCharacteristic = pMainService->getCharacteristic(this->MS_TEMP_CHARACTERISTICS_UUID);
    if (pTemperatureCharacteristic != nullptr && pTemperatureCharacteristic->canRead())
    {
        std::string tempEncryted = pTemperatureCharacteristic->readValue();
        uint8_t data[tempEncryted.length()];
        memcpy(data, tempEncryted.c_str(), tempEncryted.length());
        decrypt(data, tempEncryted.length());
        roomTemperature = ((float)data[1]) / 2.0F;
    }

    BLERemoteService *pBatteryService = pClient->getService(this->BATTERY_SERVICE_UUID);

    BLERemoteCharacteristic *pBatteryLevelCharacteristic = pBatteryService->getCharacteristic(this->BS_LEVEL_CHARACTERISTICS_UUID);
    if (pBatteryLevelCharacteristic != nullptr && pBatteryLevelCharacteristic->canRead())
    {
        batteryLevel = pBatteryLevelCharacteristic->readUInt8();
    }

    dataReceived = true;
}

void DanfossECO2::connect()
{
    pClient->connect(*pAddress, BLE_ADDR_TYPE_PUBLIC);
    BLERemoteService *pMainService = pClient->getService(this->MAIN_SERVICE_UUID);

    BLERemoteCharacteristic *pPinCharacteristic = pMainService->getCharacteristic(this->MS_PIN_CHARACTERISTICS_UUID);
    if (pPinCharacteristic != nullptr && pPinCharacteristic->canWrite())
    {
        pPinCharacteristic->writeValue(pinCode, 4, true);
    }

    this->refreshValues();
}

void DanfossECO2::disconnect()
{
    pClient->disconnect();
    while (pClient->isConnected())
        delay(100);

    dataReceived = false;
    roomTemperature = 0;
    batteryLevel = 0;
    name = "";
}

String DanfossECO2::toString()
{
    char output[200];
    if (!pClient->isConnected())
        sprintf(output, "Device '%s' is disconnected.", pAddress->toString().c_str());
    else
    {
        if (!dataReceived)
            sprintf(output, "Device '%s' is connected, but no data available. Please check PIN code and ecryption key.", pAddress->toString().c_str());
        else
            sprintf(output, "Device '%s (%s)' is reporting room temperature %.1fC and remaining battery level is %u%%.", name.c_str(), pAddress->toString().c_str(), roomTemperature, batteryLevel);
    }
    return output;
}

bool DanfossECO2::decrypt(uint8_t *data, size_t length)
{
    if (xxtea_setup_key(encryptionKey, 16) != XXTEA_STATUS_SUCCESS)
        return false;

    swapEndianness(data, length);

    if (xxtea_decrypt(data, length) != XXTEA_STATUS_SUCCESS)
        return false;

    swapEndianness(data, length);

    return true;
}

void DanfossECO2::swapEndianness(uint8_t *data, size_t length)
{
    uint8_t buffer[length];
    for (int i = 0; i < length; i++)
    {
        buffer[i] = data[((i / 4) * 4) + (3 - (i % 4))];
    }
    memcpy(data, buffer, length);
}