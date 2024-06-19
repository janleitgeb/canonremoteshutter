#include <main.h>
#include <stm/stm8s.h>

#define PHOTO_PORT GPIOC
#define SHUTTER_PIN GPIO_PIN_5
#define FOCUS_PIN GPIO_PIN_6

#define NEXT_BTN GPIO_PIN_1
#define UP_BTN GPIO_PIN_2
#define DOWN_BTN GPIO_PIN_3

#define LCD_RS     GPIOB, GPIO_PIN_4
#define LCD_EN     GPIOB, GPIO_PIN_3
#define LCD_DB4    GPIOD, GPIO_PIN_1
#define LCD_DB5    GPIOD, GPIO_PIN_2
#define LCD_DB6    GPIOD, GPIO_PIN_3
#define LCD_DB7    GPIOD, GPIO_PIN_4

#include <lcd.h>

void rx_action(void) // will not compile without this event definition
{
    char c = UART1_ReceiveData8();
}

// External interrupt handler
INTERRUPT_HANDLER(EXTI_PORTD_IRQHandler, 6) {
}
char* itoa(int val, int base){
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}
void shutter(){
    GPIO_WriteLow(PHOTO_PORT, FOCUS_PIN);
    delay_ms(2000);
    GPIO_WriteHigh(PHOTO_PORT, FOCUS_PIN);
    GPIO_WriteLow(PHOTO_PORT, SHUTTER_PIN);
    delay_ms(500);
    GPIO_WriteHigh(PHOTO_PORT, SHUTTER_PIN);
    delay_ms(500);
    GPIO_WriteLow(PHOTO_PORT, SHUTTER_PIN);
    delay_ms(500);
    GPIO_WriteHigh(PHOTO_PORT, SHUTTER_PIN);
}
void init_display(){
    Lcd_Begin();
    Lcd_Clear();
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
    init_display();

    bool running = FALSE;
    uint32_t shutter_cycle = 1000; //1min
    uint32_t cycle_count = 10; //1 frames
    uint32_t current_cycle = 0;
    uint32_t timestamp = 0;
    uint8_t down_down = 0;
    uint8_t up_down = 0;
    uint8_t next_down = 0;
    uint8_t settings = 0;
    bool rewrite = TRUE;
    while(1) {
        if(rewrite){
            Lcd_Begin();
            Lcd_Clear();
            if(running) Lcd_Print_String("Shooting");
            else{
                if(settings == 0) {
                    Lcd_Print_String("Delay ");
                    Lcd_Print_String(itoa(shutter_cycle/1000, 10));
                    Lcd_Print_Char('s');
                }else if(settings == 1) {
                    Lcd_Print_String("Cycles ");
                    Lcd_Print_String(itoa(cycle_count, 10));
                }else{
                    Lcd_Print_String("DOWN to Shoot ");
                }
            }
        }
        rewrite = FALSE;
        if(running){
            if(GPIO_ReadInputPin(PHOTO_PORT, NEXT_BTN) == 0){
                running = FALSE;
                rewrite = TRUE;
                while(GPIO_ReadInputPin(PHOTO_PORT, NEXT_BTN) == 0);
            }
            if(milis() - timestamp >= shutter_cycle){
                shutter();
                current_cycle++;
                timestamp = milis();
            }
            if(current_cycle >= cycle_count){
                current_cycle = 0;
                running = FALSE;
            }
            continue;
        }
        if(GPIO_ReadInputPin(PHOTO_PORT, DOWN_BTN) == 0){
            if(down_down == 0) {
                rewrite = TRUE;
                if(settings == 0) {
                    if(shutter_cycle > 10000) shutter_cycle -= 10000;
                }else if(settings == 1) {
                    if(cycle_count > 1) cycle_count -= 1;
                }else{
                    while(GPIO_ReadInputPin(PHOTO_PORT, DOWN_BTN) == 0);
                    running = TRUE;
                    shutter();
                    continue;
                }
            }
            down_down = 1;
        }else down_down = 0;
        if(GPIO_ReadInputPin(PHOTO_PORT, UP_BTN) == 0){
            if(up_down == 0) {
                rewrite = TRUE;
                if(settings == 0) {
                    shutter_cycle += 10000;
                }else if(settings == 1) {
                    cycle_count += 1;
                }
            }
            up_down = 1;
        }else up_down = 0;
        if(GPIO_ReadInputPin(PHOTO_PORT, NEXT_BTN) == 0){
            if(next_down == 0) {
                rewrite = TRUE;
                settings++;
                if(settings > 2) settings = 0;
            }
            next_down = 1;
        }else next_down = 0;
    }
}
/*-------------------------------  Assert -----------------------------------*/
#include "stm/__assert__.h"
