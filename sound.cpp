#include "precompiled.h"
#include "sound.h"
#include "engine.h"

float discreteNoise[1024];

IDirectSound8* gDSound8;
IDirectSoundBuffer* primaryBuffer;
IDirectSound3DListener8* gListener;
Sound * Sound::msRoot;

Buffer * gExplosionBuffer;
Buffer * gFootstepsBuffer[4];
Buffer * gPlasmaSound;
Buffer * gRailSound;
Buffer * gPickUpSound;
Buffer * gShotgunSound;
Buffer * gMinigunSound;
Buffer * gRocketSound;

double IntegerNoise (int n) {
	n = (n >> 13) ^ n;
	int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
	return 1.0 - ((double)nn / 1073741824.0);
}


void GenerateDiscreteNoise() {
	for( int i = 0; i < 1024; ++i ) {
		discreteNoise[i]=IntegerNoise(i);
	};
}

float sigma( float x ) {
	return x/(1.0f+Math::FAbs(x));
};

float noise0( float t ){ 
	return discreteNoise[((unsigned int)(t+0.5f))&1023];
}

float noise1( float t ){
	unsigned int n = (unsigned int)t & 1023;
	float x = t - Math::Floor(t);
	return discreteNoise[n] * ( 1.0f - x ) + discreteNoise[(n+1)&1023]*x;
}

float noise3( float t ){
	unsigned int n = (unsigned int)t & 1023;
	float x = t - Math::Floor(t);
	return (1.0f/2.0f)*((x*((2.0f-x)*x-1.0f)) *
		discreteNoise[(n+1023)&1023]+(x*x*(3*x-5)+2)*
		discreteNoise[n]+(x*((4.0f-3.0f*x)*x+1))*
		discreteNoise[(n+1)&1023]+((x-1.0f)*x*x)*
		discreteNoise[(n+2)&1023]);
}

float GenerateFootsteps( float t, float A, float B, float C, float D, float E, float F, float G ) {
	float a=1000*Math::Exp(10*(A-0.5));
	float b=37*Math::Exp(10*(B-0.5));
	float c=10*Math::Exp(10*(C-0.5));
	float d=0.025*Math::Exp(10*(D-0.5));
	float e=0.5*Math::Exp(10*(E-0.5));
	float f=0.7*Math::Exp(10*(F-0.5));
	float g=5000*Math::Exp(10*(G-0.5));
	float x = noise3(a* Math::Sqrt(0.1+t))*Math::Exp(-b*t);
	x+=d*noise3(g*t)*Math::Exp(-c*t);
	if(t>0.1) {
		x+=e*noise3(f*a*Math::Sqrt(0.1+t-0.1))*Math::Exp(-b*(t-0.1));
		x+=e*d*noise3(g*(t-0.1))*Math::Exp(-c*(t-0.1));
	}
	x*=0.25;
	return x;
}

float GenerateExplosion( float t, float A, float B, float C, float D, float E, float F, float G ) {
	float a=1*Math::Exp(10*(A+0.44140625-1));
	float b=1*Math::Exp(10*(B+0.40625-1));
	float c=1*Math::Exp(10*(C+0.5390625-1));
	float d=1*Math::Exp(10*(D+0.5234375-1));
	float e=1*Math::Exp(10*(E+0.44921875-1));
	float y=0;
	y+=Math::Sin(a*(730*t+5*noise1(270*t)));
	y+=noise1(b*370*t);
	y+=noise1(b*500*t);
	y+=noise1(b*730*t);
	y*=Math::Exp(-0.25*e*t);
	y+=(noise3(c*1000*t)/2+
		noise3(c*1700*t)/2+
		noise1(c*3700*t)/2)*noise3(d*250*t);
	y*=Math::Exp(-e*t * (1+5*G));
	y*=sigma(500*t);
	y *= 1.0;
	return y;
}

float GenerateDrip( float t, float A, float B, float C, float D, float E, float F, float G ) {
	// Drip
	float p[]={0.5,-0.125,0.25};
	float q[]={0,0.02,0.125};
	float a=1000*Math::Exp(5*(A-0.5));
	float b=100*Math::Exp(5*(B-0.5));
	float c=0.125*Math::Exp(5*(C-0.5));
	float d=0.005*Math::Exp(5*(D-0.5));
	float y=0;
	for(int i=0;i<3;++i) {
		float x=0;
		float t2 = t-q[i];
		x=sigma(t2/d)*Math::Sin(2*Math::Pi*a/(b*c)*Math::Exp(c*b*t2))*Math::Exp(-b*t2);
		y+= ( t > q[i] ? p[i] * x : 0 );
	}
	return y;
};

