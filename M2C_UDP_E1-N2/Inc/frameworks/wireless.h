#ifndef WIRELESS_H__
#define WIRELESS_H__

#include "package.h"
//#include "main.h"

struct wireless_t;

#ifndef WIRELESS_BIN
#define WIRELESS_BIN 1
#endif

#define wireless_keyInt(c,k) (WIRELESS_KEY_##c##_##k)
#define wireless_keyString(c,k) (#k)

#if WIRELESS_BIN
#define wireless_key(c,k) wireless_keyInt(c,k)
#define wireless_findValueByKey(p,k) package_findValueByIntKey(p,(k))
#define wireless_new(k) package_newInt16((k))
typedef int16_t wireless_key_t;
#else
#define wireless_key(c,k) wireless_keyString(c,k)
#define wireless_findValueByKey(p,k) package_findValueByStringKey(p,(k))
#define wireless_new(k) package_newString((k))
typedef char* wireless_key_t;
#endif

#define WIRELESS_KEY_contents_contents 0
enum wireless_keys_root_t {
	WIRELESS_KEY_root_deviceId = 0,
	WIRELESS_KEY_root_sessionId,
	WIRELESS_KEY_root_version,
	WIRELESS_KEY_root_routers,
	WIRELESS_KEY_root_route,
	WIRELESS_KEY_root_setSessionId,
	WIRELESS_KEY_root_data = 0x10,
	WIRELESS_KEY_root_requests,
	WIRELESS_KEY_root_responses,
	WIRELESS_KEY_root_commands,
	WIRELESS_KEY_root_line,
	WIRELESS_KEY_root_error = 30,
};

enum wireless_keys_version_t {
	WIRELESS_KEY_version_hardware,
	WIRELESS_KEY_version_firmware,
	WIRELESS_KEY_version_protocol,
};

enum wireless_keys_route_t {
	WIRELESS_KEY_route_protocol,
	WIRELESS_KEY_route_address,
	WIRELESS_KEY_route_port,
};

enum wireless_keys_data_t{
	WIRELESS_KEY_data__interval = 0x00,
	WIRELESS_KEY_data_battery = 0x10,
	WIRELESS_KEY_data_rssi = 0x11,

	WIRELESS_KEY_data_pm2d5 = 0x20,
	WIRELESS_KEY_data_pm0d3 = 0x21,
	WIRELESS_KEY_data_pm1d0 = 0x22,
	WIRELESS_KEY_data_pm5d0 = 0x23,
	WIRELESS_KEY_data_pm10	= 0x24,
	WIRELESS_KEY_data_pc0d3 = 0x28,
	WIRELESS_KEY_data_pc1d0 = 0x29,
	WIRELESS_KEY_data_pc2d5 = 0x2a,
	WIRELESS_KEY_data_pc5d0 = 0x2b,
	WIRELESS_KEY_data_pc10	= 0x2c,
	WIRELESS_KEY_data_pal		= 0x2f,
	WIRELESS_KEY_data_ch2o  = 0x30,
	WIRELESS_KEY_data_voc		= 0x33,
	WIRELESS_KEY_data_ch4		= 0x34,
	WIRELESS_KEY_data_co2 = 0x35,
	WIRELESS_KEY_data_temperature = 0x38,
	WIRELESS_KEY_data_humidity = 0x39,
	WIRELESS_KEY_data_noiseSpl = 0x3a,
	WIRELESS_KEY_data_anion = 0x3e,
	WIRELESS_KEY_data_aqi = 0x40,
	WIRELESS_KEY_data_o3 = 0x41,
	WIRELESS_KEY_data_so2 = 0x42,
	WIRELESS_KEY_data_no2 = 0x43,
	WIRELESS_KEY_data_co = 0x44,
	WIRELESS_KEY_data_wind_speed = 0x54,
	WIRELESS_KEY_data_wind_scale = 0x55,
	WIRELESS_KEY_data_wind_direction_degree = 0x57,
	WIRELESS_KEY_data_pressure = 0x58,
	WIRELESS_KEY_data_visibility = 0x5b,
	WIRELESS_KEY_data_feels_like = 0x5c,
	WIRELESS_KEY_data_code = 0x60,
	WIRELESS_KEY_data_code_day = 0x61,
	WIRELESS_KEY_data_high = 0x62,
	WIRELESS_KEY_data_code_night = 0x63,
	WIRELESS_KEY_data_low = 0x64,
	WIRELESS_KEY_data_co2Warming = 0xc5,
};
//request
enum wireless_keys_request_t{
	WIRELESS_KEY_request_time = 0x00,
	WIRELESS_KEY_request_weather = 0x10,
	WIRELESS_KEY_request_weatherHistory = 0x11,
	WIRELESS_KEY_request_weatherForecast = 0x12,
};

