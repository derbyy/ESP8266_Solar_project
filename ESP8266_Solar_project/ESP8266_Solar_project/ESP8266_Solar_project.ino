/*
Name:		ESP8266_Firmware.ino
Created:	3/31/2018 6:38:55 PM
Author:	Martin
*/
#include <ESP8266WiFi.h>
#include "arduino.h"
#include <WiFiClient.h>

/*********************************** DEFINES ****************************************/

//Serial port connection speed
#define	BAUD_RATE					9600

//Maximal response time from server
#define	HTTP_TIMEOUT				10000

//Server port number
#define	SERVER_PORT_NUMBER			80

//URL of server (google.sk)
#define	SERVER_URL					"www.google.sk"

//HTTP resource
#define HTTP_RESOURCE				"/"

//Timeout
#define INTERVAL_MS					100000

//Day offset
#define DAY_OFFSET					5

//Macro for finding leap year
#define LEAP_YEAR(Y)				( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ))

//GMT+1 constant
#define PRAHA_GMT_TIME_SHIFT		1

//Daylight saving offset - from March to October + 1 hour
#define DAYLIGHT_SAVING				1

//Number of days in year
#define NUM_OF_DAYS_IN_YEAR			365

//Number of days in leaped year
#define NUM_OF_DAYS_IN_YEAR_LEAPED	366

/*************************************************************************************/

//Array of Sunrise hours for every days in normal year
static const uint8_t u8SunriseArrHour[ NUM_OF_DAYS_IN_YEAR ] =
{ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7 };

//Array of Sunrise minutes for every days in normal year
static const uint8_t u8SunriseArrMinutes[ NUM_OF_DAYS_IN_YEAR ] =
{ 36, 36, 36, 35, 35, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 29, 28, 27, 26,
26, 25, 24, 22, 21, 20, 19, 18, 17, 15, 14, 13, 11, 10,  8,  7,  5,  4,  2,  1,
59, 57, 56, 54, 52, 51, 49, 47, 45, 43, 42, 40, 38, 36, 34, 32, 30, 28, 26, 24,
22, 20, 18, 16, 14, 12, 10,  8,  6,  4,  2,  0, 58, 56, 53, 51, 49, 47, 45, 43,
41, 39, 37, 34, 32, 30, 28, 26, 24, 22, 20, 17, 15, 13, 11,  9,  7,  5,  3,  1,
59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33, 32, 30, 28, 26, 24, 23,
21, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,  0, 59, 58, 56, 55, 54, 53,
52, 50, 49, 48, 47, 46, 45, 45, 44, 43, 42, 41, 41, 40, 39, 39, 38, 38, 37, 37,
37, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 38, 38, 39, 39,
40, 40, 41, 41, 42, 43, 44, 45, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 57,
58, 59,  0,  1,  2,  4,  5,  6,  8,  9, 10, 12, 13, 14, 16, 17, 18, 20, 21, 22,
24, 25, 27, 28, 30, 31, 32, 34, 35, 37, 38, 39, 41, 42, 44, 45, 47, 48, 50, 51,
52, 54, 55, 57, 58,  0,  1,  2,  4,  5,  7,  8, 10, 11, 12, 14, 15, 17, 18, 20,
21, 22, 24, 25, 27, 28, 30, 31, 33, 34, 35, 37, 38, 40, 41, 43, 44, 46, 47, 49,
50, 52, 53, 55, 56, 58, 59,  1,  2,  4,  6,  7,  9, 10, 12, 13, 15, 17, 18, 20,
21, 23, 24, 26, 28, 29, 31, 33, 34, 36, 37, 39, 41, 42, 44, 45, 47, 48, 50, 52,
53, 55, 56, 58, 59,  1,  2,  4,  5,  7,  8,  9, 11, 12, 13, 15, 16, 17, 18, 20,
21, 22, 23, 24, 25, 26, 27, 28, 28, 29, 30, 31, 31, 32, 33, 33, 34, 34, 34, 35,
35, 35, 35, 35, 36 };

//Array of Sunset hours for every days in normal year
static const uint8_t u8SunsetArrayHour[ NUM_OF_DAYS_IN_YEAR ] =
{ 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
17, 17, 17, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18,
18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15 };

