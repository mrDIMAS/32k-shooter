#pragma once


class Timer {
private:
	double mLastTime;
public:
	Timer();
	~Timer();
	void Restart( );
	float GetTimeInSeconds( ) const;
	float GetTimeInMilliSeconds( ) const;
	float GetTimeInMicroSeconds( ) const;
	float GetElapsedTimeInSeconds( ) const;
	float GetElapsedTimeInMilliSeconds( ) const;
	float GetElapsedTimeInMicroSeconds( ) const;
};