enum wireless_keys_request_time_t{
	WIRELESS_KEY_request_time_timestamp = 0x00,
};
enum wireless_keys_request_weather_t{
	WIRELESS_KEY_request_weather_timestamp = 0x00,
	WIRELESS_KEY_request_weather_dataTypes = 0x01,
	WIRELESS_KEY_request_weather_data = 0x10,
};
// enum wireless_keys_request_weather_data_t{
// 	WIRELESS_KEY_request_weather_data_data,
// };

enum wireless_keys_request_weatherHistory_t{
	WIRELESS_KEY_request_weatherHistory_timestamp = 0x00,
	WIRELESS_KEY_request_weatherHistory_dataTypes = 0x01,
	WIRELESS_KEY_request_weatherHistory_level  = 0x02,
	WIRELESS_KEY_request_weatherHistory_interval  = 0x03,
	WIRELESS_KEY_request_weatherHistory_startInclude = 0x04,
	WIRELESS_KEY_request_weatherHistory_startIncludeOffset = 0x05,
	WIRELESS_KEY_request_weatherHistory_endExclude = 0x06,
	WIRELESS_KEY_request_weatherHistory_endExcludeOffset = 0x07,
	WIRELESS_KEY_request_weatherHistory_data = 0x10,
};
enum wireless_keys_request_weatherHistory_data_t{
	WIRELESS_KEY_request_weatherHistory_data_timestamp = 0x01,
	WIRELESS_KEY_request_weatherHistory_data_data = 0x10,
};


