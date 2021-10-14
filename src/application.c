#include <application.h>

#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)
#define ACCELEROMETER_UPDATE_NORMAL_INTERVAL (200)


// LED instance
twr_led_t led;

// Button instance
twr_button_t button;

// Accelerometer
twr_lis2dh12_t acc;
twr_lis2dh12_result_g_t result;


float level = 0.09;

void lis2dh12_event_handler(twr_lis2dh12_t *self, twr_lis2dh12_event_t event, void *event_param)
{


    if (event == TWR_LIS2DH12_EVENT_UPDATE)
    {
        twr_lis2dh12_get_result_g(self, &result);
    }
}

// This function dispatches battery events
void battery_event_handler(twr_module_battery_event_t event, void *event_param)
{
    // Update event?
    if (event == TWR_MODULE_BATTERY_EVENT_UPDATE)
    {
        float voltage;

        // Read battery voltage
        if (twr_module_battery_get_voltage(&voltage))
        {
            twr_log_info("APP: Battery voltage = %.2f", voltage);

            // Publish battery voltage
            twr_radio_pub_battery(&voltage);
        }
    }
}

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    float x;
    float y;
    float z;


    if (event == TWR_BUTTON_EVENT_CLICK)
    {
            x = fabsf(result.x_axis);
            y = fabsf(result.y_axis);
            z = fabsf(result.z_axis);
            if((x <= level && z <= level) ||
                (x <= level && y <= level) ||
                (y <= level && z <= level))
            {
                twr_led_set_mode(&led, TWR_LED_MODE_ON);
                twr_radio_pub_bool("leveled", true);
            }
            else
            {
                twr_radio_pub_bool("leveled", false);
                twr_led_set_mode(&led, TWR_LED_MODE_OFF);
            }
    }
}

void application_init(void)
{
    // Initialize logging
    twr_log_init(TWR_LOG_LEVEL_INFO, TWR_LOG_TIMESTAMP_ABS);

    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);
    twr_led_pulse(&led, 2000);

    // Initialize battery
    twr_module_battery_init();
    twr_module_battery_set_event_handler(battery_event_handler, NULL);
    twr_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    twr_lis2dh12_init(&acc, TWR_I2C_I2C0, 0x19);
    twr_lis2dh12_set_resolution(&acc, TWR_LIS2DH12_RESOLUTION_12BIT);
    twr_lis2dh12_set_event_handler(&acc, lis2dh12_event_handler, NULL);
    twr_lis2dh12_set_update_interval(&acc, ACCELEROMETER_UPDATE_NORMAL_INTERVAL);

    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_hold_time(&button, 500);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
    twr_radio_pairing_request("water-level-detector", VERSION);

}