//Array of Sunset minutes for every days in normal year
static const uint8_t u8SunsetArrayMinutes[ NUM_OF_DAYS_IN_YEAR ] =
{ 53, 54, 55, 56, 57, 59,  0,  1,  2,  3,  5,  6,  7,  9, 10, 12, 13, 14, 16, 17,
19, 21, 22, 24, 25, 27, 28, 30, 32, 33, 35, 37, 38, 40, 42, 43, 45, 47, 48, 50,
52, 53,  55, 57, 58, 0,  2,  3,  5,  6,  8, 10, 11, 13, 15, 16, 18, 20, 21, 23,
24, 26, 28, 29, 31, 32, 34, 35, 37, 39, 40, 42, 43, 45, 46, 48, 49, 51, 52, 54,
55, 57, 59,  0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 15, 17, 18, 20, 21, 23, 24,
26, 27, 29, 30, 32, 33, 35, 36, 38, 39, 41, 42, 44, 45, 47, 48, 50, 51, 53, 54,
56, 57, 59,  0,  1,  3,  4,  6,  7,  9, 10, 11, 13, 14, 16, 17, 18, 20, 21, 22,
24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 42, 43,
44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50,
49, 49, 49, 49, 48, 48, 48, 47, 47, 46, 45, 45, 44, 43, 42, 42, 41, 40, 39, 38,
37, 36, 35, 34, 32, 31, 30, 29, 27, 26, 25, 23, 22, 20, 19, 17, 16, 14, 13, 11,
9,  8,  6,  4,  2,  1, 59, 57, 55, 53, 51, 49, 48, 46, 44, 42, 40, 38, 36, 34,
32, 30, 28, 26, 24, 22, 19, 17, 15, 13, 11,  9,  7,  5,  3,  0, 58, 56, 54, 52,
50, 48, 45, 43, 41, 39, 37, 35, 33, 30, 28, 26, 24, 22, 20, 18, 16, 13, 11,  9,
7,  5,  3,  1, 59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 38, 36, 34, 32, 30,
28, 27, 25, 23, 22, 20, 18, 17, 15, 14, 12, 11,  9,  8,  6,  5,  4,  2,  1,  0,
59, 58, 56, 55, 54, 53, 52, 51, 51, 50, 49, 48, 48, 47, 46, 46, 45, 45, 45, 44,
44, 44, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 46, 46, 47, 47, 48,
49, 49, 50, 51, 52 };

//Array of Sunrise hours for every days in Leaped year
static const uint8_t u8SunriseArrayHours_Leaped[ NUM_OF_DAYS_IN_YEAR_LEAPED ] =
{ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7 };

//Array of Sunrise minutes for every days in Leaped year
static const uint8_t u8SunriseArrayMinutes_Leaped[ NUM_OF_DAYS_IN_YEAR_LEAPED ] =
{ 36, 36, 36, 35, 35, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 29, 28, 27, 26,
26, 25, 24, 22, 21, 20, 19, 18, 17, 15, 14, 13, 11, 10,  8,  7,  5,  4,  2,  1,
59, 57, 56, 54, 52, 51, 49, 47, 45, 43, 42, 40, 38, 36, 34, 32, 30, 28, 26, 24,
22, 20, 18, 16, 14, 12, 10,  8,  6,  4,  2,  0, 58, 56, 53, 51, 49, 47, 45, 43,
41, 39, 37, 34, 32, 30, 28, 26, 24, 22, 20, 17, 15, 13, 11,  9,  7,  5,  3,  1,
59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33, 32, 30, 28, 26, 24, 23,
21, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,  0, 59, 58, 56, 55, 54, 53,
52, 50, 49, 48, 47, 46, 45, 45, 44, 43, 42, 41, 41, 40, 39, 39, 38, 38, 37, 37,
37, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 38, 38, 39, 39,
40, 40, 41, 41, 42, 43, 44, 45, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 57,
58, 59,  0,  1,  2,  4,  5,  6,  8,  9, 10, 12, 13, 14, 16, 17, 18, 20, 21, 22,
24, 25, 27, 28, 30, 31, 32, 34, 35, 37, 38, 39, 41, 42, 44, 45, 47, 48, 50, 51,
52, 54, 55, 57, 58,  0,  1,  2,  4,  5,  7,  8, 10, 11, 12, 14, 15, 17, 18, 20,
21, 22, 24, 25, 27, 28, 30, 31, 33, 34, 35, 37, 38, 40, 41, 43, 44, 46, 47, 49,
50, 52, 53, 55, 56, 58, 59,  1,  2,  4,  6,  7,  9, 10, 12, 13, 15, 17, 18, 20,
21, 23, 24, 26, 28, 29, 31, 33, 34, 36, 37, 39, 41, 42, 44, 45, 47, 48, 50, 52,
53, 55, 56, 58, 59,  1,  2,  4,  5,  7,  8,  9, 11, 12, 13, 15, 16, 17, 18, 20,
21, 22, 23, 24, 25, 26, 27, 28, 28, 29, 30, 31, 31, 32, 33, 33, 34, 34, 34, 35,
35, 35, 35, 35, 36, 36, };