enum wireless_keys_request_weatherForecast_t{
	WIRELESS_KEY_request_weatherForecast_timestamp = 0x00,
	WIRELESS_KEY_request_weatherForecast_dataTypes = 0x01,
	WIRELESS_KEY_request_weatherForecast_data = 0x10,

};
enum wireless_keys_request_weatherForecast_data_t{
	WIRELESS_KEY_request_weatherForecast_data_timestamp = 0x01,
	WIRELESS_KEY_request_weatherForecast_data_data = 0x10,
};
//weather data
/*
enum wireless_keys_request_weather_data_data_t{
	WIRELESS_KEY_request_weather_data_data_interval = 0x00,

	WIRELESS_KEY_request_weather_data_data_pm2d5 = 0x20,
	WIRELESS_KEY_request_weather_data_data_pm0d3 = 0x21,
	WIRELESS_KEY_request_weather_data_data_pm1d0 = 0x22,
	WIRELESS_KEY_request_weather_data_data_pm5d0 = 0x23,
	WIRELESS_KEY_request_weather_data_data_pm10	= 0x24,
	WIRELESS_KEY_request_weather_data_data_pc0d3 = 0x28,
	WIRELESS_KEY_request_weather_data_data_pc1d0 = 0x29,
	WIRELESS_KEY_request_weather_data_data_pc2d5 = 0x2a,
	WIRELESS_KEY_request_weather_data_data_pc5d0 = 0x2b,
	WIRELESS_KEY_request_weather_data_data_pc10	= 0x2c,
	WIRELESS_KEY_request_weather_data_data_pal		= 0x2f,
	WIRELESS_KEY_request_weather_data_data_ch2o  = 0x30,
	WIRELESS_KEY_request_weather_data_data_voc		= 0x33,
	WIRELESS_KEY_request_weather_data_data_ch4		= 0x34,
	WIRELESS_KEY_request_weather_data_data_co2 = 0x35,
	WIRELESS_KEY_request_weather_data_data_temperature = 0x38,
	WIRELESS_KEY_request_weather_data_data_humidity = 0x39,
	WIRELESS_KEY_request_weather_data_data_noiseSpl = 0x3a,
	WIRELESS_KEY_request_weather_data_data_anion = 0x3e,
	WIRELESS_KEY_request_weather_data_data_aqi = 0x40,
	WIRELESS_KEY_request_weather_data_data_o3 = 0x41,
	WIRELESS_KEY_request_weather_data_data_so2 = 0x42,
	WIRELESS_KEY_request_weather_data_data_no2 = 0x43,
	WIRELESS_KEY_request_weather_data_data_co = 0x44,
	WIRELESS_KEY_request_weather_data_data_wind_speed = 0x54,
	WIRELESS_KEY_request_weather_data_data_wind_scale = 0x55,
	WIRELESS_KEY_request_weather_data_data_wind_direction_degree = 0x57,
	WIRELESS_KEY_request_weather_data_data_pressure = 0x58,
	WIRELESS_KEY_request_weather_data_data_visibility = 0x5b,
	WIRELESS_KEY_request_weather_data_data_feels_like = 0x5c,
	WIRELESS_KEY_request_weather_data_data_code = 0x60,
	WIRELESS_KEY_request_weather_data_data_code_day = 0x61,
	WIRELESS_KEY_request_weather_data_data_high = 0x62,
	WIRELESS_KEY_request_weather_data_data_code_night = 0x63,
	WIRELESS_KEY_request_weather_data_data_low = 0x64,
};
enum wireless_keys_request_weatherHistory_data_data_t{
	WIRELESS_KEY_request_weatherHistory_data_data_interval = 0x00,

	WIRELESS_KEY_request_weatherHistory_data_data_pm2d5 = 0x20,
	WIRELESS_KEY_request_weatherHistory_data_data_pm0d3 = 0x21,
	WIRELESS_KEY_request_weatherHistory_data_data_pm1d0 = 0x22,
	WIRELESS_KEY_request_weatherHistory_data_data_pm5d0 = 0x23,
	WIRELESS_KEY_request_weatherHistory_data_data_pm10	= 0x24,
	WIRELESS_KEY_request_weatherHistory_data_data_pc0d3 = 0x28,
	WIRELESS_KEY_request_weatherHistory_data_data_pc1d0 = 0x29,
	WIRELESS_KEY_request_weatherHistory_data_data_pc2d5 = 0x2a,
	WIRELESS_KEY_request_weatherHistory_data_data_pc5d0 = 0x2b,
	WIRELESS_KEY_request_weatherHistory_data_data_pc10	= 0x2c,
	WIRELESS_KEY_request_weatherHistory_data_data_pal		= 0x2f,
	WIRELESS_KEY_request_weatherHistory_data_data_ch2o  = 0x30,
	WIRELESS_KEY_request_weatherHistory_data_data_voc		= 0x33,
	WIRELESS_KEY_request_weatherHistory_data_data_ch4		= 0x34,
	WIRELESS_KEY_request_weatherHistory_data_data_co2 = 0x35,
	WIRELESS_KEY_request_weatherHistory_data_data_temperature = 0x38,
	WIRELESS_KEY_request_weatherHistory_data_data_humidity = 0x39,
	WIRELESS_KEY_request_weatherHistory_data_data_noiseSpl = 0x3a,
	WIRELESS_KEY_request_weatherHistory_data_data_anion = 0x3e,
	WIRELESS_KEY_request_weatherHistory_data_data_aqi = 0x40,
	WIRELESS_KEY_request_weatherHistory_data_data_o3 = 0x41,
	WIRELESS_KEY_request_weatherHistory_data_data_so2 = 0x42,
	WIRELESS_KEY_request_weatherHistory_data_data_no2 = 0x43,
	WIRELESS_KEY_request_weatherHistory_data_data_co = 0x44,
	WIRELESS_KEY_request_weatherHistory_data_data_wind_speed = 0x54,
	WIRELESS_KEY_request_weatherHistory_data_data_wind_scale = 0x55,
	WIRELESS_KEY_request_weatherHistory_data_data_wind_direction_degree = 0x57,
	WIRELESS_KEY_request_weatherHistory_data_data_pressure = 0x58,
	WIRELESS_KEY_request_weatherHistory_data_data_visibility = 0x5b,
	WIRELESS_KEY_request_weatherHistory_data_data_feels_like = 0x5c,
	WIRELESS_KEY_request_weatherHistory_data_data_code = 0x60,
	WIRELESS_KEY_request_weatherHistory_data_data_code_day = 0x61,
	WIRELESS_KEY_request_weatherHistory_data_data_high = 0x62,
	WIRELESS_KEY_request_weatherHistory_data_data_code_night = 0x63,
	WIRELESS_KEY_request_weatherHistory_data_data_low = 0x64,
};
enum wireless_keys_request_weatherForecast_data_data_t{
	WIRELESS_KEY_request_weatherForecast_data_data_interval = 0x00,

	WIRELESS_KEY_request_weatherForecast_data_data_pm2d5 = 0x20,
	WIRELESS_KEY_request_weatherForecast_data_data_pm0d3 = 0x21,
	WIRELESS_KEY_request_weatherForecast_data_data_pm1d0 = 0x22,
	WIRELESS_KEY_request_weatherForecast_data_data_pm5d0 = 0x23,
	WIRELESS_KEY_request_weatherForecast_data_data_pm10	= 0x24,
	WIRELESS_KEY_request_weatherForecast_data_data_pc0d3 = 0x28,
	WIRELESS_KEY_request_weatherForecast_data_data_pc1d0 = 0x29,
	WIRELESS_KEY_request_weatherForecast_data_data_pc2d5 = 0x2a,
	WIRELESS_KEY_request_weatherForecast_data_data_pc5d0 = 0x2b,
	WIRELESS_KEY_request_weatherForecast_data_data_pc10	= 0x2c,
	WIRELESS_KEY_request_weatherForecast_data_data_pal		= 0x2f,
	WIRELESS_KEY_request_weatherForecast_data_data_ch2o  = 0x30,
	WIRELESS_KEY_request_weatherForecast_data_data_voc		= 0x33,
	WIRELESS_KEY_request_weatherForecast_data_data_ch4		= 0x34,
	WIRELESS_KEY_request_weatherForecast_data_data_co2 = 0x35,
	WIRELESS_KEY_request_weatherForecast_data_data_temperature = 0x38,
	WIRELESS_KEY_request_weatherForecast_data_data_humidity = 0x39,
	WIRELESS_KEY_request_weatherForecast_data_data_noiseSpl = 0x3a,
	WIRELESS_KEY_request_weatherForecast_data_data_anion = 0x3e,
	WIRELESS_KEY_request_weatherForecast_data_data_aqi = 0x40,
	WIRELESS_KEY_request_weatherForecast_data_data_o3 = 0x41,
	WIRELESS_KEY_request_weatherForecast_data_data_so2 = 0x42,
	WIRELESS_KEY_request_weatherForecast_data_data_no2 = 0x43,
	WIRELESS_KEY_request_weatherForecast_data_data_co = 0x44,
	WIRELESS_KEY_request_weatherForecast_data_data_wind_speed = 0x54,
	WIRELESS_KEY_request_weatherForecast_data_data_wind_scale = 0x55,
	WIRELESS_KEY_request_weatherForecast_data_data_wind_direction_degree = 0x57,
	WIRELESS_KEY_request_weatherForecast_data_data_pressure = 0x58,
	WIRELESS_KEY_request_weatherForecast_data_data_visibility = 0x5b,
	WIRELESS_KEY_request_weatherForecast_data_data_feels_like = 0x5c,
	WIRELESS_KEY_request_weatherForecast_data_data_code = 0x60,
	WIRELESS_KEY_request_weatherForecast_data_data_code_day = 0x61,
	WIRELESS_KEY_request_weatherForecast_data_data_high = 0x62,
	WIRELESS_KEY_request_weatherForecast_data_data_code_night = 0x63,
	WIRELESS_KEY_request_weatherForecast_data_data_low = 0x64,
};

//response
enum wireless_keys_response_t{
	WIRELESS_KEY_response_time = 0x00,
	WIRELESS_KEY_response_weather = 0x10,
	WIRELESS_KEY_response_weatherHistory = 0x11,
	WIRELESS_KEY_response_weatherForecast = 0x12,
};
enum wireless_keys_response_time_t{
	WIRELESS_KEY_response_time_timestamp = 0x00,

};
enum wireless_keys_response_weather_t{
	WIRELESS_KEY_response_weather_timestamp = 0x00,
	WIRELESS_KEY_response_weather_dataTypes = 0x01,
	WIRELESS_KEY_response_weather_data = 0x10,
};
enum wireless_keys_response_weatherHistory_t{
	WIRELESS_KEY_response_weatherHistory_timestamp = 0x00,
	WIRELESS_KEY_response_weatherHistory_dataTypes = 0x01,
	WIRELESS_KEY_response_weatherHistory_level  = 0x02,
	WIRELESS_KEY_response_weatherHistory_interval  = 0x03,
	WIRELESS_KEY_response_weatherHistory_startInclude = 0x04,
	WIRELESS_KEY_response_weatherHistory_startIncludeOffset = 0x05,
	WIRELESS_KEY_response_weatherHistory_endExclude = 0x06,
	WIRELESS_KEY_response_weatherHistory_endExcludeOffset = 0x07,
	WIRELESS_KEY_response_weatherHistory_data = 0x10,
};
enum wireless_keys_response_weatherHistory_data_t{
	WIRELESS_KEY_response_weatherHistory_data_timestamp = 0x01,
	WIRELESS_KEY_response_weatherHistory_data_data = 0x10,
};


enum wireless_keys_response_weatherForecast_t{
	WIRELESS_KEY_response_weatherForecast_timestamp = 0x00,
	WIRELESS_KEY_response_weatherForecast_dataTypes = 0x01,
	WIRELESS_KEY_response_weatherForecast_data = 0x10,

};
enum wireless_keys_response_weatherForecast_data_t{
	WIRELESS_KEY_response_weatherForecast_data_timestamp = 0x01,
	WIRELESS_KEY_response_weatherForecast_data_data = 0x10,
};


//weather data
enum wireless_keys_response_weather_data_data_t{
	WIRELESS_KEY_response_weather_data_data_interval = 0x00,

	WIRELESS_KEY_response_weather_data_data_pm2d5 = 0x20,
	WIRELESS_KEY_response_weather_data_data_pm0d3 = 0x21,
	WIRELESS_KEY_response_weather_data_data_pm1d0 = 0x22,
	WIRELESS_KEY_response_weather_data_data_pm5d0 = 0x23,
	WIRELESS_KEY_response_weather_data_data_pm10	= 0x24,
	WIRELESS_KEY_response_weather_data_data_pc0d3 = 0x28,
	WIRELESS_KEY_response_weather_data_data_pc1d0 = 0x29,
	WIRELESS_KEY_response_weather_data_data_pc2d5 = 0x2a,
	WIRELESS_KEY_response_weather_data_data_pc5d0 = 0x2b,
	WIRELESS_KEY_response_weather_data_data_pc10	= 0x2c,
	WIRELESS_KEY_response_weather_data_data_pal		= 0x2f,
	WIRELESS_KEY_response_weather_data_data_ch2o  = 0x30,
	WIRELESS_KEY_response_weather_data_data_voc		= 0x33,
	WIRELESS_KEY_response_weather_data_data_ch4		= 0x34,
	WIRELESS_KEY_response_weather_data_data_co2 = 0x35,
	WIRELESS_KEY_response_weather_data_data_temperature = 0x38,
	WIRELESS_KEY_response_weather_data_data_humidity = 0x39,
	WIRELESS_KEY_response_weather_data_data_noiseSpl = 0x3a,
	WIRELESS_KEY_response_weather_data_data_anion = 0x3e,
	WIRELESS_KEY_response_weather_data_data_aqi = 0x40,
	WIRELESS_KEY_response_weather_data_data_o3 = 0x41,
	WIRELESS_KEY_response_weather_data_data_so2 = 0x42,
	WIRELESS_KEY_response_weather_data_data_no2 = 0x43,
	WIRELESS_KEY_response_weather_data_data_co = 0x44,
	WIRELESS_KEY_response_weather_data_data_wind_speed = 0x54,
	WIRELESS_KEY_response_weather_data_data_wind_scale = 0x55,
	WIRELESS_KEY_response_weather_data_data_wind_direction_degree = 0x57,
	WIRELESS_KEY_response_weather_data_data_pressure = 0x58,
	WIRELESS_KEY_response_weather_data_data_visibility = 0x5b,
	WIRELESS_KEY_response_weather_data_data_feels_like = 0x5c,
	WIRELESS_KEY_response_weather_data_data_code = 0x60,
	WIRELESS_KEY_response_weather_data_data_code_day = 0x61,
	WIRELESS_KEY_response_weather_data_data_high = 0x62,
	WIRELESS_KEY_response_weather_data_data_code_night = 0x63,
	WIRELESS_KEY_response_weather_data_data_low = 0x64,
};
enum wireless_keys_response_weatherHistory_data_data_t{
	WIRELESS_KEY_response_weatherHistory_data_data_interval = 0x00,

	WIRELESS_KEY_response_weatherHistory_data_data_pm2d5 = 0x20,
	WIRELESS_KEY_response_weatherHistory_data_data_pm0d3 = 0x21,
	WIRELESS_KEY_response_weatherHistory_data_data_pm1d0 = 0x22,
	WIRELESS_KEY_response_weatherHistory_data_data_pm5d0 = 0x23,
	WIRELESS_KEY_response_weatherHistory_data_data_pm10	= 0x24,
	WIRELESS_KEY_response_weatherHistory_data_data_pc0d3 = 0x28,
	WIRELESS_KEY_response_weatherHistory_data_data_pc1d0 = 0x29,
	WIRELESS_KEY_response_weatherHistory_data_data_pc2d5 = 0x2a,
	WIRELESS_KEY_response_weatherHistory_data_data_pc5d0 = 0x2b,
	WIRELESS_KEY_response_weatherHistory_data_data_pc10	= 0x2c,
	WIRELESS_KEY_response_weatherHistory_data_data_pal		= 0x2f,
	WIRELESS_KEY_response_weatherHistory_data_data_ch2o  = 0x30,
	WIRELESS_KEY_response_weatherHistory_data_data_voc		= 0x33,
	WIRELESS_KEY_response_weatherHistory_data_data_ch4		= 0x34,
	WIRELESS_KEY_response_weatherHistory_data_data_co2 = 0x35,
	WIRELESS_KEY_response_weatherHistory_data_data_temperature = 0x38,
	WIRELESS_KEY_response_weatherHistory_data_data_humidity = 0x39,
	WIRELESS_KEY_response_weatherHistory_data_data_noiseSpl = 0x3a,
	WIRELESS_KEY_response_weatherHistory_data_data_anion = 0x3e,
	WIRELESS_KEY_response_weatherHistory_data_data_aqi = 0x40,
	WIRELESS_KEY_response_weatherHistory_data_data_o3 = 0x41,
	WIRELESS_KEY_response_weatherHistory_data_data_so2 = 0x42,
	WIRELESS_KEY_response_weatherHistory_data_data_no2 = 0x43,
	WIRELESS_KEY_response_weatherHistory_data_data_co = 0x44,
	WIRELESS_KEY_response_weatherHistory_data_data_wind_speed = 0x54,
	WIRELESS_KEY_response_weatherHistory_data_data_wind_scale = 0x55,
	WIRELESS_KEY_response_weatherHistory_data_data_wind_direction_degree = 0x57,
	WIRELESS_KEY_response_weatherHistory_data_data_pressure = 0x58,
	WIRELESS_KEY_response_weatherHistory_data_data_visibility = 0x5b,
	WIRELESS_KEY_response_weatherHistory_data_data_feels_like = 0x5c,
	WIRELESS_KEY_response_weatherHistory_data_data_code = 0x60,
	WIRELESS_KEY_response_weatherHistory_data_data_code_day = 0x61,
	WIRELESS_KEY_response_weatherHistory_data_data_high = 0x62,
	WIRELESS_KEY_response_weatherHistory_data_data_code_night = 0x63,
	WIRELESS_KEY_response_weatherHistory_data_data_low = 0x64,
};
enum wireless_keys_response_weatherForecast_data_data_t{
	WIRELESS_KEY_response_weatherForecast_data_data_interval = 0x00,

	WIRELESS_KEY_response_weatherForecast_data_data_pm2d5 = 0x20,
	WIRELESS_KEY_response_weatherForecast_data_data_pm0d3 = 0x21,
	WIRELESS_KEY_response_weatherForecast_data_data_pm1d0 = 0x22,
	WIRELESS_KEY_response_weatherForecast_data_data_pm5d0 = 0x23,
	WIRELESS_KEY_response_weatherForecast_data_data_pm10	= 0x24,
	WIRELESS_KEY_response_weatherForecast_data_data_pc0d3 = 0x28,
	WIRELESS_KEY_response_weatherForecast_data_data_pc1d0 = 0x29,
	WIRELESS_KEY_response_weatherForecast_data_data_pc2d5 = 0x2a,
	WIRELESS_KEY_response_weatherForecast_data_data_pc5d0 = 0x2b,
	WIRELESS_KEY_response_weatherForecast_data_data_pc10	= 0x2c,
	WIRELESS_KEY_response_weatherForecast_data_data_pal		= 0x2f,
	WIRELESS_KEY_response_weatherForecast_data_data_ch2o  = 0x30,
	WIRELESS_KEY_response_weatherForecast_data_data_voc		= 0x33,
	WIRELESS_KEY_response_weatherForecast_data_data_ch4		= 0x34,
	WIRELESS_KEY_response_weatherForecast_data_data_co2 = 0x35,
	WIRELESS_KEY_response_weatherForecast_data_data_temperature = 0x38,
	WIRELESS_KEY_response_weatherForecast_data_data_humidity = 0x39,
	WIRELESS_KEY_response_weatherForecast_data_data_noiseSpl = 0x3a,
	WIRELESS_KEY_response_weatherForecast_data_data_anion = 0x3e,
	WIRELESS_KEY_response_weatherForecast_data_data_aqi = 0x40,
	WIRELESS_KEY_response_weatherForecast_data_data_o3 = 0x41,
	WIRELESS_KEY_response_weatherForecast_data_data_so2 = 0x42,
	WIRELESS_KEY_response_weatherForecast_data_data_no2 = 0x43,
	WIRELESS_KEY_response_weatherForecast_data_data_co = 0x44,
	WIRELESS_KEY_response_weatherForecast_data_data_wind_speed = 0x54,
	WIRELESS_KEY_response_weatherForecast_data_data_wind_scale = 0x55,
	WIRELESS_KEY_response_weatherForecast_data_data_wind_direction_degree = 0x57,
	WIRELESS_KEY_response_weatherForecast_data_data_pressure = 0x58,
	WIRELESS_KEY_response_weatherForecast_data_data_visibility = 0x5b,
	WIRELESS_KEY_response_weatherForecast_data_data_feels_like = 0x5c,
	WIRELESS_KEY_response_weatherForecast_data_data_code = 0x60,
	WIRELESS_KEY_response_weatherForecast_data_data_code_day = 0x61,
	WIRELESS_KEY_response_weatherForecast_data_data_high = 0x62,
	WIRELESS_KEY_response_weatherForecast_data_data_code_night = 0x63,
	WIRELESS_KEY_response_weatherForecast_data_data_low = 0x64,
};

*/

