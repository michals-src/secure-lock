#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include <VL53L0X.h>

#include <stale.h>
#include <led.h>
#include <tof.h>

// Set to true to reset eeprom before to write something
#define RESET_EEPROM true

#ifndef STASSID
#define STASSID "ESPOdbiornik"
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

void ustawienia_pinow()
{
	pinMode(IR_OUT, INPUT);
	pinMode(LED_VCC, OUTPUT);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
}

/*

void eeprom_conf()
{
	EEPROM.begin(2048);
	Serial.println("");

	Serial.println("");
	Serial.println("Check writing");
	String ssid;
	for (int k = addr_ssid; k < addr_ssid + 20; ++k)
	{
		ssid += char(EEPROM.read(k));
	}
	Serial.print("SSID: ");
	Serial.println(ssid);

	String password;
	for (int l = addr_password; l < addr_password + 20; ++l)
	{
		password += char(EEPROM.read(l));
	}
	Serial.print("PASSWORD: ");
	Serial.println(password);
}

void zapisz_do_eeprom(string wartosc)
{
	String a = wartosc;
}



*/

uint16_t czytaj_z_eeprom()
{

	return 0;
}

void setup()
{

	// put your setup code here, to run once:
	Serial.begin(115200);
	ustawienia_pinow();

	Wire.begin(TOF_SDA, TOF_SCL);
	if (!tofsensor.init())
		Serial.println("Nie mozna zaladowac VL53L0X");

	if (analogRead(A0) > 100)
	{
		tryb_konfiguracji = true;
		return;
	}

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	/* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
	WiFi.mode(WIFI_STA);
	//WiFi.begin(ssid, password);

	digitalWrite(LED_VCC, 1);

	// while (WiFi.status() != WL_CONNECTED)
	// {
	// 	Led::LaczenieWiFi(false);
	// }

	// Serial.println("");
	// Serial.println("WiFi connected");
	// Serial.println("IP address: ");
	// Serial.println(WiFi.localIP());
}

bool abc()
{

	bool httpStan = false;
	uint8_t zasieg_tof = 0;

	WiFiClient client;
	HTTPClient http; //must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)

	Serial.print("connecting to ");
	Serial.print(host);
	Serial.print(':');
	Serial.println(port);

	// Odczytanie wartosci zapisanej odleflosci do obiektu
	uint8_t odczytany_tof = czytaj_z_eeprom();

	if (zasieg_tof > odczytany_tof + 10 || zasieg_tof < odczytany_tof - 10)
	{
		// Odleglosc do obiektu nie zgadza sie z wartoscia zapisana
		// Wyslanie sygnalu do odbiornika

		Serial.print("[HTTP] begin...\n");

		// Wyslanie wiadomosci wylaczajacej przekaznik
		http.begin(client, "http://192.168.8.5/drzwi_otwarte");
		//http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");
	}
	else
	{

		Serial.print("[HTTP] begin...\n");

		// Wyslanie wiadomosci wylaczajacej przekaznik
		//http.begin(client, "http://192.168.8.5/drzwi_zamkniete");
		http.begin(client, "http://192.168.8.5/");
		//http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");
	}

	int httpCode = http.GET();

	if (httpCode > 0)
	{

		httpStan = Led::HttpStan(httpCode == HTTP_CODE_OK);
		// file found at server
		if (httpStan)
		{
			Serial.println("[HTTP] HTTP_CODE_OK.");
			Serial.println("[HTTP] zamknięto połączenie lub zakończenie pliku.");
		}
	}
	else
	{
		httpStan = Led::HttpStan(false);
		Serial.printf("[HTTP] GET... niepowodzenie, error: %s\n", http.errorToString(httpCode).c_str());
	}

	http.end();

	return httpStan;
}

void loop()
{

	// Praca z opźnieniem czasowym 50ms
	if (millis() - timer1 < 10)
		return;

	timer1 = millis();

	//uint16_t zasieg_tof = tofsensor.readRangeSingleMillimeters();
	uint8_t zasieg_tof = 0;

	// Serial.println(String(zasieg_tof));

	if (tryb_konfiguracji)
	{
		Led::Konfiguracja(konfiguracja_zakonczona);

		if (konfiguracja_zakonczona)
			return;

		//zapisz_do_eeprom(String(zasieg_tof));
		//String zasieg_tof_str = String(zasieg_tof);
		//Tof::zapisz(zasieg_tof_str);

		konfiguracja_zakonczona = true;

		return;
	}

	bool httpStan = false;

	// Przejście w stan deepSleep
	// jeżeli odpowiedź serwera ma kod 200 => OK
	if (httpStan)
	{
		digitalWrite(LED_VCC, 0);
		ESP.deepSleep(0, WAKE_RF_DEFAULT);
	}

	if (WiFi.status() == WL_CONNECTED)
	{
		httpStan = abc();
		//Led::LaczenieWiFi(true);
		return;
	}

	//Led::LaczenieWiFi(false);

	//try not to reuse begin(ssid,pwd) as some blogs
	//mention possible corruption of the flash memory
	if (String(ssid) != WiFi.SSID())
	{
		Serial.print("Connecting to ");
		Serial.print(ssid);
		WiFi.begin(ssid, password);

		digitalWrite(LED_BLUE, 1);
		digitalWrite(LED_RED, 0);
	}
	else
	{
		//not supposed to happen with WiFi.persistent(false)
		Serial.print("Connecting using saved credentials");
		WiFi.begin();
	}

	Serial.println("...");
	if (WiFi.waitForConnectResult() == WL_CONNECTED)
	{
		Serial.println("WiFi connected");
	}
	else
	{
		//scan for networks if connection fails
		Serial.println("scan start");
		// WiFi.scanNetworks will return the number of networks found
		int n = WiFi.scanNetworks();
		Serial.println("scan done");
		if (n == 0)
			Serial.println("no networks found");
		else
		{
			Serial.print(n);
			Serial.println(" networks found");
			for (int i = 0; i < n; ++i)
			{
				// Print SSID and RSSI for each network found
				Serial.print(i + 1);
				Serial.print(": ");
				Serial.print(WiFi.SSID(i));
				Serial.print(" (");
				Serial.print(WiFi.RSSI(i));
				Serial.print(")");
				Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
				delay(10);
			}
		}
	}
}