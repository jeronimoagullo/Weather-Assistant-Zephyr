
//TODO: https://docs.zephyrproject.org/latest/services/smf/index.html
#include <zephyr/kernel.h>
#include <zephyr/smf.h>

/* States of the FSM = function definitions */
void st_init_run(Fsm *fsm, const Event *event);
void fsm_display_sensor_values(Fsm *fsm, const Event *event);
void fsm_display_forecast_1(Fsm *fsm, const Event *event);
void fsm_display_forecast_2(Fsm *fsm, const Event *event);
void start_display_fsm(struct device *display_device,
				uint8_t height,
				struct sensor_value *temperature_SV,
        		struct sensor_value *humidity_SV);
				
//void fsm_sleep_state(Fsm *fsm, const Event *event);