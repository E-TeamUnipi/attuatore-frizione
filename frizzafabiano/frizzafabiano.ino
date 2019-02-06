/*
 * Software sviluppato da Giuseppe Fabiano
 *
 * con la collaborazione di
 *  - Carlo Lupi
 *  - Enes Bajraktari
 *  - Lorenzo Visconti
 *  - Mirko Lazzarini
 *
 * durante una lunghissima notte, in preda al panico.
 *
 *
 * P.S.
 *  Il nome del programma è stato scelto da Mirko (non più) Rasta Lazzarini
 */

#include <SoftwareSerial.h>
#include "RoboClaw.h"

unsigned int POT_MIN;//   130 //140
unsigned int POT_MAX;//   190 //230

SoftwareSerial serial(10, 11);
RoboClaw roboclaw(&serial, 10000);

#define robo_addr 0x80
#define pin 8

#define MAP_POT_MIN 10200
#define MAP_POT_MAX 10800

// treshold, soglia errore
const unsigned int th = 30;
unsigned long duration;
unsigned long input_pot;
int error = 0;
unsigned int abs_err = 0;
int last_pot = 0;

void setup() {
    POT_MIN = analogRead(A1);
    
    // cambiare costante per aumentare o diminuire apertura,
    // 0.29 gradi ogni unità
    POT_MAX = POT_MIN + 100;
    Serial.begin(9600);
    roboclaw.begin(38400);

    pinMode(pin, INPUT);
    pinMode(A1, INPUT);

    roboclaw.ForwardM1(robo_addr, 0);
}

inline
unsigned int map_pot(unsigned int input_pot) {
    input_pot = map(input_pot, POT_MIN, POT_MAX, MAP_POT_MIN, MAP_POT_MAX);
    if (input_pot < MAP_POT_MIN) {
      return MAP_POT_MIN;
    } else if (input_pot > MAP_POT_MAX) {
      return MAP_POT_MAX;
    }
    return input_pot;
}

inline
unsigned int map_duration(unsigned int duration) {
    if (duration < 25) {
      return MAP_POT_MIN;
    } else if (duration > 475) {
      return MAP_POT_MAX;
    }
    return map(duration, 25, 475, MAP_POT_MIN, MAP_POT_MAX);
}

inline
int map_fix(int err) { 
    err = map(error, -600, 600, -127, 110);
    if (err < -127) {
        return -127;
    } else if (err > 110) {
        return 110;
    }
    return err;
}

bool update_status()
{
    input_pot = analogRead(A1);
    input_pot = map_pot(input_pot);

    duration = pulseIn(pin, HIGH);
    duration = map_duration(duration);
    error = duration - input_pot;
    abs_err = abs(error);
    if (abs_err < th) {
      return true;
    }
    return false;
}

inline
void check_limits() {
    if (abs(input_pot - MAP_POT_MIN) < th) {
      roboclaw.ForwardM1(robo_addr, 0);
    }
}

void loop() {
    while (!update_status()) {
        int fix = map_fix(error);

        last_pot = [fix]() {
            last_pot += fix;
            if (last_pot < 0) {
                return 0;
            } else if (last_pot > 110) {
                return 110;
            }
            return last_pot;
        }();

        roboclaw.ForwardM1(robo_addr, last_pot);

        Serial.print("last_pot: ");
        Serial.print(last_pot);
        Serial.print("  errore: ");
        Serial.print(err);
        Serial.print("target: ");
        Serial.print(duration);
        Serial.print("sensor: ");
         Serial.println(input_pot);
    }
    check_limits();
}
