#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/display/cfb.h>
#include <zephyr/smf.h>

#include "display_fsm.h"

/* Defining display stack area */
#define DISPLAY_STACK_SIZE 512
#define THREAD_DISPLAY_PRIORITY 5
K_THREAD_STACK_DEFINE(display_stack_area, DISPLAY_STACK_SIZE);
struct k_thread thread_display_data;


/* States of the FSM */
enum demo_state { 
	ST_INIT,
	ST_DISPLAY_SENSOR_VALUES,
	ST_DISPLAY_FORECAST_1,
	ST_DISPLAY_FORECAST_2
};

/* State table declaration */
const struct smf_state display_fsm_states[] = {
   [ST_INIT] = SMF_CREATE_STATE(NULL, st_init_run, NULL),
   [ST_DISPLAY_SENSOR_VALUES] = SMF_CREATE_STATE(NULL, fsm_display_sensor_values, NULL),
   [ST_DISPLAY_FORECAST_1] = SMF_CREATE_STATE(NULL, fsm_display_forecast_1, NULL),
   [ST_DISPLAY_FORECAST_2] = SMF_CREATE_STATE(NULL, fsm_display_forecast_2, NULL)
};

/* User defined object */
struct s_object {
        /* This must be first */
        struct smf_ctx ctx;

        /* Other state specific data add here */
        struct device *display_device;
        struct sensor_value *temperature_SV; 
        struct sensor_value *humidity_SV;
        uint8_t height;
}s_obj;

extern void thread_display(void* v1, void *v2, void *v3){
	int32_t ret = 0;

        /* Run the state machine */
        while(1) {
                /* State machine terminates if a non-zero value is returned */
                ret = smf_run_state(SMF_CTX(&s_obj));
                if (ret) {
                        /* handle return code and terminate state machine */
                        break;
                }
                k_msleep(1000);
        }

}

void st_init_run(Fsm *fsm, const Event *event){
	k_tid_t idThreadDisplay = k_thread_create(&thread_display_data, display_stack_area,
							K_THREAD_STACK_SIZEOF(display_stack_area),
							thread_display,
							NULL, NULL, NULL,
							THREAD_DISPLAY_PRIORITY, 0, K_MSEC(2000));

        smf_set_state(SMF_CTX(&s_obj), &demo_states[ST_DISPLAY_SENSOR_VALUES]);
}

void fsm_display_sensor_values(void *o){
        cfb_framebuffer_clear(s_obj.display_device, false);
        cfb_framebuffer_set_font(s_obj.display_device, 0);

        // Display Weather Assistant in lines 1 and 2
        cfb_print(s_obj.display_device, ">>Ambient:",0,0);

        cfb_framebuffer_set_font(display_device, 1);

        // Display temperature in line 3
        sprintf(buf, "%.02fC", sensor_value_to_double(s_obj.temperature_SV));
        cfb_print(display_device, buf, 0, 2 * s_obj.height);

        // Display humidity in line 4
        sprintf(buf, "%.02f%%", sensor_value_to_double(s_obj.humidity_SV));
        cfb_print(display_device, buf, 0, 3 * s_obj.height);

        // Finalize frame to load it into RAM to be displayed
        cfb_framebuffer_finalize(display_device);
}

void fsm_display_forecast_1(Fsm *fsm, const Event *event){
        cfb_framebuffer_clear(display_device, false);
        cfb_framebuffer_set_font(display_device, 0);

        // Display Weather Assistant in lines 1 and 2
        cfb_print(display_device, ">>forecast 1",0,0);

        cfb_framebuffer_set_font(display_device, 1);

        // Finalize frame to load it into RAM to be displayed
        cfb_framebuffer_finalize(display_device);
}

void fsm_display_forecast_2(Fsm *fsm, const Event *event){
        cfb_framebuffer_clear(display_device, false);
        cfb_framebuffer_set_font(display_device, 0);

        // Display Weather Assistant in lines 1 and 2
        cfb_print(display_device, ">>forecast 1",0,0);

        cfb_framebuffer_set_font(display_device, 1);

        // Finalize frame to load it into RAM to be displayed
        cfb_framebuffer_finalize(display_device);
}

void fsm_sleep_state(Fsm *fsm, const Event *event){

}

void start_display_fsm(struct device *display_device,
		uint8_t height,
		struct sensor_value *temperature_SV,
        	struct sensor_value *humidity_SV) {

        s_obj.display_device = display_device;
        s_obj.height= height;
        s_obj.temperature_SV = temperature_SV;
        s_obj.humidity_SV = humidity_SV;

        /* Set initial state */
        smf_set_initial(SMF_CTX(&s_obj), &display_fsm_states[ST_INIT]);
}