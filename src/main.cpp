#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "Nodemcu-CENTRALA"
#define APPSK "systemwykrywaniaotwieraniadrzwi"
#endif

/**
 * @brief Definicja nazwy punktu dostępu i jego hasła
 * 
 */
const char *ssid = APSSID;
const char *password = APPSK;

/**
 * @brief Definicja klasy ESP8266WebServer, przypisanie portu 80
 * 
 */
ESP8266WebServer server(80);

/**
 * @brief Definicja stałych punktu dostępu
 * 
 * @param local_IP, statyczny adres ip centrali
 * @param gateway, bramka dostępu sieci punktu dostępu
 * @param mask, maska sieci punktu dotępowego
 */
IPAddress local_IP(192, 168, 8, 5);
IPAddress gateway(192, 168, 8, 1);
IPAddress mask(255, 255, 255, 0);

/**
 * @brief Deklaracja zmiennych
 * 
 */
unsigned long czas_pracy;
unsigned char softap_stations_count_last;
unsigned char softap_stations_count;
bool led_ostatni_stan;

bool pwm_dir;
uint16_t itor_pwm;

/**
 * @brief Definicja nazw dla wyjść diody led rgb
 * 
 */
enum DIODA_LED
{
  R_LED = D2,
  G_LED = D3,
  B_LED = D4
};

/**
 * @brief Definicja nazw dla wyjść przekaźnika
 * 
 */
enum PRZEKAZNIK_PINY
{
  PRZEKAZNIK_C1 = D0,
  PRZEKAZNIK_C2 = D1
};

/**
 * @brief Funkcja zapytania serwera /
 * 
 */
void handleRoot()
{
  server.send(200, "text/html", "<h1>Centrala systemu wykrywania otwierania drzwi</h1>");
}

/**
 * @brief Funkcja zapytania serwera /drzwi_otwarte
 * 
 */
void handleDetector_drzwi_otwarte()
{
  server.send(200, "text/html", "<h1>Drzwi są otwarte, rozłączenie przełącznika</h1>");
  digitalWrite(PRZEKAZNIK_C1, HIGH);
  digitalWrite(PRZEKAZNIK_C2, HIGH);

  digitalWrite(G_LED, HIGH);
  digitalWrite(B_LED, HIGH);
  digitalWrite(R_LED, LOW);
}

/**
 * @brief Funkcja zapytania serwera /drzwi_zamknięte
 * 
 */
void handleDetector_drzwi_zamkniete()
{
  server.send(200, "text/html", "<h1>Drzwi są zamknięte, włączenie przełącznika</h1>");
  digitalWrite(PRZEKAZNIK_C1, LOW);
  digitalWrite(PRZEKAZNIK_C2, LOW);

  digitalWrite(R_LED, HIGH);
  digitalWrite(B_LED, HIGH);
  digitalWrite(G_LED, LOW);
}

/**
 * @brief Funkcja dla nieistniejącego zapytania
 * 
 */
void handleDetector_nie_znaleziono()
{
  server.send(400, "text/html", "Błąd zapytania");
}

/**
 * @brief Przypisanie funkcji dla pinów mikrokontrolera
 * 
 */
void ustawienia_pinow()
{
  pinMode(PRZEKAZNIK_C1, OUTPUT);
  pinMode(PRZEKAZNIK_C2, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
}

/**
 * @brief Definicja zapytań serwera
 * 
 */
void http_punktykoncowe()
{
  server.on("/", handleRoot);
  server.on("/drzwi_otwarte", handleDetector_drzwi_otwarte);
  server.on("/drzwi_zamkniete", handleDetector_drzwi_zamkniete);
  server.onNotFound(handleDetector_nie_znaleziono);
}

void setup()
{

  ustawienia_pinow();

  /**
   * @brief Ustawienie wartości domyślnych na pinach
   * 
   */
  digitalWrite(R_LED, HIGH);
  digitalWrite(B_LED, HIGH);
  digitalWrite(G_LED, HIGH);
  digitalWrite(PRZEKAZNIK_C1, LOW);
  digitalWrite(PRZEKAZNIK_C2, LOW);

  Serial.begin(115200);

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
  Serial.print("Konfiguracja punktu dostępu");

  /**
   * @brief Konfiguracja punktu dostępu
   * 
   */
  WiFi.softAPConfig(local_IP, gateway, mask);

  /**
   * @brief Uruchomienie punktu dostępu
   * 
   * @param ssid, nazwa punktu dostępu
   * @param password, hasło dostępu
   * @param kanał, ustawiony na 1
   * @param ukryty, punktu dostępu nie jest publiczny
   */
  WiFi.softAP(ssid, password, 8, true);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP adres: ");
  Serial.println(myIP);

  http_punktykoncowe();
  server.begin();

  Serial.println("HTTP server started");
  //digitalWrite(B_LED, LOW);

  /**
   * @brief Definicja zmiennych
   * 
   */
  czas_pracy = millis();
  softap_stations_count_last = 0;
  led_ostatni_stan = false;
  pwm_dir = false;
  itor_pwm = 0;

  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  Serial.printf("MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void loop()
{
  if (millis() - czas_pracy < 10)
    return;

  // Pobranie wartości połącznonych stacji do centrali
  softap_stations_count = wifi_softap_get_station_num();

  /**
   * @brief Sygnalizacja diodą jeżeli nie ma połączonych clientów
   * 
   */
  if (softap_stations_count == 0)
  {
    // Wzrost wartości pwm w zakresie 0-255
    if (!pwm_dir)
      itor_pwm += 3;

    // Spadek wartości pwm w zakresie 0-255
    if (pwm_dir)
      itor_pwm -= 3;

    digitalWrite(R_LED, HIGH);
    digitalWrite(G_LED, HIGH);
    analogWrite(B_LED, itor_pwm);

    // Określenie, czy sygnał pwm ma rosnąć bądź maleć
    if (itor_pwm >= 255 & !pwm_dir)
      pwm_dir = true;

    if (itor_pwm <= 0 & pwm_dir)
      pwm_dir = false;
  }

  /**
   * @brief Sygnalizacja diodą o nowym cliencie połączonym z centralą
   * 
   */
  if (softap_stations_count_last < softap_stations_count)
  {
    for (uint8_t i = 0; i < 7; i++)
    {
      digitalWrite(B_LED, led_ostatni_stan);
      led_ostatni_stan = !led_ostatni_stan;
      delay(50);
    }

    softap_stations_count_last = softap_stations_count;
    digitalWrite(B_LED, HIGH);
  }

  server.handleClient();
  czas_pracy = millis();
}