//Array of Sunset hours for every days in Leaped year
static const uint8_t u8SunsetArrayHours_Leaped[ NUM_OF_DAYS_IN_YEAR_LEAPED ] =
{ 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
17, 17, 17, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
20, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18,
18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 16, 16,
16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15 };

//Array of Sunset minutes for every days in Leaped year
static const uint8_t u8SunsetArrayMinutes_Leaped[ NUM_OF_DAYS_IN_YEAR_LEAPED ] =
{ 53, 54, 55, 56, 57, 59,  0,  1,  2,  3,  5,  6,  7,  9, 10, 12, 13, 14, 16, 17,
19, 21, 22, 24, 25, 27, 28, 30, 32, 33, 35, 37, 38, 40, 42, 43, 45, 47, 48, 50,
52, 53, 55, 57, 58,  0,  2,  3,  5,  6,  8, 10, 11, 13, 15, 16, 18, 20, 21, 23,
24, 26, 28, 29, 31, 32, 34, 35, 37, 39, 40, 42, 43, 45, 46, 48, 49, 51, 52, 54,
55, 57, 59,  0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 15, 17, 18, 20, 21, 23, 24,
26, 27, 29, 30, 32, 33, 35, 36, 38, 39, 41, 42, 44, 45, 47, 48, 50, 51, 53, 54,
56, 57, 59,  0,  1,  3,  4,  6,  7,  9, 10, 11, 13, 14, 16, 17, 18, 20, 21, 22,
24, 25, 26, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 42, 43,
44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50,
49, 49, 49, 49, 48, 48, 48, 47, 47, 46, 45, 45, 44, 43, 42, 42, 41, 40, 39, 38,
37, 36, 35, 34, 32, 31, 30, 29, 27, 26, 25, 23, 22, 20, 19, 17, 16, 14, 13, 11,
9,  8,  6,  4,  2,  1, 59, 57, 55, 53, 51, 49, 48, 46, 44, 42, 40, 38, 36, 34,
32, 30, 28, 26, 24, 22, 19, 17, 15, 13, 11,  9,  7,  5,  3,  0, 58, 56, 54, 52,
50, 48, 45, 43, 41, 39, 37, 35, 33, 30, 28, 26, 24, 22, 20, 18, 16, 13, 11,  9,
7,  5,  3,  1, 59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 38, 36, 34, 32, 30,
28, 27, 25, 23, 22, 20, 18, 17, 15, 14, 12, 11,  9,  8,  6,  5,  4,  2,  1,  0,
59, 58, 56, 55, 54, 53, 52, 51, 51, 50, 49, 48, 48, 47, 46, 46, 45, 45, 45, 44,
44, 44, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 46, 46, 47, 47, 48,
49, 49, 50, 51, 52, 53 };

