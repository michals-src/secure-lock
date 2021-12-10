#pragma once

#include <Arduino.h>
#include <stale.h>

class Led
{
private:
    static unsigned long zapisCzas;
    static bool led_ostatni_stan;

public:
    unsigned long zapisCzas = millis();
    bool led_ostatni_stan = false;

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
        if (millis() - zapisCzas > 150)
        {
            digitalWrite(LED_GREEN, led_ostatni_stan);
            digitalWrite(LED_BLUE, led_ostatni_stan);
            led_ostatni_stan = !led_ostatni_stan;
        }

        zapisCzas = millis();
    }

    static void LaczenieWiFi(bool statusWiFi)
    {
        if (!statusWiFi)
        {
            digitalWrite(LED_RED, 1);
            digitalWrite(LED_GREEN, 1);

            for (uint16_t i = 1023; i >= 0; i--)
            {
                if (statusWiFi)
                    break;
                if (millis() - zapisCzas > 1)
                {
                    analogWrite(LED_BLUE, i);
                    zapisCzas = millis();
                }
            }
            for (uint16_t i = 0; i < 1024; i++)
            {
                if (statusWiFi)
                    break;
                if (millis() - zapisCzas > 1)
                {
                    analogWrite(LED_BLUE, i);
                    zapisCzas = millis();
                }
            }

            return;
        }

        if (millis() - zapisCzas < 5000)
        {
            digitalWrite(LED_GREEN, 0);
            digitalWrite(LED_BLUE, 1);
            zapisCzas = millis();
            return;
        }

        digitalWrite(LED_GREEN, 1);

        zapisCzas = millis();
    }

    static void NiskiPoziomBaterii(uint16_t min_wartosc, uint16_t aktualna_wartosc)
    {
        if (aktualna_wartosc > min_wartosc)
            return;

        if (millis() - zapisCzas > 3000)
        {
            digitalWrite(LED_RED, led_ostatni_stan);
            zapisCzas = millis();
            led_ostatni_stan = !led_ostatni_stan;
        }
    }
};