//command
enum wireless_keys_command_t{
	WIRELESS_KEY_command_data = 0x10,
	WIRELESS_KEY_command_line = 0x01,
	WIRELESS_KEY_command_delayed = 0x02,
	WIRELESS_KEY_command_time = 0x82,
	WIRELESS_KEY_command_by = 0x03,
};
//general
enum wireless_keys_commandGeneral_data_t{
	WIRELESS_KEY_command_data_ssid = 0x00,
	WIRELESS_KEY_command_data_password = 0x01,
	WIRELESS_KEY_command_data_imei = 0x08,
	WIRELESS_KEY_command_data_imsi = 0x09,
	WIRELESS_KEY_command_data_latitude = 0x0a,
	WIRELESS_KEY_command_data_longitude = 0x0b,
	WIRELESS_KEY_command_data_ipLocate = 0x0c,
	WIRELESS_KEY_command_data_started = 0x10,
	WIRELESS_KEY_command_data_power = 0x20,
	WIRELESS_KEY_command_data_pairing = 0x30,
	WIRELESS_KEY_command_data_firmware = 0x38,
	WIRELESS_KEY_command_data_durTarget = 0x60,
	WIRELESS_KEY_command_data_deviceIdChanged = 0x70,
	WIRELESS_KEY_command_data_weatherIdChanged = 0x71,
	WIRELESS_KEY_command_data_upgradeVersionChanged = 0x72,
};
enum wireless_keys_command_data_firmware_t{
	WIRELESS_KEY_command_data_firmware_version = 0x00,
	WIRELESS_KEY_command_data_firmware_start,
	WIRELESS_KEY_command_data_firmware_lines,
	WIRELESS_KEY_command_data_firmware_total,
	WIRELESS_KEY_command_data_firmware_data = 0x10,
};
//M3 80-8f
enum wireless_keys_commandM3_data_t{
	WIRELESS_KEY_command_data_settings_00 = 0x80,
	WIRELESS_KEY_command_data_settings_01= 0x81,
	WIRELESS_KEY_command_data_settings_02 = 0x82,
	WIRELESS_KEY_command_data_settings_03 = 0x83,
	WIRELESS_KEY_command_data_settings_04 = 0x84,
	WIRELESS_KEY_command_data_settings_05 = 0x85,
	WIRELESS_KEY_command_data_settings_06 = 0x86,
};