static unsigned char const crc8_table[] = {
	0xea, 0xd4, 0x96, 0xa8, 0x12, 0x2c, 0x6e, 0x50, 0x7f, 0x41, 0x03, 0x3d,
	0x87, 0xb9, 0xfb, 0xc5, 0xa5, 0x9b, 0xd9, 0xe7, 0x5d, 0x63, 0x21, 0x1f,
	0x30, 0x0e, 0x4c, 0x72, 0xc8, 0xf6, 0xb4, 0x8a, 0x74, 0x4a, 0x08, 0x36,
	0x8c, 0xb2, 0xf0, 0xce, 0xe1, 0xdf, 0x9d, 0xa3, 0x19, 0x27, 0x65, 0x5b,
	0x3b, 0x05, 0x47, 0x79, 0xc3, 0xfd, 0xbf, 0x81, 0xae, 0x90, 0xd2, 0xec,
	0x56, 0x68, 0x2a, 0x14, 0xb3, 0x8d, 0xcf, 0xf1, 0x4b, 0x75, 0x37, 0x09,
	0x26, 0x18, 0x5a, 0x64, 0xde, 0xe0, 0xa2, 0x9c, 0xfc, 0xc2, 0x80, 0xbe,
	0x04, 0x3a, 0x78, 0x46, 0x69, 0x57, 0x15, 0x2b, 0x91, 0xaf, 0xed, 0xd3,
	0x2d, 0x13, 0x51, 0x6f, 0xd5, 0xeb, 0xa9, 0x97, 0xb8, 0x86, 0xc4, 0xfa,
	0x40, 0x7e, 0x3c, 0x02, 0x62, 0x5c, 0x1e, 0x20, 0x9a, 0xa4, 0xe6, 0xd8,
	0xf7, 0xc9, 0x8b, 0xb5, 0x0f, 0x31, 0x73, 0x4d, 0x58, 0x66, 0x24, 0x1a,
	0xa0, 0x9e, 0xdc, 0xe2, 0xcd, 0xf3, 0xb1, 0x8f, 0x35, 0x0b, 0x49, 0x77,
	0x17, 0x29, 0x6b, 0x55, 0xef, 0xd1, 0x93, 0xad, 0x82, 0xbc, 0xfe, 0xc0,
	0x7a, 0x44, 0x06, 0x38, 0xc6, 0xf8, 0xba, 0x84, 0x3e, 0x00, 0x42, 0x7c,
	0x53, 0x6d, 0x2f, 0x11, 0xab, 0x95, 0xd7, 0xe9, 0x89, 0xb7, 0xf5, 0xcb,
	0x71, 0x4f, 0x0d, 0x33, 0x1c, 0x22, 0x60, 0x5e, 0xe4, 0xda, 0x98, 0xa6,
	0x01, 0x3f, 0x7d, 0x43, 0xf9, 0xc7, 0x85, 0xbb, 0x94, 0xaa, 0xe8, 0xd6,
	0x6c, 0x52, 0x10, 0x2e, 0x4e, 0x70, 0x32, 0x0c, 0xb6, 0x88, 0xca, 0xf4,
	0xdb, 0xe5, 0xa7, 0x99, 0x23, 0x1d, 0x5f, 0x61, 0x9f, 0xa1, 0xe3, 0xdd,
	0x67, 0x59, 0x1b, 0x25, 0x0a, 0x34, 0x76, 0x48, 0xf2, 0xcc, 0x8e, 0xb0,
	0xd0, 0xee, 0xac, 0x92, 0x28, 0x16, 0x54, 0x6a, 0x45, 0x7b, 0x39, 0x07,
	0xbd, 0x83, 0xc1, 0xff };




static bool bDebug = false;
static bool bOneLoop = false;
static uint8_t u8LoopCount = 0;
static WiFiClient client;

//static const char* ssid = "FRIPI";
//static const char* password = "Vypalen52";

static const char* ssid = "ASUS";
static const char* password = "791382465";


//HAL Functions
void HAL_vInitSerialPort( void );
void HAL_vConnectWiFi( void );
bool HAL_bConnect( const char* pu8HostName, const uint16_t u16Port );
void HAL_vDisconnect( void );
bool HAL_bSendRequest( const char* pu8Host, const char* pu8Resource );

typedef struct DATE_TIME
{
	uint16_t u16Year;
	uint8_t u8Month;
	uint8_t u8Day;
	uint8_t u8Hour;
	uint8_t u8Minute;
}DATE_TIME_T;

static String dateAndTime;
static bool s_bIsSummerTime = false;
static bool s_bIsLeapYear = false;
static DATE_TIME_T s_sDateTime;

//Functions for extract Date and Time from website
void DT_vShowDateAndTime();
void HAL_vSendDateTime();
bool DT_bFindDateAndTimeInResponseHeaders( void );
bool bNeedToCHangeTime( uint8_t u8Day, uint8_t u8DayOfWeek );
bool bGetTimeStatus( uint16_t u16Year, uint8_t u8Month, uint8_t u8Day );
unsigned char u8_Crc8( String strData );
String DT_strExtractDayFromDateTimeString( String strDateTime );
String DT_strExtractMonthFromDateTimeString( String strDateTime );
String DT_strExtractYearFromDateTimeString( String strDateTime );
String DT_strExtractHourFromDateTimeString( String strDateTime );
String DT_strExtractMinuteFromDateTimeString( String strDateTime );
String DT_strTranslateMonth( String strMonthStr );
String DT_GetSunriseTime( uint8_t u8Month, uint8_t u8Day );
String DT_GetSunsetTime( uint8_t u8Month, uint8_t u8Day );
uint8_t DT_u8GetDayOfWeek( uint16_t u16Year, uint8_t u8Month, uint8_t u8Day );
uint16_t DT_u16GetDayInYear( uint8_t u8Month, uint8_t u8Day );

static unsigned long previousMillis = 0;