float GenerateCoin( float t, float A, float B, float C, float D, float E, float F, float G ) {
	// Coin
	float a=7000*Math::Exp(5*(A-0.5));
	float b=35*Math::Exp(5*(B-0.5));
	float x,y;
	x=Math::Sin(2*Math::Pi*a*t);
	y=1-Math::Fmod(b*t+2*noise3(2*b/5*t),1);
	y*=y*y;
	return x*y*Math::Exp(-5*t);
};

float GenerateWave( float t, float A, float B, float C, float D, float E, float F, float G ) {
	return (Math::Sin( t * 500 ) * noise3( t * 1200 )  * Math::Exp( t ) + Math::Sin( t * 300 ) * Math::Atan( t * 2.5 )) * Math::Exp( 1 - t * 15 );
}

float GenerateRail( float t, float A, float B, float C, float D, float E, float F, float G ) {
	float a = Math::Sin( t * 1200 ) * Math::Exp( 1 - t * 12 );
	float b = Math::Sin( t * 2200 ) * Math::Exp( 1 - t * 5 );
	float c = Math::Sin( t * 2200 ) * Math::Exp( 1 - t * 8 ) ;
	float d = Math::Sin( t * 1200 ) * Math::Exp( 1 - t * 12 );

	return 0.25f * ( a + b - c * d ) * Math::Cos( t * 3.25 );
}

Sound::Sound( Buffer * buf, bool autoDelete ) : mAutoDelete( autoDelete ), 
	mPrev( nullptr ), mNext( nullptr ), m3DBuffer( nullptr ), mBuffer( nullptr ) 
{
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;
	unsigned char* bufferPtr;
	unsigned long bufferSize;

	int dataSize = buf->mSampleCount * sizeof( short );
	unsigned char * data = new unsigned char[ dataSize ];
	short * samples16 = (short*)data;	
	for( int i = 0; i < buf->mSampleCount; ++i ) {
		samples16[i] = buf->mSamples[i] * SHRT_MAX;		
	}

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 1;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D | DSBCAPS_GLOBALFOCUS;
	bufferDesc.dwBufferBytes = dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	if( FAILED( gDSound8->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL))) {
		mBuffer = nullptr;
		m3DBuffer = nullptr;
		return;
	}

	if( FAILED( tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&mBuffer ))) {
		mBuffer = nullptr;
		m3DBuffer = nullptr;
		return;
	}

	tempBuffer->Release();
	tempBuffer = 0;

	if( FAILED( mBuffer->Lock( 0, dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0  ))) {
		mBuffer->Release();
		mBuffer = nullptr;
		m3DBuffer = nullptr;
		return;
	}

	Memory::Copy( bufferPtr, data, dataSize);

	if( FAILED( mBuffer->Unlock((void*)bufferPtr, dataSize, NULL, 0 ))) {
		mBuffer->Release();
		mBuffer = nullptr;
		m3DBuffer = nullptr;
		delete [] data;
		return;
	}

	delete [] data;

	if( FAILED( mBuffer->QueryInterface(IID_IDirectSound3DBuffer8, (void**)&m3DBuffer ))) {
		mBuffer->Release();
		mBuffer = nullptr;
		m3DBuffer = nullptr;
		return;
	}

	LinkedList::Append( msRoot, this );
}

Sound::~Sound() {
	LinkedList::Extract( msRoot, this );
	while( m3DBuffer->Release()) {};
	while( mBuffer->Release()) {};
}