//error
enum wireless_keys_error_t{
	WIRELESS_KEY_error_code = 0x00,
	WIRELESS_KEY_error_message = 0x01,
};
enum wireless_error_code_t{
	WIRELESS_ERROR_code_noContents = 1000,
	WIRELESS_ERROR_code_noDeviceId = 2000,
	WIRELESS_ERROR_code_noVersion = 3000,
	WIRELESS_ERROR_code_noVersion_hadware = 3001,
	WIRELESS_ERROR_code_noVersion_firmware = 3002,
	WIRELESS_ERROR_code_sessionIdTimeout = 4000,
};


enum wireless_lineType_t{
	WIRELESS_LINE_TYPE_download,
	WIRELESS_LINE_TYPE_upload,
	WIRELESS_LINE_TYPE_uploadGenerator,
};


typedef void (* wireless_responseFunction)(struct wireless_t*, package_element_t* key, package_element_t* request, package_element_t* response);
typedef void (* wireless_commandFunction)(struct wireless_t*, package_element_t* data, uint32_t delayed, uint8_t by);
typedef void (* wireless_dataFunction)(struct wireless_t*, package_element_t* key, package_element_t* value);
typedef void (* wireless_lineChangedFunction)(struct wireless_t*, uint8_t type, uint16_t line);

