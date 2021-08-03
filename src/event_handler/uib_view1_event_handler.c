/*******************************************************************************
* This file was generated by UI Builder.
* User should hand edit this file.
********************************************************************************/

#include "app_main.h"
#include "view_data.h"
#include "uib_app_manager.h"
#include "uib_views.h"
#include "uib_views_inc.h"
#include "sensor_params.h"
#include <sensor.h>
#include <device/power.h>

#define FONT_SIZE 20
#define SENSOR_FREQ 10
char time_file[256];
bool flag = false; //To check if the data is recorded


float check_count_error_HRM = 0;
float check_count_HRM = 0;
float checking_time = 3; //Checking time duration in seconds
float recording_time = 60; //Recording time duration in seconds


int pointer_PPG = 0;
int pointer_HRM = 0;
int pointer_ACC = 0;
int pointer_GRA = 0;
int pointer_GYR = 0;
int pointer_PRE = 0;

float array_PPG[array_size];
float array_HRM[array_size];
float array_ACCX[array_size];
float array_ACCY[array_size];
float array_ACCZ[array_size];
float array_GRAX[array_size];
float array_GRAY[array_size];
float array_GRAZ[array_size];
float array_GYRX[array_size];
float array_GYRY[array_size];
float array_GYRZ[array_size];
float array_PRES[array_size];

unsigned long long array_ts[array_size];

typedef struct _uib_view1_control_context {
	/* add your variables here */

} uib_view1_control_context;

/* Define callback */
void
example_sensor_callback(sensor_h sensor, sensor_event_s *event, uib_view1_view_context *user_data)
{
    /*
       If a callback is used to listen for different sensor types,
       it can check the sensor type
    */
    sensor_type_e type;
    sensor_get_type(sensor, &type);
    char *formatted_label = (char*)malloc(256 * sizeof(char));	// 256문자가 들어갈 수 있는 label을 형성시킨다.
    if (type == SENSOR_HRM) {	// 센서가 Heart Rate이면
		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>HR=%3d</font>", FONT_SIZE,(int) event->values[0]);	// Heart Rate를 표시한다.
		elm_object_text_set(user_data->hrm_data, formatted_label);
    }
    if (type == SENSOR_HRM_LED_GREEN) {	// LED_GREEN 센서가 작동되면
		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font>", FONT_SIZE,event->values[0]);
		elm_object_text_set(user_data->ppg_green, formatted_label);
    }
    if (type == SENSOR_ACCELEROMETER) {
		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font>", FONT_SIZE,event->values[0]);
		elm_object_text_set(user_data->accel_x, formatted_label);
		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font>", FONT_SIZE,event->values[1]);
		elm_object_text_set(user_data->accel_y, formatted_label);
		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font>", FONT_SIZE,event->values[2]);
		elm_object_text_set(user_data->accel_z, formatted_label);
    }
    if (type == SENSOR_GYROSCOPE) {
    		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font>", FONT_SIZE,event->values[0]);
    		elm_object_text_set(user_data->gyro_x, formatted_label);
    		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font>", FONT_SIZE,event->values[1]);
    		elm_object_text_set(user_data->gyro_y, formatted_label);
    		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font>", FONT_SIZE,event->values[2]);
    		elm_object_text_set(user_data->gyro_z, formatted_label);
        }
    if (type == SENSOR_PRESSURE) {
    		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font/>", FONT_SIZE,event->values[0]);
    		elm_object_text_set(user_data->baro, formatted_label);
        }
    if (type == SENSOR_GRAVITY) {
    		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font/>", FONT_SIZE,event->values[0]);
    		elm_object_text_set(user_data->gravity_x, formatted_label);
    		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font/>", FONT_SIZE,event->values[1]);
    		elm_object_text_set(user_data->gravity_y, formatted_label);
    		sprintf(formatted_label, "<font=Tizen:style=Regular font_size=%d>%.2f</font/>", FONT_SIZE,event->values[2]);
			elm_object_text_set(user_data->gravity_z, formatted_label);
        }
}

Eina_Bool
end_sensor(listener){
	// Release all resources.
	sensor_listener_stop(listener);	// Terminates the sensor server for the given listener.
	sensor_destroy_listener(listener);	// Destory the sensor lisnter
	return ECORE_CALLBACK_CANCEL;
}

