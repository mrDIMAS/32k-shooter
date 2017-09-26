#pragma once

#include "engine.h"
#include "cstring.h"

// GUI uses virtual canvas with fixed size - 1024x768 pixels

class GUIFont {
public:	
	HFONT mFont;
	int mBase;
	static int msCount;

	void Select();
	void DrawString( const char * str );
	GUIFont( int size );
	~GUIFont();
};

class GUINode {
public:
	MakeList( GUINode );

	Math::Color mColor;
	float mLX, mLY;
	float mGX, mGY;
	float mWidth, mHeight;

	bool mVisible;
	GUINode * mParent;
	Array<GUINode*> mChildren;

	GUINode( );
	virtual ~GUINode();
	virtual void FixedUpdate();
	virtual void Render( );
	void AttachTo( GUINode * parent );
	void CalculateTransforms();
	void SetLocalPosition( float x, float y );
	void SetSize( float w, float h );
	bool IsVisible();
	static void FixedUpdateAll();
};

class GUILine : public GUINode {
public:
	float mThickness;

	GUILine();
	virtual void Render();
};

class GUIRect : public GUINode {
public:
	bool mStroke;
	float mStrokeWidth;
	Math::Color mStrokeColor;
	GUIRect( float x, float y, float w, float h, const Math::Color & color );
	virtual void Render( );	
};

class GUIText : public GUINode {
public:
	enum class Alignment {
		Left,
		Center,
	};

	Alignment mAlignment;
	GUIFont * mFont;
	char mText[2048]; // Fixed buffer size, enough for most purposes

	GUIText( GUIFont * font, const char * text );
	virtual void Render( );
};

typedef void (*GUICallback)( );

class GUIButton : public GUIRect {
public:
	GUIText * mText;
	GUICallback OnClick;
	bool mMouseInside;
	GUIButton( float x, float y, float w, float h, const Math::Color & color, const char * text, GUIFont * font );
	virtual void Render();
	virtual void FixedUpdate();
};

class GUISelect {
public:
	GUIText * mText;
	GUIButton * mIncrease;
	GUIButton * mDecrease;

	GUISelect( GUINode * parent, float x, float y, float w, float h, const Math::Color & color, const char * text, GUIFont * font );
	void SetCallbacks( GUICallback onDecrease, GUICallback onIncrease );
};