typedef void (* wireless_routeFunction)(struct wireless_t*, char* address, uint16_t port);
typedef void (* wireless_commonFunction)(struct wireless_t*);
typedef uint8_t (* wireless_subPackageFunction)(struct wireless_t*, package_element_t* upload, package_element_t* download);

typedef struct wireless_t {
	wireless_responseFunction responseFunction;
	wireless_commandFunction commandFunction;
	wireless_dataFunction dataFunction;
	wireless_lineChangedFunction lineChangedFunction;
	wireless_commandFunction acceptedFunction;
	wireless_routeFunction routeFunction;
	wireless_commonFunction initFunction;
	wireless_subPackageFunction subPackageFunction;
	uint16_t downloadLine;
	uint16_t uploadLine;
	uint16_t uploadLineGenerator;
	// uint8_t hardwareType;
	// void *hardwarePointer;
	package_element_t* uploadPackage;
	list_t* subPackages;
	// package_element_t* downloadPackage;
} wireless_t;

typedef struct wireless_socket_t {
	uint32_t id;
	uint8_t type;
	uint8_t connected;
	char* host;
	uint16_t port;
} wireless_socket_t;

typedef struct wireless_httpRequest_t {
	char* path;
	uint8_t method;
	list_t* headers;
	uint8_t* body;
	int16_t size;
} wireless_httpRequest_t;

