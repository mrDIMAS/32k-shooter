#include "precompiled.h"
#include "gui.h"

int GUIFont::msCount = 1;
GUINode * GUINode::msRoot;
Math::Vector2 gGUIGlobalScale;

void GUIFont::Select() {
	SelectObject( gEngine->mDeviceContext, mFont );
}

void GUIFont::DrawString( const char * str ) {
	Select();
	GLCall( glListBase( mBase ));
	GLCall( glCallLists( String::Length( str ), GL_UNSIGNED_BYTE, str ));
}

GUIFont::GUIFont( int size ) : mBase( msCount * 255 ) {
	mFont = CreateFontA( size * gGUIGlobalScale.x, size * gGUIGlobalScale.y / ( 1.5f + 2.0f - gAspect ),0,0,FW_MEDIUM,FALSE,FALSE,FALSE,RUSSIAN_CHARSET,OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial" );                
	SelectObject( gEngine->mDeviceContext, mFont );    
	glGetError();
	wglUseFontBitmapsA( gEngine->mDeviceContext, 0, 255, mBase );
	++msCount;
}

GUIFont::~GUIFont() {
	SelectObject( gEngine->mDeviceContext, 0 );
	DeleteObject( mFont );
}

GUINode::GUINode() : mNext( nullptr ), mPrev( nullptr ), mVisible( true ), mParent( nullptr ), 
	mGX( 0.0f ), mGY( 0.0f ), mLY( 0.0f ), mLX( 0.0f ), mWidth( 0.0f ), mHeight( 0.0f )
{
	LinkedList::Append( msRoot, this );
}

GUINode::~GUINode() {
	LinkedList::Extract( msRoot, this );
}

void GUINode::FixedUpdate()
{

}

void GUINode::Render() {

}

void GUINode::AttachTo( GUINode * parent ) {
	mParent = parent;
	mParent->mChildren.Append( this );
	CalculateTransforms();
	mParent->CalculateTransforms();
}

void GUINode::CalculateTransforms() {
	if( mParent ) {
		mGX = mParent->mGX + mLX;
		mGY = mParent->mGY + mLY;
	} else {
		mGX = mLX;
		mGY = mLY;
	}

	for( int i = 0; i < mChildren.mCount; ++i ) {
		mChildren[i]->CalculateTransforms();
	}
}

void GUINode::SetLocalPosition( float x, float y ) {
	mLX = x * gGUIGlobalScale.x;
	mLY = y * gGUIGlobalScale.y;
	CalculateTransforms();
}

void GUINode::SetSize( float w, float h ) {
	mWidth = w * gGUIGlobalScale.x;
	mHeight = h * gGUIGlobalScale.y;
}

bool GUINode::IsVisible() {
	bool visible = mVisible;
	if( mParent ) {
		visible &= mParent->IsVisible();
	}
	return visible;
}

void GUINode::FixedUpdateAll()
{
	auto node = msRoot;
	while( node ) {
		node->FixedUpdate();
		node = node->mNext;
	}
}

void GUIRect::Render() {
	glColor4ub( mColor.mRed, mColor.mGreen, mColor.mBlue, mColor.mAlpha );

	glBegin( GL_QUADS );

	glVertex2f( mGX, mGY );
	glTexCoord2f( 0.0f, 0.0f );

	glVertex2f( mGX + mWidth, mGY );
	glTexCoord2f( 1.0f, 0.0f );

	glVertex2f( mGX + mWidth, mGY + mHeight );
	glTexCoord2f( 1.0f, 1.0f );

	glVertex2f( mGX, mGY + mHeight );
	glTexCoord2f( 0.0f, 1.0f );

	GLCall( glEnd());

	if( mStroke ) {
		glColor4ub( mStrokeColor.mRed, mStrokeColor.mGreen, mStrokeColor.mBlue, mStrokeColor.mAlpha );

		glLineWidth( mStrokeWidth );
		glBegin( GL_LINE_LOOP );

		glVertex2f( mGX, mGY );
		glTexCoord2f( 0.0f, 0.0f );

		glVertex2f( mGX + mWidth, mGY );
		glTexCoord2f( 1.0f, 0.0f );

		glVertex2f( mGX + mWidth, mGY + mHeight );
		glTexCoord2f( 1.0f, 1.0f );

		glVertex2f( mGX, mGY + mHeight );
		glTexCoord2f( 0.0f, 1.0f );

		GLCall( glEnd());

		glPointSize( mStrokeWidth );
		glBegin( GL_POINTS );

		glVertex2f( mGX, mGY );
		glTexCoord2f( 0.0f, 0.0f );

		glVertex2f( mGX + mWidth, mGY );
		glTexCoord2f( 1.0f, 0.0f );

		glVertex2f( mGX + mWidth, mGY + mHeight );
		glTexCoord2f( 1.0f, 1.0f );

		glVertex2f( mGX, mGY + mHeight );
		glTexCoord2f( 0.0f, 1.0f );

		GLCall( glEnd());
	}
}

GUIRect::GUIRect( float x, float y, float w, float h, const Math::Color & color ) : 
	mStroke( false ), mStrokeColor( 112, 146, 190, 255 ), mStrokeWidth( 5 ) {
	SetLocalPosition( x, y );
	SetSize( w, h );
	mColor = color;
	CalculateTransforms();
}

GUIText::GUIText( GUIFont * font, const char * text ) : mFont( font ), mAlignment( Alignment::Left ) {
	String::Copy( mText, text );
}

static char buffer [2048];
void GUIText::Render() {
	SIZE metrics;

	mFont->Select();

	glColor4ub( mColor.mRed, mColor.mGreen, mColor.mBlue, mColor.mAlpha );

	float x = mGX, y = mGY;

	String::Copy( buffer, mText );
	char * substring = String::Tokenize( buffer, "\n" );
	while( substring ) {
		if( mAlignment == Alignment::Center ) {
			GetTextExtentPoint32A( gEngine->mDeviceContext, substring, String::Length( substring ), &metrics );
			glRasterPos2d( x + mWidth / 2 - metrics.cx / 2, y + mHeight / 2 + metrics.cy / 2 ); 
		} else {
			glRasterPos2d( x, y ); 
		}

		mFont->DrawString( substring );
		substring = String::Tokenize( nullptr, "\n" );		

		y += metrics.cy;
	}
}

GUIButton::GUIButton( float x, float y, float w, float h, const Math::Color & color, const char * text, GUIFont * font ) : GUIRect( x, y, w, h, color ), mMouseInside( false ) {
	mText = new GUIText( font, text );
	mText->SetSize( w, h );
	mText->AttachTo( this );
	mText->mAlignment = GUIText::Alignment::Center;
	OnClick = nullptr;

	mStroke = true;
	mStrokeColor = Math::Color( 112, 146, 190, 255 );
}

void GUIButton::Render() {
	GUIRect::Render();
	if( gMouse.x > mGX && gMouse.x < ( mGX + mWidth ) && gMouse.y > mGY && gMouse.y < ( mGY + mHeight ) ) {
		mText->mColor = Math::Color( 255, 0, 0, 255 );
	} else {
		mText->mColor = Math::Color( 255, 255, 255, 255 );
	}
}

void GUIButton::FixedUpdate() {
	if( IsVisible() ) {		
		if( gMouse.x > mGX && gMouse.x < ( mGX + mWidth ) && gMouse.y > mGY && gMouse.y < ( mGY + mHeight ) ) {	
			if( !mMouseInside ) {
				Sound * snd = new Sound( gFootstepsBuffer[0] );
				snd->Play();
				mMouseInside = true;
			}
			if( gMouse.buttonPressed[0] ) {

				if( OnClick ) {
					OnClick();
				}	
			}
		} else {
			mMouseInside = false;
		}		
	}
}

GUILine::GUILine() : mThickness( 3.0f ) {
}

void GUILine::Render()
{
	glLineWidth( mThickness );
	glBegin( GL_LINES );
	glColor4ub( mColor.mRed, mColor.mGreen, mColor.mBlue, mColor.mAlpha );
	glVertex2f( mGX, mGY );
	glVertex2f( mGX + mWidth, mGY + mHeight );
	glEnd();
}

GUISelect::GUISelect( GUINode * parent, float x, float y, float w, float h, const Math::Color & color, const char * text, GUIFont * font )
{
	mText = new GUIText( font, text );
	mText->AttachTo( parent );
	mText->SetSize( w, 32 );
	mText->mAlignment = GUIText::Alignment::Center;
	mText->SetLocalPosition( 0, y );

	mDecrease = new GUIButton( x - 64 - 32, y, 32, h, color, "<", font );
	mDecrease->AttachTo( parent );
	mDecrease->OnClick = nullptr;

	mIncrease = new GUIButton( x + 64, y, 32, h, color, ">", font );
	mIncrease->AttachTo( parent );
	mIncrease->OnClick = nullptr;
}

void GUISelect::SetCallbacks( GUICallback onDecrease, GUICallback onIncrease )
{
	mDecrease->OnClick = onDecrease;
	mIncrease->OnClick = onIncrease;
}
