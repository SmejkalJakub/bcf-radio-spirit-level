#include <application.h>

#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)
#define ACCELEROMETER_UPDATE_NORMAL_INTERVAL (200)


// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Accelerometer
bc_lis2dh12_t acc;
bc_lis2dh12_result_g_t result;


float level = 0.09;

void lis2dh12_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param)
{


    if (event == BC_LIS2DH12_EVENT_UPDATE)
    {
        bc_lis2dh12_get_result_g(self, &result);
    }
}

// This function dispatches battery events
void battery_event_handler(bc_module_battery_event_t event, void *event_param)
{
    // Update event?
    if (event == BC_MODULE_BATTERY_EVENT_UPDATE)
    {
        float voltage;

        // Read battery voltage
        if (bc_module_battery_get_voltage(&voltage))
        {
            bc_log_info("APP: Battery voltage = %.2f", voltage);

            // Publish battery voltage
            bc_radio_pub_battery(&voltage);
        }
    }
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    float x;
    float y;
    float z;


    if (event == BC_BUTTON_EVENT_CLICK)
    {
            x = fabsf(result.x_axis);
            y = fabsf(result.y_axis);
            z = fabsf(result.z_axis);
            if((x <= level && z <= level) ||
                (x <= level && y <= level) ||
                (y <= level && z <= level))
            {
                bc_led_set_mode(&led, BC_LED_MODE_ON);
                bc_radio_pub_bool("leveled", true);
            }
            else
            {
                bc_radio_pub_bool("leveled", false);
                bc_led_set_mode(&led, BC_LED_MODE_OFF);
            }
    }
}

void application_init(void)
{
    // Initialize logging
    bc_log_init(BC_LOG_LEVEL_INFO, BC_LOG_TIMESTAMP_ABS);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_pulse(&led, 2000);

    // Initialize battery
    bc_module_battery_init();
    bc_module_battery_set_event_handler(battery_event_handler, NULL);
    bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    bc_lis2dh12_init(&acc, BC_I2C_I2C0, 0x19);
    bc_lis2dh12_set_resolution(&acc, BC_LIS2DH12_RESOLUTION_12BIT);
    bc_lis2dh12_set_event_handler(&acc, lis2dh12_event_handler, NULL);
    bc_lis2dh12_set_update_interval(&acc, ACCELEROMETER_UPDATE_NORMAL_INTERVAL);

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_hold_time(&button, 500);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);
    bc_radio_pairing_request("water-level-detector", VERSION);

}