typedef struct wireless_httpResponse_t {
	uint8_t status;
	list_t* headers;
	uint8_t* body;
	int16_t size;
} wireless_httpResponse_t;

package_element_t* wireless_safeAddObject(package_element_t* e, wireless_key_t key, package_element_t* valueE);
uint8_t wireless_keyEqual(package_element_t* a, wireless_key_t b);
uint8_t wireless_setDeviceId(wireless_t* wireless, char* deviceId);
uint8_t wireless_setVersion(wireless_t* wireless, char const* hardware, char const* firmware, char const* protocol);

package_element_t* wireless_getRequest(wireless_t* wireless, wireless_key_t key);
int16_t wireless_getRequestsCount(wireless_t* wireless);
int16_t wireless_getCommandsCount(wireless_t* wireless);
uint8_t wireless_addRequest(wireless_t* wireless, package_element_t* key, package_element_t* value);
uint8_t wireless_addCommand(wireless_t* wireless, package_element_t* key, package_element_t* value, uint32_t delayed);
uint8_t wireless_addData(wireless_t* wireless, package_element_t* key, package_element_t* value);
void wireless_received(wireless_t* wireless, uint8_t* buffer, uint16_t size);
int16_t wireless_fetchBytes(wireless_t* wireless, uint32_t time, uint8_t** out);
void wireless_init(wireless_t* wireless);

#endif
