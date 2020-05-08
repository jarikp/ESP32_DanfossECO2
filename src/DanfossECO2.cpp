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
#include <BLEDevice.h>
#include <xxtea-lib.h>
#include "DanfossECO2.h"

DanfossECO2::DanfossECO2(const char *macAddress)
{
    pAddress = new BLEAddress(std::string(macAddress));
    pClient = BLEDevice::createClient();
    resetConnectionState();
    usePin(0000);
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

const uint8_t *DanfossECO2::getKey()
{
    return encryptionKey;
}

bool DanfossECO2::isConnected()
{
    return isConnectedWithDataAccess;
}

void DanfossECO2::usePin(int pin)
{
    pinCode[0] = 0;
    pinCode[1] = 0;
    pinCode[2] = 0;
    pinCode[3] = 0;
}

bool DanfossECO2::connectWithKey(const uint8_t *key)
{
    memcpy(encryptionKey, key, 16);

    if (!pClient->isConnected())
        pClient->connect(*pAddress, BLE_ADDR_TYPE_PUBLIC);

    if (setCharacteristicValue(MAIN_SERVICE_UUID, MS_PIN_CHARACTERISTICS_UUID, pinCode, 4))
    {
        return refreshValues();
    }

    return false;
}

bool DanfossECO2::refreshValues()
{
    int valueLength;
    uint8_t valueBuffer[20];

    valueLength = getCharacteristicValue(MAIN_SERVICE_UUID, MS_NAME_CHARACTERISTICS_UUID, valueBuffer);
    if (valueLength > 0 && decrypt(valueBuffer, valueLength))
    {
        name = String((char *)valueBuffer);
        memset(valueBuffer, 0, sizeof(valueBuffer));
    }
    else
    {
        resetConnectionState();
        return false;
    }

    valueLength = getCharacteristicValue(MAIN_SERVICE_UUID, MS_TEMP_CHARACTERISTICS_UUID, valueBuffer);
    if (valueLength > 0 && decrypt(valueBuffer, valueLength))
    {
        roomTemperature = ((float)valueBuffer[1]) / 2.0F;
        memset(valueBuffer, 0, sizeof(valueBuffer));
    }
    else
    {
        resetConnectionState();
        return false;
    }

    valueLength = getCharacteristicValue(BATTERY_SERVICE_UUID, BS_LEVEL_CHARACTERISTICS_UUID, valueBuffer);
    if (valueLength > 0)
    {
        batteryLevel = ((uint8_t)valueBuffer[0]);
        memset(valueBuffer, 0, sizeof(valueBuffer));
    }
    else
    {
        resetConnectionState();
        return false;
    }

    isConnectedWithDataAccess = true;
    return true;
}

void DanfossECO2::disconnect()
{
    resetConnectionState();
    pClient->disconnect();
    while (pClient->isConnected())
        delay(100);
}

String DanfossECO2::toString()
{
    char output[200];
    if (!pClient->isConnected())
        sprintf(output, "Device '%s' is disconnected.", pAddress->toString().c_str());
    else
    {
        if (!isConnectedWithDataAccess)
            sprintf(output, "Device '%s' is reacheable, but no data available. Please check PIN code and ecryption key.", pAddress->toString().c_str());
        else
            sprintf(output, "Device '%s (%s)' is reporting room temperature %.1fC and remaining battery level is %u%%.", name.c_str(), pAddress->toString().c_str(), roomTemperature, batteryLevel);
    }
    return output;
}

int DanfossECO2::getCharacteristicValue(const char *service_uuid, const char *characteristic_uuid, uint8_t *buffer)
{
    if (pClient->isConnected())
    {
        BLERemoteService *pService = pClient->getService(service_uuid);
        if (pService != nullptr)
        {
            BLERemoteCharacteristic *pCharacteristic = pService->getCharacteristic(characteristic_uuid);
            if (pCharacteristic != nullptr && pCharacteristic->canRead())
            {
                std::string valueBytes = pCharacteristic->readValue();
                int valueLength = valueBytes.length();
                if (valueLength > 0)
                {
                    memcpy(buffer, valueBytes.c_str(), valueLength);
                    return valueLength;
                }
            }
        }
    }
    return -1;
}

bool DanfossECO2::setCharacteristicValue(const char *service_uuid, const char *characteristic_uuid, uint8_t *buffer, size_t length)
{
    if (pClient->isConnected())
    {
        BLERemoteService *pService = pClient->getService(service_uuid);
        if (pService != nullptr)
        {
            BLERemoteCharacteristic *pCharacteristic = pService->getCharacteristic(characteristic_uuid);
            if (pCharacteristic != nullptr && pCharacteristic->canWrite())
            {
                pCharacteristic->writeValue(buffer, length, true);
                return true;
            }
        }
    }
    return false;
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

void DanfossECO2::resetConnectionState()
{
    isConnectedWithDataAccess = false;
    roomTemperature = 0;
    batteryLevel = 0;
    name = "";
}