#include "stm8s.h"
#include "main.h"
#include "daughterboard.h"

char bagr;
char string[20] = "\0ahoj"; // ukončení řetězce - ahoj se nevypíše
char tempstring[20] = "\0ahoj";
uint8_t index = 0;
char tmp;
uint8_t i; // char - znak, uint8_t - číslo

void rx_action(void)
{
    REVERSE(LED1);
    // vlajka zůstane nahoře tak dlouho, dokud příchozí znak nepřečtu
    // ReceiveData se musí zavolat
    // při přečtení znaku se vlajka automaticky spadne
    bagr = UART1_ReceiveData8();
    // znak můžu poslat zpět, aby byl vidět
    //putchar(bagr); // nežádoucí // echo
    /*string[0] = bagr;
    string[1] = '\0'; // vypíše poslední znak, který jsme napsali */

    /*string[index] = bagr;
    string[index + 1] = '\0';
    index++;*/

    if (bagr == '\n') {  // Když bude \n, tak se string přepíše
        // překopíruju tempstring do string
        i = 0;
        while ((tmp = tempstring[i])) { // ukončí se při konci řetězce '\0' == 0
            string[i] = tmp;
            i++;
        }
        string[i] = '\0';
        index = 0;

        /*
                                 /
        char * from = tempstring;
        char * to = string;

        while (*to++ = &from++);
        */


    }
    else if (bagr == '\r') { // \r ingnoruje
        ;
    }
    else {
        tempstring[index] = bagr;
        tempstring[index + 1] = '\0';
        index++;
    }
}