/**************************** HAL FUNCTIONS ***********************************/
//Init serial port
void HAL_vInitSerialPort()
{
	//Set GPIO15 as an output
	pinMode( D8, OUTPUT );
	//Initialize UART interface
	Serial.begin( BAUD_RATE );
	//Swap to alternative pins(GPIO13 = Rx, GPIO15 = Tx)
	Serial.swap( D8 );
	while( !Serial )
	{
		;  // wait for serial port to initialize
	}
	if( bDebug )
	{
		Serial.println( "Serial ready" );
	}
}

// Attempt to connect to WiFi
void HAL_vConnectWiFi()
{
	WiFi.mode( WIFI_STA );
	// connect to the WiFi network
	WiFi.begin( ssid, password );
	while( WiFi.status() != WL_CONNECTED )
	{
		delay( 500 );
		if( bDebug )
		{
			Serial.print( "." );
		}
	}
	if( bDebug )
	{
		Serial.println( "" );
		Serial.println( "WiFi connected" );
		Serial.print( "IP address: " );
		Serial.println( WiFi.localIP() );
	}
}

// Open connection to the HTTP server
bool HAL_bConnect( const char* pu8HostName, const uint16_t u16Port )
{
	if( bDebug )
	{
		Serial.print( "Connect to " );
		Serial.println( pu8HostName );
	}
	bool ok = client.connect( pu8HostName, u16Port );
	if( bDebug )
	{
		Serial.println( ok ? "Connected" : "Connection Failed!" );
	}
	return ok;
}

// Close the connection with the HTTP server
void HAL_vDisconnect()
{
	if( bDebug )
	{
		Serial.println( "Disconnect from HTTP server" );
	}
	client.stop();
}

// Send the HTTP GET request to the server 
bool HAL_bSendRequest( const char* pu8Host, const char* pu8Resource )
{
	if( bDebug )
	{
		Serial.print( "GET " );
		Serial.println( pu8Resource );
	}
	client.print( "GET " );
	client.print( pu8Resource );
	client.println( " HTTP/1.1" );
	client.print( "Host: " );
	client.println( pu8Host );
	client.println( "Accept: */*" );
	client.println( "Connection: close" );
	client.println();
	return true;
}

void HAL_vSendDateTime()
{
	if( bDebug )
	{
		Serial.println( "loop: get date and time" );
	}

	if( HAL_bConnect( SERVER_URL, SERVER_PORT_NUMBER ) )
	{
		if( HAL_bSendRequest( SERVER_URL, HTTP_RESOURCE ) )
		{
			if( DT_bFindDateAndTimeInResponseHeaders() )
			{
				if( bDebug )
				{
					Serial.print( "Date and Time from HTTP response header: " );
					Serial.println( dateAndTime.c_str() );
				}
				DT_vShowDateAndTime();
			}
		}
		HAL_vDisconnect();
	}
}
/*************************************************************************/




/************************** DATE & TIME FUNCTIONS*************************/
//Function for get date and time string from HTTP page
bool DT_bFindDateAndTimeInResponseHeaders()
{
	client.setTimeout( HTTP_TIMEOUT );
	char header[ 85 ];
	size_t length = client.readBytes( header, 85 );
	header[ length ] = 0;
	String headerString = String( header );
	int ds = headerString.indexOf( "Date: " );
	int de = headerString.indexOf( "GMT" );
	dateAndTime = headerString.substring( ds + 6, de );

	if( bDebug )
	{
		Serial.print( "HTTP response header " );
		Serial.println( headerString.c_str() );
		Serial.print( "index start date " );
		Serial.println( ds );
		Serial.print( "index end time " );
		Serial.println( de );
		Serial.println();
		Serial.print( "date and time: " );
		Serial.println( dateAndTime.c_str() );
	}
	return dateAndTime.length();
}

// Function for extract Day from string
String DT_strExtractDayFromDateTimeString( String strDateTime )
{
	uint8_t firstSpace = strDateTime.indexOf( ' ' );
	String dayStr = strDateTime.substring( firstSpace + 1, firstSpace + 3 );
	if( bDebug )
	{
		Serial.print( "Day: " );
		Serial.println( dayStr.c_str() );
	}
	s_sDateTime.u8Day = (uint8_t) dayStr.toInt();
	if( s_sDateTime.u8Day < 10 )
	{
		dayStr = String( '0' + s_sDateTime.u8Day );
	}
	return dayStr;
}

