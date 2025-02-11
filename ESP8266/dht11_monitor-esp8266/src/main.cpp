#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <credentials.h>


// api-rest e endpoints
const char *baseURL = "http://minha-api.com/"

// declaração de objetos
HTTPClient http;

void setup()
{
    // iniciar a serial
    Serial.begin(115200);

    // conectar ao wifi
    WiFi.begin(ssid, password);
    Serial.print("CONECTANDO O ESP8266 AO WIFI...");
    while (WiFi.status != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nESP8266 CONECTADO COM SUCESSO AO WIFI...");
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {

        // faz uma requisição get,
        http.begin(baseURL) // faz um get neste rota...
            int codeStatus = http.GET();

        // verificar o código de resposta
        if (codeStatus > 0)
        {
            String payload == http.getString();
            Serial.println(payload);
        }

        else
        {
            Serial.println("ERRO NA REQUISIÇÃO.")
        }
        http.end();
    }
    else
    {
        Serial.println("ESP8266 PERDEU A CONEXÃO COM A REDE...")
    }

    delay(10000)
}
