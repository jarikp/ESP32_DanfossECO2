/*
  SimpleTemperatureSensor
  
  An example of how to use Danfoss ECO 2 eTRV as simple room temperature sensor.

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

// PLEASE PROVIDE CORRECT KEY DATA 
const uint8_t secretKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

DanfossECO2 *eTRV;

void setup()
{
    Serial.begin(115200);

    BLEDevice::init("");

    // PLEASE PROVIDE VALID MAC ADDRESS OF YOUR eTRV 
    eTRV = new DanfossECO2("00:00:00:00:00:00");

    // PLEASE PROVIDE CORRECT PIN CODE (IF IT'S SET)
    eTRV->usePin(0000);
    eTRV->useKey(secretKey);

    Serial.println("Starting DanfossECO2 temperature monitor loop, reading data every 10 second...");
}

void loop()
{
    if (eTRV->connect())
    {
        Serial.println(eTRV->toString());
        eTRV->disconnect();
    }
    else
    {
        Serial.println("The provided key and/or PIN didn't work, connection to eTRV failed.");
        Serial.println(eTRV->toString());
    }

    delay(30000);
}