//Function for extract Month from string
String DT_strExtractMonthFromDateTimeString( String strDateTime )
{
	uint8_t firstSpace = strDateTime.indexOf( ' ', 7 );
	uint8_t u8_MonthNum;
	String monthStr = strDateTime.substring( firstSpace + 1, firstSpace + 4 );
	if( bDebug )
	{
		Serial.print( "Month: " );
		Serial.println( monthStr.c_str() );
	}
	u8_MonthNum = ( DT_strTranslateMonth( monthStr ) ).toInt();
	s_sDateTime.u8Month = u8_MonthNum;

	return monthStr;
}

//Function for extract Year from string
String DT_strExtractYearFromDateTimeString( String strDateTime )
{
	uint8_t firstSpace = strDateTime.indexOf( ' ', 10 );
	String yearStr = strDateTime.substring( firstSpace + 1, firstSpace + 5 );
	if( bDebug )
	{
		Serial.print( "Year: " );
		Serial.println( yearStr.c_str() );
	}
	s_sDateTime.u16Year = (uint16_t) yearStr.toInt();

	//Find if year is Leap or Common
	if( LEAP_YEAR( s_sDateTime.u16Year ) )
	{
		s_bIsLeapYear = true;
	}

	return yearStr;
}

//Function for extract Hour from string
String DT_strExtractHourFromDateTimeString( String strDateTime )
{
	uint8_t firstColon = strDateTime.indexOf( ':' );
	uint8_t u8DayOfWeek;
	bool bSummerTime = false;
	String hourStr = strDateTime.substring( firstColon, firstColon - 2 );

	if( bDebug )
	{
		Serial.print( "Hour (GMT): " );
		Serial.println( hourStr.c_str() );
	}

	//Add DayLight saving into hours
	bSummerTime = bGetTimeStatus( s_sDateTime.u16Year, s_sDateTime.u8Month, s_sDateTime.u8Day );

	//Adjust GMT time + DayLight saving
	s_bIsSummerTime ? s_sDateTime.u8Hour = (uint8_t) hourStr.toInt() + PRAHA_GMT_TIME_SHIFT + 1 :
		s_sDateTime.u8Hour = (uint8_t) hourStr.toInt() + PRAHA_GMT_TIME_SHIFT;

	if( bDebug )
	{
		Serial.print( "Hour (adjusted for summertime): " );
		Serial.println( s_sDateTime.u8Hour );
	}
	return s_sDateTime.u8Hour < 10 ? "0" + String( s_sDateTime.u8Hour ) : String( s_sDateTime.u8Hour );
}

//Function for extract Minute from string
String DT_strExtractMinuteFromDateTimeString( String strDateTime )
{
	uint8_t secondColon = strDateTime.lastIndexOf( ':' );
	String minuteStr = strDateTime.substring( secondColon, secondColon - 2 );
	if( bDebug )
	{
		Serial.print( "Minute: " );
		Serial.println( minuteStr.c_str() );
	}
	return minuteStr;
}

//Function for extract Seconds from string
String DT_strExtractSecondsFromDateTimeString( String strDateTime )
{
	uint8_t secondColon = strDateTime.lastIndexOf( ':' );
	String secondsString = strDateTime.substring( secondColon + 1, secondColon + 3 );
	if( bDebug )
	{
		Serial.print( "Second: " );
		Serial.println( secondsString.c_str() );
	}

	return secondsString;
}

//Function for translate Month string into number
String DT_strTranslateMonth( String strMonthStr )
{
	if( strMonthStr.equals( String( "Jan" ) ) ) return String( "01" );
	if( strMonthStr.equals( String( "Feb" ) ) ) return String( "02" );
	if( strMonthStr.equals( String( "Mar" ) ) ) return String( "03" );
	if( strMonthStr.equals( String( "Apr" ) ) ) return String( "04" );
	if( strMonthStr.equals( String( "May" ) ) ) return String( "05" );
	if( strMonthStr.equals( String( "Jun" ) ) ) return String( "06" );
	if( strMonthStr.equals( String( "Jul" ) ) ) return String( "07" );
	if( strMonthStr.equals( String( "Aug" ) ) ) return String( "08" );
	if( strMonthStr.equals( String( "Sep" ) ) ) return String( "09" );
	if( strMonthStr.equals( String( "Oct" ) ) ) return String( "10" );
	if( strMonthStr.equals( String( "Nov" ) ) ) return String( "11" );
	if( strMonthStr.equals( String( "Dec" ) ) ) return String( "12" );
}