void
start_sensor(sensor_type_e  sensor_type, uib_view1_view_context *vc){
	//Set sensors and start recording
	sensor_h sensor;
	sensor_get_default_sensor(sensor_type, &sensor);	// Gets a specific sensor handle.
	sensor_listener_h listener;
	sensor_create_listener(sensor, &listener);	// Creates a sensor listener.
	sensor_listener_set_event_cb(listener, 1000/SENSOR_FREQ, example_sensor_callback, vc); //25Hz
	sensor_listener_set_option(listener, SENSOR_OPTION_ALWAYS_ON); // Changes the option of the sensor.
	sensor_listener_start(listener);	// Starts the sensor server for the given listener.
	//End the sensors after the "recording time".
	ecore_timer_add(recording_time,end_sensor,listener);	// Creates a timer to call the given function in the given period of time.
}

void
sensor_not_supported(sensor_name){
	//Record an Error if the sensor is not supported, else continue.
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	FILE * fp = fopen("/opt/usr/media/Downloads/possible_errors.csv","a");
	fprintf (fp, "%s is not available: %s \n", sensor_name, asctime (timeinfo));
	fclose (fp);
}

void view1_start_stop_onclicked(uib_view1_view_context *vc, Evas_Object *obj, void *event_info) {
	//PPG
			bool supported_PPG = false;	// PPG support boolean variable 초기화
			sensor_type_e sensor_type_PPG = SENSOR_HRM_LED_GREEN;	// LED GREEN으로 Heart Rate를 측정한다.
			sensor_is_supported(sensor_type_PPG, &supported_PPG); 	// Checks whether a given sensor type is available on a device.
			if (!supported_PPG) {	// sensor가 support가 안된다면
				char sensor_name_PPG[256] = "PPG";	// sensor_name_PPG에 PPG 값을 집어 넣는다.
				sensor_not_supported(sensor_name_PPG);	// sensor_not_supported 에 이름을 집어 넣는다?
			} else{
				start_sensor(sensor_type_PPG,vc);	// 센서 기능을 시작한다.
			}

			//HRM
			bool supported_HRM = false;
			sensor_type_e sensor_type_HRM = SENSOR_HRM;
			sensor_is_supported(sensor_type_HRM, &supported_HRM);
			if (!supported_HRM) {
				char sensor_name_HRM[256] = "HRM";
				sensor_not_supported(sensor_name_HRM);
			} else{
				start_sensor(sensor_type_HRM, vc);
			}

			//ACC
			bool supported_ACC = false;
			sensor_type_e sensor_type_ACC = SENSOR_ACCELEROMETER;
			sensor_is_supported(sensor_type_ACC, &supported_ACC);
			if (!supported_ACC) {
				char sensor_name_ACC[256] = "ACC";
				sensor_not_supported(sensor_name_ACC);
			} else{
				start_sensor(sensor_type_ACC, vc);
			}
			//Gravity
			bool supported_Gravity = false;
			sensor_type_e sensor_type_Gravity = SENSOR_GRAVITY;
			sensor_is_supported(sensor_type_Gravity, &supported_Gravity);
			if (!supported_Gravity) {
				char sensor_name_Gravity[256] = "Gravity";
				sensor_not_supported(sensor_name_Gravity);
			} else{
				start_sensor(sensor_type_Gravity, vc);
			}

			//Gyroscope
			bool supported_Gyro = false;
			sensor_type_e sensor_type_Gyro = SENSOR_GYROSCOPE;
			sensor_is_supported(sensor_type_Gyro, &supported_Gyro);
			if (!supported_Gyro) {
				char sensor_name_Gyro[256] = "Gyro";
				sensor_not_supported(sensor_name_Gyro);
			} else{
				start_sensor(sensor_type_Gyro, vc);
			}

			//Atmospheric pressure
			bool supported_Pres = false;
			sensor_type_e sensor_type_Pres = SENSOR_PRESSURE;
			sensor_is_supported(sensor_type_Pres, &supported_Pres);
			if (!supported_Pres) {
				char sensor_name_Pres[256] = "Pressure";
				sensor_not_supported(sensor_name_Pres);
			} else{
				start_sensor(sensor_type_Pres, vc);
			}


}




