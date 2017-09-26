#pragma once

extern IDirectSound3DListener8* gListener;

#define SampleRate 44100

typedef float (*GenFunc)( float t, float A, float B, float C, float D, float E, float F, float G );

class Buffer {
public:
	float * mSamples;
	int mSampleCount;

	Buffer( float length, GenFunc f, float A, float B, float C, float D, float E, float F, float G );
};

extern Buffer * gExplosionBuffer;
extern Buffer * gFootstepsBuffer[4];
extern Buffer * gPlasmaSound;
extern Buffer * gRailSound;
extern Buffer * gPickUpSound;
extern Buffer * gShotgunSound;
extern Buffer * gMinigunSound;
extern Buffer * gRocketSound;
class Sound {
public:
	MakeList( Sound );

	IDirectSoundBuffer8 * mBuffer;
	IDirectSound3DBuffer8 * m3DBuffer;

	bool mAutoDelete;
	Sound( Buffer * buf, bool autoDelete = true );
	~Sound();
	static void InitSoundSystem();
	void Play();
	void SetPosition( const Math::Vector3 & pos );
	bool IsPlaying();
	static void UpdateAll();
};