String DT_GetSunriseTime( uint8_t u8Month, uint8_t u8Day )
{
	uint16_t u16DayOfYear = DT_u16GetDayInYear( u8Month, u8Day );
	String sunriseTime = "0";

	if( s_bIsLeapYear )
	{
		sunriseTime = sunriseTime + String( u8SunriseArrayHours_Leaped[ u16DayOfYear ] ) + "-";
		u8SunriseArrayMinutes_Leaped[ u16DayOfYear ] < 10 ?
			sunriseTime = sunriseTime + "0" + String( u8SunriseArrayMinutes_Leaped[ u16DayOfYear ] ) :
			sunriseTime += String( u8SunriseArrayMinutes_Leaped[ u16DayOfYear ] );
	}
	else
	{
		sunriseTime = sunriseTime + String( u8SunriseArrHour[ u16DayOfYear ] ) + "-";
		u8SunriseArrMinutes[ u16DayOfYear ] < 10 ?
			sunriseTime = sunriseTime + "0" + String( u8SunriseArrMinutes[ u16DayOfYear ] ) :
			sunriseTime += String( u8SunriseArrMinutes[ u16DayOfYear ] );
	}
	return sunriseTime;
}

String DT_GetSunsetTime( uint8_t u8Month, uint8_t u8Day )
{
	uint16_t u16DayOfYear = DT_u16GetDayInYear( u8Month, u8Day );
	String sunsetTime = "";

	if( s_bIsLeapYear )
	{
		sunsetTime = sunsetTime + String( u8SunsetArrayHours_Leaped[ u16DayOfYear ] ) + "-";
		u8SunsetArrayMinutes_Leaped[ u16DayOfYear ] < 10 ?
			sunsetTime = sunsetTime + "0" + String( u8SunsetArrayMinutes_Leaped[ u16DayOfYear ] ) :
			sunsetTime += String( u8SunsetArrayMinutes_Leaped[ u16DayOfYear ] );
	}
	else
	{
		sunsetTime = sunsetTime + String( u8SunsetArrayHour[ u16DayOfYear ] ) + "-";
		u8SunsetArrayMinutes_Leaped[ u16DayOfYear ] < 10 ?
			sunsetTime = sunsetTime + "0" + String( u8SunsetArrayMinutes[ u16DayOfYear ] ) :
			sunsetTime += String( u8SunsetArrayMinutes[ u16DayOfYear ] );
	}
	return sunsetTime;
}

//Function for create string with date
void DT_vShowDateAndTime()
{
	String strDate = "";
	unsigned char u8crc = 0x00;

	strDate += DT_strExtractYearFromDateTimeString( dateAndTime );
	strDate += "-";
	strDate += DT_strTranslateMonth( DT_strExtractMonthFromDateTimeString( dateAndTime ) );
	strDate += "-";
	strDate += DT_strExtractDayFromDateTimeString( dateAndTime );
	strDate += "-";
	strDate += DT_strExtractHourFromDateTimeString( dateAndTime );
	strDate += "-";
	strDate += DT_strExtractMinuteFromDateTimeString( dateAndTime );
	strDate += "-";
	strDate += DT_strExtractSecondsFromDateTimeString( dateAndTime );
	strDate += "-";
	strDate += DT_GetSunriseTime( s_sDateTime.u8Month, s_sDateTime.u8Day );
	strDate += "-";
	strDate += DT_GetSunsetTime( s_sDateTime.u8Month, s_sDateTime.u8Day );
	strDate += "-";
	u8crc = u8_Crc8( strDate );
	strDate = (uint8_t) u8crc < 0x10 ? strDate + "0" + String( u8crc, HEX ) : strDate + String( u8crc, HEX );

	if( bDebug )
	{
		Serial.println( u8crc );
	}
	strDate += '\n';
	Serial.print( strDate );
}

