#define JAN_1970 0x83aa7e80 /* 2208988800 1970 - 1900 in seconds */

#define NTP_TO_UNIX(n,u) do {  u = n - JAN_1970; } while (0)

#define NTOHL_FP(n, h)  do { (h)->l_ui = ntohl((n)->l_ui); \
	(h)->l_uf = ntohs((n)->l_uf); } while (0)

typedef enum
{
	LEAPINDICATOR_NO_WARNING = 0,
	LEAPINDICATOR_61_SECOND_DAY,
	LEAPINDICATOR_59_SECOND_DAY,
	LEAPINDICATOR_ALARM,
}
leapIndicator_e;

typedef enum
{
	MODE_RESERVED = 0,
	MODE_SYMMETRIC_ACTIVE,
	MODE_SYMMETRIC_PASSIVE,
	MODE_CLIENT,
	MODE_SERVER,
	MODE_BROADCAST,
	MODE_NTP_CONTROL_MSG,
	MODE_PRIVATE,
}
mode_e;


#define  SNTP_UDP_PORT   123

#define l_ui Ul_i.Xl_ui /* unsigned integral part */
#define l_i  Ul_i.Xl_i  /* signed integral part */
#define l_uf Ul_f.Xl_uf /* unsigned fractional part */
#define l_f  Ul_f.Xl_f  /* signed fractional part */


typedef struct
{
	union
	{
		uint32_t Xl_ui;
		int32_t Xl_i;
	}
	Ul_i;

	union
	{
		uint32_t Xl_uf;
		int32_t Xl_f;
	}
	Ul_f;
}
__attribute__ ((packed)) l_fp;

typedef struct
{
	uint16_t mode          : 3;
	uint16_t versionNumber : 3;
	uint16_t leapIndicator : 2;
	uint16_t stratum       : 8;
	uint16_t poll          : 8;
	uint16_t precision     : 8;
	uint32_t rootDelay;
	uint32_t rootDispersion;
	uint32_t refID;
	l_fp  refTimeStamp;
	l_fp  orgTimeStamp;
	l_fp  rxTimeStamp;
	l_fp  txTimeStamp;
}__attribute__ ((packed)) ntphdr_t;

