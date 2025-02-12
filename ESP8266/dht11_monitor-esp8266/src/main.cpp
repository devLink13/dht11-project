#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <array>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

// inclusão de arquivos locais
#include "credentials.h"

// api-rest e endpoints
const char *baseURL = "http://minha-api.com/";

// configuração do sensor
#define DHTPIN D4     // pino do sensor
#define DHTTYPE DHT11 // Tipo de sensor

// definicições gerais
#define PRIMARY_SERVER_NTP "pool.ntp.org"    // servidor principal para o ntp
#define SECUNDARY_SERVER_NTP "time.nist.gov" // servidor secundário para o ntp
#define TIME_ZONE_ms -14400                  // fuso horário de campo grande
#define TIME_ZONE_bsb -10800                 // fuso horário de brasília
#define DEFAULT_INTERVAL_UPDATE 3600000      // se comunica com o server NTP a cada 1 hora
#define DHT_AMOSTRAS 10                      // número de amostragem por leitura do DHT11

// declaração de objetos
HTTPClient http;
WiFiClient wifiClient;
WiFiUDP ntpUDP;

DHT dht(DHTPIN, DHTTYPE);
NTPClient timeClient(ntpUDP, PRIMARY_SERVER_NTP);

// declaração de funções
std::array<float, 5> readDHT11(int amostragem = 10); // função para ler o dht11 (ler tudo)
String ntp_isConnected();
bool ntp_reconnect(bool activate_secundary_server = true, bool force_reboot_ESP = false, int max_try = 10);

void setup()
{

    Serial.begin(115200);                                  // iniciar a serial
    timeClient.begin();                                    // iniciar o ntp client
    timeClient.setTimeOffset(TIME_ZONE_ms);                // modifica o fuso horário.
    timeClient.setUpdateInterval(DEFAULT_INTERVAL_UPDATE); // define o tempo de atualização

    dht.begin(); // iniciar o dht11

    // conectar ao wifi
    WiFi.begin(ssid, password);
    Serial.print("CONECTANDO O ESP8266 AO WIFI...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nESP8266 CONECTADO COM SUCESSO AO WIFI...");

    String ntp_connection = ntp_isConnected();
    if (ntp_connection != "")
    {
        Serial.printf("ESP8266 SINCRONIZADO AO SERVIDOR NTP PRIMÁRIO COM SUCESSO, HORA DA CONEXÃO: %s\n", ntp_connection.c_str());
    }
    else
    {
        Serial.println("CONEXÃO COM O SERVIDOR NTP PRIMÁRIO FALHOU.");
        //ntp_reconnect(); // essa função mantém o código em loop até reconectar ou então reiniciar o esp8266...
        ntp_reconnect(false, true, 10);
    }
}

void loop()
{
    while (WiFi.status() == WL_CONNECTED)
    {
        timeClient.update();                                 // atualiza o ntp
        unsigned long epochTime = timeClient.getEpochTime(); // pega o epoch time
        setTime(epochTime);                                  // passa o epoch para a biblioteca TimeLib

        // pega as datas usando a timelib
        int currentYear = year();
        int currentMonth = month();
        int currentDay = day();
        int currentHour = hour();
        int currentMinute = minute();
        int currentSecond = second();

        // Serial.printf("Data e hora: %04d-%02d-%2d %02d:%2d:%4d\n", currentYear, currentMonth, currentDay, currentHour, currentMinute, currentSecond);

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
        std::array<float, 5> leitura_dht = readDHT11(DHT_AMOSTRAS);
        float temperatureC = leitura_dht[0];
        float temperatureF = leitura_dht[1];
        float humidity = leitura_dht[2];
        float sensacao_termica = leitura_dht[3];
        // float count_amostras = leitura_dht[4]; // ignorar isso

        // formatação do datalog
        char data_log[75];
        // data = "0000-00-00 00:00:00;temperaturaC;temperaturaF;sensacao_termica;humidity"
        const char *data_template = "%04d-%02d-%02d %02d:%02d:%02d;%.2f;%.2f;%.2f;%.0f";
        snprintf(data_log, sizeof(data_log), data_template, currentYear, currentMonth, currentDay, currentHour, currentMinute, currentSecond, temperatureC, temperatureF, sensacao_termica, humidity);

        Serial.println(data_log);
        delay(2000);
    }

    Serial.println("ESP8266 PERDEU A CONEXÃO COM A REDE...");
}

std::array<float, 5> readDHT11(int amostragem)
{

    float media_tempC = 0.0;
    float media_tempF = 0.0;
    float media_humidity = 0.0;
    int count_erros = 0;
    int count_amostras = 0;

    for (int i = 0; i < amostragem; i++)
    {
        while (true)
        { // usar o while para garantir que todas amostras sejam válidas
            float temperature_celsius = dht.readTemperature(false);
            float temperature_fahrenheit = dht.readTemperature(true);
            float humidity = dht.readHumidity();

            // verificar se alguma leitura falhou e tentar novamente
            if (isnan(temperature_celsius) || isnan(temperature_fahrenheit) || isnan(humidity))
            {
                count_erros++; // acrescenta 1 a cada falha.

                if (count_erros == 10)
                {                                // se a amostra falhar 10x...
                    return {0.0, 0.0, 0.0, 0.0}; // retorna um array de zeros, indicando erro.
                }

                continue; // refaz a leitura da mesma amostra pois ela falhou
            }

            media_tempC = media_tempC + temperature_celsius;
            media_tempF = media_tempF + temperature_fahrenheit;
            media_humidity = media_humidity + humidity;
            count_amostras++;
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

String ntp_isConnected()
{
    if (timeClient.update())
    {
        String formattedTime = timeClient.getFormattedTime();
        return formattedTime; // se der certo entrega o horário atual.
    }
    else
    {
        return ""; // string vazia indica falha
    }
}

bool ntp_reconnect(bool activate_secundary_server, bool force_reboot_ESP, int max_try) {
    int reconnect_count = 0;
    Serial.println("INICIANDO RECONEXÃO COM O SERVIDOR NTP.");
    while (!timeClient.update()) {
        timeClient.forceUpdate();
        delay(1000);
        Serial.print(".");
        reconnect_count++;

        if (reconnect_count == max_try) {
            if (activate_secundary_server) {
                timeClient.end();
                timeClient = NTPClient(ntpUDP, SECUNDARY_SERVER_NTP, TIME_ZONE_ms, DEFAULT_INTERVAL_UPDATE);
                timeClient.begin();

                if (timeClient.forceUpdate()) {
                    Serial.println("\nCONECTADO AO SERVIDOR NTP SECUNDÁRIO.");
                    return true;
                }
            }

            if (force_reboot_ESP) {
                Serial.println("MÁXIMO DE TENTATIVAS DE RECONEXÃO COM O SERVIDOR NTP ATINGIDAS, REINICIANDO ESP8266 EM 10s.");
                delay(10000);
                ESP.restart();
            }

            reconnect_count = 0; // Resetar o contador se não reiniciar
        }
    }

    return true;
}