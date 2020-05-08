#include <Arduino.h>
#include <DanfossECO2.h>

const uint8_t secretKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

DanfossECO2 *eTRV;

void setup()
{
    Serial.begin(115200);

    BLEDevice::init("");

    eTRV = new DanfossECO2("00:00:00:00:00:00");
    eTRV->usePin(0000);

    Serial.println("Starting DanfossECO2 temperature monitor loop, reading data every 10 second...");
}

void loop()
{
    if (eTRV->connectWithKey(secretKey))
    {
        Serial.println(eTRV->toString());
        eTRV->disconnect();
    }
    else
    {
        Serial.println("The provided key and/or PIN didn't work, connection to eTRV failed.");
        Serial.println(eTRV->toString());
    }

    delay(10000);
}
