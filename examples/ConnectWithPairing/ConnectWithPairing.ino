/*
  ConnectWithPairing
  
  An example of how to pair Danfoss ECO 2 eTRV and obtain the encryption key.

  For documentation visit: https://github.com/jarikp/ESP32_DanfossECO2
  
  Copyright (c) 2020 Jarik Poplavski 
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <Arduino.h>
#include <DanfossECO2.h>

// the default button is bound to BOOT button on ESP32 devkit board
const uint8_t buttonPin = 0x00;

DanfossECO2 *eTRV;
bool isConnecting;

void setup()
{
    Serial.begin(115200);

    pinMode(buttonPin, INPUT);

    BLEDevice::init("");

    // PLEASE PROVIDE VALID MAC ADDRESS OF YOUR eTRV
    eTRV = new DanfossECO2("00:00:00:00:00:00");
   
    // PLEASE PROVIDE CORRECT PIN CODE (IF IT'S SET)
    eTRV->usePin(0000);

    Serial.println();
    Serial.println("Ready for pairing with Danfoss ECO, press the button on ESP32...");
}

void loop()
{
    if (!isConnecting && !digitalRead(buttonPin))
    {
        isConnecting = true;
        Serial.println("Thank you! Now press the button on the eTRV device...");
        delay(5000);
        if (eTRV->connectWithPairing())
        {
            Serial.print("Pairing successfull! The key is ");
            for (size_t i = 0; i < 16; i++)
                Serial.printf("%02X ", eTRV->getKey()[i]);
            Serial.println();
            Serial.println(eTRV->toString());
            Serial.println("Please reset ESP32 if you want to repeat pairing.");
        }
        else
        {
            isConnecting = false;
            Serial.println("Pairing failed, please try again! Press the button on ESP32...");
        }
        eTRV->disconnect();
    }
    delay(100);
}