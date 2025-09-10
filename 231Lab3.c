// ECE 231 Lab 3, Saad Ahmed Malik //

#include "configure_gpio_input.c"
#include "togglePWM.c"

int main() {

    char pin_number[32] = "P9_16";
    char pwmchip[32] = "pwmchip4";
    char channel[32] = "1";

    int gpio_number1 = 67;
    int gpio_number2 = 69;
    configure_gpio_input(gpio_number1);
    configure_gpio_input(gpio_number2);
    
    char valuePath1[40];
    char valuePath2[40];
    sprintf(valuePath1, "/sys/class/gpio/gpio%d/value", gpio_number1);
    sprintf(valuePath2, "/sys/class/gpio/gpio%d/value", gpio_number2);    
    
    sleep(1);
    int state1, state2;
    FILE *fp1, *fp2;
    
    while(1) {
        fp1 = fopen(valuePath1, "r");
        fp2 = fopen(valuePath2, "r");
        fscanf(fp1,"%d", &state1);
        fscanf(fp2,"%d", &state2);
        fclose(fp1);
        fclose(fp2);

        if (state1 == 0) {
            char duty_cycle[32] = "50000000";
            char period[32] = "100000000";            
            printf("Button referring to GPIO67 has been pressed\n");
            start_pwm(pin_number, pwmchip, channel, period, duty_cycle);
            usleep(100000);
            stop_pwm(pin_number, pwmchip, channel);
        }
        if (state2 == 0 ){
            char duty_cycle[32] = "500000";
            char period[32] = "1000000";
            printf("Button referring to GPIO69 has been pressed\n");
            start_pwm(pin_number, pwmchip, channel, period, duty_cycle);
            usleep(100000);
            stop_pwm(pin_number, pwmchip, channel);
        }
    }
    return 0;
}
