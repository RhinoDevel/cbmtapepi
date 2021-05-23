
// Marcel Timm, RhinoDevel, 2021may22

// *** Target: ***
//
// - ESP8266
// - Board (title in Arduino): "NodeMCU 1.0 (ESP-12E Module)"
//
// *** Notes: ***
//
// - At start-up, pins are configured as INPUT (internal pull-ups disabled)
//   (https://arduino-esp8266.readthedocs.io/en/3.0.0/reference.html#digital-io).
//
// - Which PINs to use? See:
//   https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"

#define GPIO_PIN_NR_ACT LED_BUILTIN // == D4 == 2 (inverted, HIGH = off, LOW = on).
//
#define MT_TAPE_GPIO_PIN_NR_READ D3 // == 0 
#define MT_TAPE_GPIO_PIN_NR_SENSE D5 // == 14
//#define MT_TAPE_GPIO_PIN_NR_MOTOR D2 // == 4
//#define MT_TAPE_GPIO_PIN_NR_WRITE D6 // == 12
//
#define MT_GPIO_PIN_NR_LED D1 // == 5

#define s_t_measure 500000 // Microseconds (100us on Pi).

// Short: 2840 Hz
//
// 1 / (2840 Hz) = 352 microseconds
//
// 352 microseconds / 2 = 176 microseconds
//
static uint32_t const micro_short = 176;

ESP8266Timer irq_timer;

void IRAM_ATTR irq_timer_handler()
{
    static const uint32_t act_interval = 500000; // 0.5s
    static const uint32_t act_count = act_interval / s_t_measure;

    static bool act_state = false;
    static uint32_t counter = 0;

    ++counter;

    act_state ^= counter % act_count == 0;
    
    digitalWrite(GPIO_PIN_NR_ACT, act_state);
}

void setup()
{
    pinMode(GPIO_PIN_NR_ACT, OUTPUT);
    digitalWrite(GPIO_PIN_NR_ACT, HIGH); // (inverted)
    
    pinMode(MT_GPIO_PIN_NR_LED, OUTPUT);
    digitalWrite(MT_GPIO_PIN_NR_LED, LOW); 

    pinMode(MT_TAPE_GPIO_PIN_NR_SENSE, OUTPUT);
    digitalWrite(MT_TAPE_GPIO_PIN_NR_SENSE, LOW);
    //
    // (inverted, because circuit inverts signal to CBM)

    pinMode(MT_TAPE_GPIO_PIN_NR_READ, OUTPUT);
    digitalWrite(MT_TAPE_GPIO_PIN_NR_READ, LOW);
    //
    // (inverted, because circuit inverts signal to CBM)

    irq_timer.attachInterruptInterval(s_t_measure, irq_timer_handler);
}

void loop()
{
    while(true)
    {
        digitalWrite(MT_TAPE_GPIO_PIN_NR_READ, HIGH); // Inverted by circuit.
        delayMicroseconds(micro_short);
        digitalWrite(MT_TAPE_GPIO_PIN_NR_READ, LOW); // Inverted by circuit.
        delayMicroseconds(micro_short);
        wdt_reset(); // (feeds software watchdog which also takes care of hardware watchdog)
    }
}
