#include "ss_clock.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>

struct SS_CLOCK ss_clock;

struct SS_CLOCK* ss_clock_init(uint8_t config) {

    ss_clock.ahb = 16;
    ss_clock.apb1 = 16;
    ss_clock.apb2 = 16;

    switch (config)
    {
        case SS_CLOCK_DEFAULT: 
        break;

        case SS_CLOCK_FAST:
            {
                ss_clock.ahb = 168;
                ss_clock.apb1 = 42;
                ss_clock.apb2 = 84;
                rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_168MHZ]);
            }
        break;
    
        default:
        break;
    }

    return &ss_clock;
}

uint8_t ss_clock_can(struct SS_CLOCK_CAN* config, uint32_t baudrate, struct SS_CLOCK* clock_config) {
    switch (baudrate)
    {
    case 1000000:
        switch (clock_config->apb1)
        {
            case 16:
                {
                    config->prescaler = 1;
                    config->tseg1 = CAN_BTR_TS1_12TQ;
                    config->tseg2 = CAN_BTR_TS2_3TQ;
                    config->sjw = CAN_BTR_SJW_1TQ;
                }
            break;

            case 42:
                {
                    config->prescaler = 6;
                    config->tseg1 = CAN_BTR_TS1_4TQ;
                    config->tseg2 = CAN_BTR_TS2_2TQ;
                    config->sjw = CAN_BTR_SJW_1TQ;
                }
                break;
            
            default:
                return 0;
            break;
        }
        break;
    
        default:
            return 0;
        break;
    }

    return 1;
}
