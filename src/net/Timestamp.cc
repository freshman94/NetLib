#include <net/Timestamp.h>

#include <sys/time.h>
#include <stdio.h>
#include <inttypes.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

static_assert(sizeof(Timestamp) == sizeof(int64_t),
	"Timestamp is same size as int64_t");

string Timestamp::toString() const {
	char buf[32] = { 0 };
	int64_t seconds = microSecsSinceEpoch_ / MicroSecsPerSec;
	int64_t microseconds = microSecsSinceEpoch_ % MicroSecsPerSec;
	snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
	return buf;
}

string Timestamp::toFormattedString() const {
	char buf[64] = { 0 };
	time_t seconds = static_cast<time_t>(microSecsSinceEpoch_ / MicroSecsPerSec);
	int microseconds = static_cast<int>(microSecsSinceEpoch_ % MicroSecsPerSec);

	struct tm tm_time;
	gmtime_r(&seconds, &tm_time);

	
	snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
		tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
		tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
		microseconds);

	return buf;
}

/*
struct timeval
{
__time_t tv_sec;        //Seconds.
__suseconds_t tv_usec;  //Microseconds.
};
*/
Timestamp Timestamp::now(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t seconds = tv.tv_sec;
	return Timestamp(seconds * MicroSecsPerSec + tv.tv_usec);
}

