    #define ERA_DEBUG
    #define DEFAULT_MQTT_HOST "mqtt1.eoh.io"
    #define ERA_AUTH_TOKEN "462f91cf-4941-45ed-b00d-b6fdef30cb1a"

    #include <Arduino.h>
    #include <WiFi.h>
    #include <Wire.h>
    #include <ERa.hpp>

    const char ssid[] = "t.anh";
    const char pass[] = "t.anhdzvodich";

    WiFiClient mbTcpClient;
    HardwareSerial mySerial(1);

    // UART STM32
    #define RX 16
    #define TX 17

    // DATA
    int productA = 0;
    int productB = 0;
    int total = 0;

    char recvBuffer[128];
    int idx = 0;
int orderCount = 0;
    //================ ERa =================
    ERA_CONNECTED() {
        Serial.println("ERa connected!");
    }

    ERA_DISCONNECTED() {
        Serial.println("ERa disconnected!");
    }

    // Gửi dữ liệu lên app
    void timerEvent()
    {
        ERa.virtualWrite(V0, productA);
        ERa.virtualWrite(V1, productB);
        ERa.virtualWrite(V2, total);
        ERa.virtualWrite(V3, orderCount);
    }

    //================ UART =================
    void parseUART(char* data)
    {
        if (sscanf(data, "in=%d;out=%d;total=%d;ORDER=%d",
                &productA, &productB, &total, &orderCount) == 4)
        {
            Serial.println("DATA OK");
            Serial.print("A= "); Serial.println(productA);
            Serial.print("B= "); Serial.println(productB);
            Serial.print("T= "); Serial.println(total);
            Serial.print("ORDER = ");Serial.println(orderCount);
        }
        else
        {
            Serial.print("ERR: ");
            Serial.println(data);
        }
    }

    void receive_UART()
    {
        while (mySerial.available())
        {
            char c = mySerial.read();

            if (c == '\n')
            {
                recvBuffer[idx] = '\0';
                parseUART(recvBuffer);
                idx = 0;
            }
            else
            {
                if (idx < sizeof(recvBuffer) - 1)
                    recvBuffer[idx++] = c;
            }
        }
    }

    // SETUP 
    void setup() {
        Serial.begin(9600);

        mySerial.begin(9600, SERIAL_8N1, RX, TX);
        Wire.begin(21, 22);

        ERa.setModbusClient(mbTcpClient);
        ERa.setScanWiFi(true);
        ERa.begin(ssid, pass);

        ERa.addInterval(500L, timerEvent);
    }

    // LOOP 
    void loop() {
        ERa.run();
        receive_UART();
    }

