#pragma once

class Led
{
public:
    static unsigned long zapisCzas;
    static bool led_ostatni_stan;
    static unsigned int itor;

    static void Konfiguracja(bool status)
    {
        //Stały kolor w trakcie trwania konfiguracji
        if (!status)
        {
            digitalWrite(LED_RED, 1);
            digitalWrite(LED_GREEN, 0);
            digitalWrite(LED_BLUE, 0);

            return;
        }

        //Miganie diody co 150ms, potwierdzajace zakończenie konfiguracji
        if (millis() - Led::zapisCzas > 300)
        {
            digitalWrite(LED_GREEN, led_ostatni_stan);
            digitalWrite(LED_BLUE, led_ostatni_stan);
            led_ostatni_stan = !led_ostatni_stan;

            Led::zapisCzas = millis();
        }
    }

    static void LaczenieWiFi(bool statusWiFi)
    {
        if (statusWiFi == false)
        {

            if (millis() - Led::zapisCzas < 5)
                return;

            digitalWrite(LED_RED, 1);
            digitalWrite(LED_GREEN, 1);

            analogWrite(LED_BLUE, Led::itor);
            Serial.println(Led::itor);

            if (Led::led_ostatni_stan == false)
            {
                Led::itor++;
            }

            if (Led::led_ostatni_stan == true)
            {
                Led::itor--;
            }

            if (Led::itor > 255 && Led::led_ostatni_stan == false)
            {
                Led::led_ostatni_stan = true;
            }

            if (Led::itor <= 0 && Led::led_ostatni_stan == true)
            {
                Led::led_ostatni_stan = false;
            }

            Led::zapisCzas = millis();

            return;
        }

        if (millis() - Led::zapisCzas < 5000)
        {
            digitalWrite(LED_GREEN, 0);

            return;
        }

        digitalWrite(LED_GREEN, 1);
    }

    static bool HttpStan(bool server_stan)
    {

        if (server_stan)
        {
            digitalWrite(LED_RED, 1);
            digitalWrite(LED_GREEN, 1);

            if (millis() - Led::zapisCzas < 1500)
            {
                digitalWrite(LED_BLUE, 0);
                return false;
            }

            digitalWrite(LED_BLUE, 1);
            return true;
        }

        if (millis() - Led::zapisCzas < 450)
            return false;

        digitalWrite(LED_BLUE, 1);
        digitalWrite(LED_RED, led_ostatni_stan);
        led_ostatni_stan = !led_ostatni_stan;
        Led::zapisCzas = millis();

        return false;
    }
};

unsigned long Led::zapisCzas = millis();
bool Led::led_ostatni_stan = false;
unsigned int Led::itor = 0;