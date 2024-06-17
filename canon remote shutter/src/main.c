#include <main.h>
#include <stm/stm8s.h>

#define PHOTO_PORT GPIOD
#define SHUTTER_PIN GPIO_PIN_5
#define FOCUS_PIN GPIO_PIN_6

#define NEXT_BTN GPIO_PIN_1
#define UP_BTN GPIO_PIN_2
#define DOWN_BTN GPIO_PIN_3

void rx_action(void) // will not compile without this event definition
{
    char c = UART1_ReceiveData8();
}

// External interrupt handler
INTERRUPT_HANDLER(EXTI_PORTD_IRQHandler, 6) {
}
void shutter(){
    GPIO_WriteLow(PHOTO_PORT, FOCUS_PIN);
    delay_ms(2000);
    GPIO_WriteHigh(PHOTO_PORT, FOCUS_PIN);
    GPIO_WriteLow(PHOTO_PORT, SHUTTER_PIN);
    delay_ms(500)
    GPIO_WriteHigh(PHOTO_PORT, SHUTTER_PIN);
    delay_ms(500);
    GPIO_WriteLow(PHOTO_PORT, SHUTTER_PIN);
    delay_ms(500);
    GPIO_WriteHigh(PHOTO_PORT, SHUTTER_PIN);
}
int main(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); //Set CLK

    GPIO_Init(PHOTO_PORT, SHUTTER_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(PHOTO_PORT, FOCUS_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

    GPIO_Init(PHOTO_PORT, NEXT_BTN, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(PHOTO_PORT, UP_BTN, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(PHOTO_PORT, DOWN_BTN, GPIO_MODE_IN_PU_NO_IT);

    GPIO_WriteHigh(PHOTO_PORT, SHUTTER_PIN);
    GPIO_WriteHigh(PHOTO_PORT, FOCUS_PIN);

    init_milis();
    init_uart1();

    bool running = FALSE;
    uint32_t shutter_cycle = 60000; //1min
    uint32_t cycle_count = 10; //1 frames
    uint32_t current_cycle = 0;
    uint32_t timestamp = 0;
    uint8_t down_down = 0;
    uint8_t up_down = 0;
    uint8_t next_down = 0;
    uint8_t settings = 0;
    while(1) {
        if(running){
            if(GPIO_ReadInputPin(PHOTO_PORT, NEXT_BTN) == 0){
                running = FALSE;
            }
            if(milis() - timestamp >= shutter_cycle){
                shutter();
            }
            if(current_cycle >= cycle_count){
                current_cycle = 0;
                running = FALSE;
            }
            current_cycle++;
            continue;
        }
        if(GPIO_ReadInputPin(PHOTO_PORT, DOWN_BTN) == 0){
            if(down_down == 0) {
                if(settings == 0) {
                    if(shutter_cycle > 1000) shutter_cycle -= 1000;
                }else if(settings == 1) {
                    if(shutter_cycle > 1) shutter_cycle -= 1;
                }else{
                    while(GPIO_ReadInputPin(PHOTO_PORT, DOWN_BTN) == 0);
                    running = TRUE;
                    continue;
                }
            }
            down_down = 1;
        }else down_down = 0;
        if(GPIO_ReadInputPin(PHOTO_PORT, UP_BTN) == 0){
            if(up_down == 0) {
                if(settings == 0) {
                    shutter_cycle += 1000;
                }else if(settings == 1) {
                    shutter_cycle += 1;
                }
            }
            up_down = 1;
        }else up_down = 0;
        if(GPIO_ReadInputPin(PHOTO_PORT, NEXT_BTN) == 0){
            if(next_down == 0) {
                settings++;
                if(settings > 2) settings = 0;
            }
            next_down = 1;
        }else next_down = 0;
    }
}
/*-------------------------------  Assert -----------------------------------*/
#include "stm/__assert__.h"