#include "precompiled.h"
#include "time.h"


LARGE_INTEGER freq;

float Timer::GetElapsedTimeInMicroSeconds() const
{
	return GetTimeInMicroSeconds() - mLastTime;
}

float Timer::GetElapsedTimeInMilliSeconds() const
{
	return GetTimeInMilliSeconds() - mLastTime / 1000.0;
}

float Timer::GetElapsedTimeInSeconds() const
{
	return GetTimeInSeconds() - mLastTime / 1000000.0;
}

float Timer::GetTimeInMicroSeconds() const
{
	LARGE_INTEGER time;
	QueryPerformanceCounter ( &time );
	return ( float ) ( time.QuadPart * 1000000.0 ) / ( float ) ( freq.QuadPart );
}

float Timer::GetTimeInMilliSeconds() const
{
	LARGE_INTEGER time;
	QueryPerformanceCounter ( &time );
	return ( float ) ( time.QuadPart * 1000.0 ) / ( float ) ( freq.QuadPart );
}

float Timer::GetTimeInSeconds() const
{
	LARGE_INTEGER time;
	QueryPerformanceCounter ( &time );
	return ( float ) ( time.QuadPart ) / ( float ) ( freq.QuadPart );
}

void Timer::Restart() {
	mLastTime = GetTimeInMicroSeconds();
}

Timer::Timer() {
	QueryPerformanceFrequency ( &freq );
	Restart();
}

Timer::~Timer()
{

}
