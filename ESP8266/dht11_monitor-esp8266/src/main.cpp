#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <array>

// inclusão de arquivos locais
#include "credentials.h"

// api-rest e endpoints
const char* baseURL = "http://minha-api.com/";

// configuração do sensor
#define DHTPIN D4 // pino do sensor
#define DHTTYPE DHT11 // Tipo de sensor

// declaração de objetos
HTTPClient http;
WiFiClient wifiClient;
DHT dht(DHTPIN, DHTTYPE);


// declaração de funções
std::array<float, 5> readDHT11(int amostragem = 10);


void setup()
{
    // iniciar a serial
    Serial.begin(115200);

    // iniciar o dht11
    dht.begin();


    // conectar ao wifi
    WiFi.begin(ssid, password);
    Serial.print("CONECTANDO O ESP8266 AO WIFI...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nESP8266 CONECTADO COM SUCESSO AO WIFI...");
}

void loop()
{
    while (WiFi.status() == WL_CONNECTED)
    {

        // //faz uma requisição get,
        // http.begin(wifiClient, baseURL);
        //     int codeStatus = http.GET(); // faz um get neste rota...

        // verificar o código de resposta
        // if (codeStatus > 0)
        // {
        //     String payload = http.getString();
        //     Serial.println(payload);
        // }

        // else
        // {
        //     Serial.println("ERRO NA REQUISIÇÃO.");
        // }
        // http.end();

        // obter e atualizar as leituras        
        std::array<float, 5> leitura_dht = readDHT11(1000);
        float temperatureC = leitura_dht[0];
        float temperatureF = leitura_dht[1];
        float humidity = leitura_dht[2];
        float sensacao_termica = leitura_dht[3];
        //float count_amostras = leitura_dht[4]; // ignorar isso

        // exibindo os valores lidos
        Serial.println("-------------------------------------");
        Serial.println("DADOS OBTIDOS DO DHT11:");
        Serial.print(" TEMPERATURA: ");
        Serial.print(temperatureC);
        Serial.print("°C, ");
        Serial.print(temperatureF);
        Serial.print("°F");
        Serial.print(" | ");
        Serial.print("HUMIDADE: ");
        Serial.print(humidity);
        Serial.print("%");
        Serial.print(" | ");
        Serial.print("SENSAÇÃO TÉRMICA: ");
        Serial.print(sensacao_termica);
        Serial.println("°C.");
        delay(2000);
    }
   
    Serial.println("ESP8266 PERDEU A CONEXÃO COM A REDE...");
}


std::array<float, 5> readDHT11(int amostragem){
    
    float media_tempC = 0.0;
    float media_tempF = 0.0;
    float media_humidity = 0.0;
    int count_erros = 0;
    int count_amostras = 0;


    for (int i = 0; i < amostragem; i++){
        while (true) { // usar o while para garantir que todas amostras sejam válidas
            float temperature_celsius = dht.readTemperature(false);
            float temperature_fahrenheit = dht.readTemperature(true);
            float humidity = dht.readHumidity();
            
            // verificar se alguma leitura falhou e tentar novamente
            if (isnan(temperature_celsius) || isnan(temperature_fahrenheit) || isnan(humidity)) {
                count_erros ++; // acrescenta 1 a cada falha.
                

                if (count_erros == 10){ // se a amostra falhar 10x...
                    return {0.0, 0.0, 0.0, 0.0}; //retorna um array de zeros, indicando erro.
                }

                continue; // refaz a leitura da mesma amostra pois ela falhou
            }

            media_tempC = media_tempC + temperature_celsius;
            media_tempF = media_tempF + temperature_fahrenheit;
            media_humidity = media_humidity + humidity;
            count_amostras ++;
            break; // se a amostra não falhar, pula para a próxima amostra
        }
    }

    // calculos das leituras
    float leitura_tempC = media_tempC / amostragem;
    float leitura_tempF = media_tempF / amostragem;
    float leitura_humidity = media_humidity / amostragem;

    // calculo da sensacao termica e do ponto de orvalho
    float sensacao_termica = dht.computeHeatIndex(leitura_tempC, leitura_humidity, false); // sensação termica

    // retorna um array
    return {leitura_tempC, leitura_tempF, leitura_humidity, sensacao_termica, static_cast<float>(count_amostras)};
}