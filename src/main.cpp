#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include <VL53L0X.h>

#include <stale.h>
#include <led.h>
#include <tof.h>

#ifndef STASSID
#define STASSID "Nodemcu-CENTRALA"
#define STAPSK "systemwykrywaniaotwieraniadrzwi"
#endif

VL53L0X tofsensor;

const char *ssid = STASSID;
const char *password = STAPSK;

const char *host = "192.168.8.5";
const uint16_t port = 80;

bool tryb_konfiguracji = false;
bool konfiguracja_zakonczona = false;

unsigned long timer1 = millis();

IPAddress local_IP(192, 168, 8, 6);
IPAddress gateway(192, 168, 8, 1);
IPAddress mask(255, 255, 255, 0);

uint8_t bssid[6] = {0x52, 0x02, 0x91, 0xDA, 0x5F, 0x9b};

void ustawienia_pinow()
{
	pinMode(IR_OUT, INPUT);
	pinMode(LED_VCC, OUTPUT);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
}

bool centrala_komunikacja()
{

	uint16_t zasieg_tof = tofsensor.readRangeSingleMillimeters();
	bool httpStan = false;

	WiFiClient client;
	HTTPClient http; //must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)

	// Odczytanie wartosci zapisanej odleflosci do obiektu
	uint16_t pamiec_wartosc = Tof::odczytaj();
	String endpoint = "http://192.168.8.5/drzwi_otwarte";
	if (zasieg_tof <= pamiec_wartosc + 10 && zasieg_tof >= pamiec_wartosc - 10)
	{
		// Odleglosc do obiektu zgadza sie z wartoscia zapisana
		endpoint = "http://192.168.8.5/drzwi_zamkniete";
	}

	// Wyslanie sygnalu do odbiornika
	http.begin(client, endpoint);
	int httpCode = http.GET();

	if (httpCode > 0)
	{

		httpStan = Led::HttpStan(httpCode == HTTP_CODE_OK);
		// file found at server
		if (!httpStan)
		{
			httpStan = Led::HttpStan(false);
		}
	}
	else
	{
		httpStan = Led::HttpStan(false);
	}

	http.end();

	return httpStan;
}

// uint32_t calculateCRC32(const uint8_t *data, size_t length)
// {
// 	uint32_t crc = 0xffffffff;
// 	while (length--)
// 	{
// 		uint8_t c = *data++;
// 		for (uint32_t i = 0x80; i > 0; i >>= 1)
// 		{
// 			bool bit = crc & 0x80000000;
// 			if (c & i)
// 			{
// 				bit = !bit;
// 			}

// 			crc <<= 1;
// 			if (bit)
// 			{
// 				crc ^= 0x04c11db7;
// 			}
// 		}
// 	}

// 	return crc;
// }

void setup()
{
	Serial.begin(115200);

	ustawienia_pinow();

	Wire.begin(TOF_SDA, TOF_SCL);

	digitalWrite(LED_RED, 1);
	digitalWrite(LED_GREEN, 1);
	digitalWrite(LED_BLUE, 1);
	digitalWrite(LED_VCC, 1);

	while (!tofsensor.init())
	{
		Led::ToF_nieznaleziono(false);
		delay(500);
	}

	if (analogRead(A0) > 100)
	{
		tryb_konfiguracji = true;
		return;
	}

	WiFi.disconnect();
	WiFi.begin(ssid, password, 1, bssid);

	while (WiFi.status() != WL_CONNECTED)
	{
		Led::LaczenieWiFi(false);
	}
}

void loop()
{

	// Praca z opźnieniem czasowym 50ms
	if (millis() - timer1 < 1000)
		return;

	timer1 = millis();

	uint16_t zasieg_tof = tofsensor.readRangeSingleMillimeters();

	if (tryb_konfiguracji)
	{
		Led::Konfiguracja(konfiguracja_zakonczona);

		if (konfiguracja_zakonczona)
		{
			return;
		}

		Tof::zapisz(zasieg_tof);

		konfiguracja_zakonczona = true;

		return;
	}

	bool httpStan = false;

	if (WiFi.status() == WL_CONNECTED)
	{
		httpStan = centrala_komunikacja();
		Led::LaczenieWiFi(true);

		// Przejście w stan deepSleep
		// jeżeli odpowiedź serwera ma status 200
		if (httpStan)
		{
			digitalWrite(LED_VCC, 0);
			ESP.deepSleep(0, WAKE_RF_DEFAULT);
		}
	}
}