//Function for find Day of Week
uint8_t DT_u8GetDayOfWeek( uint16_t u16Year, uint8_t u8Month, uint8_t u8Day )
{
	//Constants for all months in year based on Sakamoto, Lachman, Keith and Craver Day of Week algorithm
	const uint8_t u8Slkc_Consts[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

	u16Year -= u8Month < 3;
	return( u16Year + u16Year / 4 - u16Year / 100 + u16Year / 400 + u8Slkc_Consts[ u8Month - 1 ] + u8Day ) % 7;
}

//Function for geting day in year
uint16_t DT_u16GetDayInYear( uint8_t u8Month, uint8_t u8Day )
{
	uint16_t u16NumOfDaysMon = 0;
	uint8_t u8NumOfDay = 0;
	uint8_t u8NumOfDaysInMonthArr[ 13 ] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if( s_bIsLeapYear )
	{
		u8NumOfDaysInMonthArr[ 2 ] = u8NumOfDaysInMonthArr[ 2 ] + 1;
	}
	for( uint8_t u8Cnt = 0; u8Cnt < u8Month; u8Cnt++ )
	{
		u16NumOfDaysMon += u8NumOfDaysInMonthArr[ u8Cnt ];
	}
	u8NumOfDay = u8Day;

	return ( u16NumOfDaysMon + (uint16_t) u8NumOfDay );
}


//Finction find if Daylight saving time was applied if is reset between 25-31 March or October 
bool bNeedToCHangeTime( uint8_t u8Day, uint8_t u8DayOfWeek )
{
	bool bWasChangerd = false;
	const uint8_t u8ChangeBorder = 25;

	if( ( u8Day - u8DayOfWeek ) > u8ChangeBorder )
	{
		bWasChangerd = true;
	}

	return bWasChangerd;
}

bool bGetTimeStatus( uint16_t u16Year, uint8_t u8Month, uint8_t u8Day )
{
	bool bIsDaySavingTime = false;
	uint8_t u8DayOfWeek = DT_u8GetDayOfWeek( u16Year, u8Month, u8Day );

	if( u8Month < 3 || u8Month > 10 )
	{
		bIsDaySavingTime = false;
	}
	else
	{
		if( u8Month == 3 && u8Day < 25 )
		{
			bIsDaySavingTime = false;
		}
		else if( u8Month == 3 && u8Day >= 25 )
		{
			bNeedToCHangeTime( u8Day, u8DayOfWeek ) ? bIsDaySavingTime = true : bIsDaySavingTime = false;
		}
		else if( u8Month == 10 && u8Day >= 25 )
		{
			bNeedToCHangeTime( u8Day, u8DayOfWeek ) ? bIsDaySavingTime = false : bIsDaySavingTime = true;
		}
		else
		{
			bIsDaySavingTime = true;
		}
	}
	if( s_bIsSummerTime != bIsDaySavingTime )
	{
		s_bIsSummerTime = bIsDaySavingTime;
	}
	return bIsDaySavingTime;
}


unsigned char u8_Crc8( String strData )
{
	unsigned char crc = 0x00;

	if( strData == NULL )
	{
		return 0;
	}
	crc &= 0xff;
	for( uint8_t u8Cnt = 0; u8Cnt < strData.length(); u8Cnt++ )
	{
		crc = crc8_table[ crc ^ strData[ u8Cnt ] ];
		if( bDebug )
		{
			Serial.print( crc, HEX );
			Serial.print( " " );
			Serial.println( strData[ u8Cnt ] );
		}
	}
	return crc;
}
/**********************************************************************************/



void setup()
{
	if( bDebug )
	{
		Serial.println( "setup" );
	}
	HAL_vInitSerialPort();
	HAL_vConnectWiFi();

	delayMicroseconds( 500 );

	if( HAL_bConnect( SERVER_URL, SERVER_PORT_NUMBER ) )
	{
		if( HAL_bSendRequest( SERVER_URL, HTTP_RESOURCE ) )
		{
			if( DT_bFindDateAndTimeInResponseHeaders() )
			{
				if( bDebug )
				{
					Serial.print( "Date and Time from HTTP response header: " );
					Serial.println( dateAndTime.c_str() );
				}
				DT_vShowDateAndTime();
			}
		}
		HAL_vDisconnect();
	}
	delayMicroseconds( 500 );

	if( bDebug )
	{
		Serial.println( "Im going into Deep sleep..." );
	}
	ESP.deepSleep( 0 );
}

void loop()
{
	//uint32_t currentMillis = millis();
	//// run every 10 seconds
	//if( currentMillis - previousMillis >= INTERVAL_MS )
	//{
	//	previousMillis = currentMillis;
	//	if( bDebug )
	//	{
	//		Serial.println( "loop: get date and time" );
	//	}

	//	if( HAL_bConnect( SERVER_URL, SERVER_PORT_NUMBER ) )
	//	{
	//		if( HAL_bSendRequest( SERVER_URL, HTTP_RESOURCE ) )
	//		{
	//			if( DT_bFindDateAndTimeInResponseHeaders() )
	//			{
	//				if( bDebug )
	//				{
	//					Serial.print( "Date and Time from HTTP response header: " );
	//					Serial.println( dateAndTime.c_str() );
	//				}
	//				DT_vShowDateAndTime();
	//			}
	//		}
	//		HAL_vDisconnect();
	//	}
	//}
}