void Sound::InitSoundSystem() {
	HRESULT result;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;

	if( FAILED( DirectSoundCreate8(NULL, &gDSound8, NULL ))) {
		gDSound8 = nullptr;
		return;
	}

	if( FAILED( gDSound8->SetCooperativeLevel( gEngine->mWindow, DSSCL_PRIORITY ))) {
		gDSound8->Release();
		gDSound8 = nullptr;
		return;
	}

	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D ;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	if( FAILED(	gDSound8->CreateSoundBuffer(&bufferDesc, &primaryBuffer, NULL ))) {
		gDSound8->Release();
		gDSound8 = nullptr;
		gListener = nullptr;
		return;
	}

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	if( FAILED(	primaryBuffer->SetFormat( &waveFormat ))) {
		gDSound8->Release();
		gDSound8 = nullptr;
		gListener = nullptr;
		primaryBuffer = nullptr;
		return;
	}

	if( FAILED( primaryBuffer->QueryInterface(IID_IDirectSound3DListener8, (LPVOID*)&gListener ))) {
		gDSound8->Release();
		gDSound8 = nullptr;
		gListener = nullptr;
		primaryBuffer = nullptr;
		return;
	}

	GenerateDiscreteNoise();

	gExplosionBuffer = new Buffer( 1.5, GenerateExplosion, 0.15f, 0.55f, 0.35f, 0.85f, 0.73f, 1.0f, 0.0f );
	gFootstepsBuffer[0] = new Buffer( 0.5, GenerateFootsteps, 0.5f, 0.5f, 0.5f, 0.25f, 0.5f, 0.5f, 0.17f  );
	gFootstepsBuffer[1] = new Buffer( 0.5, GenerateFootsteps, 0.5f, 0.5f, 0.35f, 0.5f, 0.55f, 0.5f, 0.17f  );
	gFootstepsBuffer[2] = new Buffer( 0.5, GenerateFootsteps, 0.55f, 0.5f, 0.5f, 0.55f, 0.35f, 0.55f, 0.17f  );
	gFootstepsBuffer[3] = new Buffer( 0.5, GenerateFootsteps, 0.5f, 0.55f, 0.55f, 0.5f, 0.5f, 0.45f, 0.17f  );
	gPlasmaSound = new Buffer( 1.5, GenerateWave, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f );
	gRailSound = new Buffer( 0.5, GenerateDrip, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f );
	gShotgunSound = new Buffer( 0.77, GenerateExplosion, 0.171f, 0.644f, 0.453f, 0.67f, 0.80f, 0.0f, 0.0f );
	gMinigunSound = new Buffer( 0.09, GenerateExplosion, 0.171f, 0.644f, 0.453f, 0.67f, 0.80f, 0.0f, 0.0f );
	gRocketSound = new Buffer( 0.5, GenerateExplosion, 0.171f, 0.644f, 0.453f, 0.67f, 0.80f, 0.0f, 0.4f );
	gPickUpSound = new Buffer( 2.0, GenerateCoin,1, 0, 0.453f, 0.67f, 0.80f, 0.0f, 0.4f );
}

void Sound::Play() {
	if( mBuffer ) {
		mBuffer->Play( 0, 0, 0 );
	}
}

void Sound::SetPosition( const Math::Vector3 & pos ) {
	if( m3DBuffer ) {
		m3DBuffer->SetPosition( pos.x, pos.y, pos.z, DS3D_IMMEDIATE );
	}
}

bool Sound::IsPlaying() {
	if( mBuffer ) {
		DWORD status;
		mBuffer->GetStatus( &status );
		return status == DSBSTATUS_PLAYING;
	} 
	return false;
}

void Sound::UpdateAll() {
	auto sound = msRoot;
	while( sound ) {
		auto next = sound->mNext;
		if( sound->mAutoDelete ) {
			if( !sound->IsPlaying() ) {
				delete sound;
			}
		}
		sound = next;
	}
}

Buffer::Buffer( float length, GenFunc f, float A, float B, float C, float D, float E, float F, float G ) : mSampleCount( 0 ), mSamples( nullptr ) {
	mSampleCount = SampleRate * length;
	mSamples = new float[ mSampleCount ];
	float dt = 1.0f / (float)SampleRate;
	float t = 0.0f;
	for( int i = 0; i < mSampleCount; ++i ) {
		mSamples[i] = f( t, A, B, C, D, E, F, G );
		if( mSamples[i] < -1.0f ) {
			mSamples[i] = -1.0f;
		} 
		if( mSamples[i] > 1.0f ) {
			mSamples[i] = 1.0;
		}
		t += dt;
	}
	float pole = 0.9f;
	float b0 = 1.0f - pole;
	float a1 = -pole;
	float last = 0;
	for( int i = 0; i < mSampleCount; ++i ) {
		last = b0 * mSamples[i] - a1 * last;
		mSamples[i] = last;
	}
	
}


