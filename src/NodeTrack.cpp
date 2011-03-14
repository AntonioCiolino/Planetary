/*
 *  NodeTrack.cpp
 *  Bloom
 *
 *  Created by Robert Hodgin on 1/21/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "cinder/app/AppBasic.h"
#include "NodeAlbum.h"
#include "NodeTrack.h"
#include "cinder/Text.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "Globals.h"


using namespace ci;
using namespace ci::ipod;
using namespace std;

NodeTrack::NodeTrack( Node *parent, int index, const Font &font )
	: Node( parent, index, font )
{	
	mIsHighlighted	= true;
	mSphereRes		= 16;
	mRadius			*= 4.0f;
	mIsPlaying		= false;
	mHasRings		= false;
	
	// FIXME: bad C++?
	float numTracks = ((NodeAlbum*)mParentNode)->mNumTracks;
	float invTrackPer = 1.0f/numTracks;
	float trackNumPer = (float)mIndex * invTrackPer;
	
	float minAmt		= mParentNode->mRadius * 2.0f;
	float maxAmt		= mParentNode->mRadius * 8.0f;
	float deltaAmt		= maxAmt - minAmt;
	mOrbitRadiusDest	= minAmt + deltaAmt * trackNumPer + Rand::randFloat( maxAmt * invTrackPer );
	
}

void NodeTrack::setData( TrackRef track, PlaylistRef album )
{
	mAlbum			= album;
	mTrack			= track;
	mTrackLength	= (*mAlbum)[mIndex]->getLength();
	mPlayCount		= (*mAlbum)[mIndex]->getPlayCount();
	mStarRating		= (*mAlbum)[mIndex]->getStarRating();
	
	/*
	for( int i=0; i<mStarRating * 4; i++ ){
		mOrbiters.push_back( Orbiter( this, i ) );
	}
	*/

	/*
	mAlbumArt		= mTrack->getArtwork( Vec2i( 256, 256 ) );
	if( !mAlbumArt ){
		mAlbumArt	= gl::Texture( 256, 256 );
	}
	*/
	
	
	//normalize playcount data
	float playCountDelta	= ( mParentNode->mHighestPlayCount - mParentNode->mLowestPlayCount ) + 1.0f;
	float normPlayCount		= ( mPlayCount - mParentNode->mLowestPlayCount )/playCountDelta;
	
    /*
	std::cout << "playCount = " << mPlayCount << std::endl;
	std::cout << "highestPlayCount = " << mParentNode->mHighestPlayCount << std::endl;
	std::cout << "lowestPlayCount = " << mParentNode->mLowestPlayCount << std::endl;
	std::cout << "normPlayCount = " << normPlayCount << std::endl;
	*/
    
	// try making own texture for ringed planet. texture stripe, maybe from album art?
	mPlanetTexIndex = mIndex%( G_NUM_PLANET_TYPES * G_NUM_PLANET_TYPE_OPTIONS );//(int)( normPlayCount * ( G_NUM_PLANET_TYPES - 1 ) );
	mCloudTexIndex  = Rand::randInt( G_NUM_CLOUD_TYPES );
   // mPlanetTexIndex *= G_NUM_PLANET_TYPE_OPTIONS + Rand::randInt( G_NUM_PLANET_TYPE_OPTIONS );

	
	if( mParentNode->mHighestPlayCount == mPlayCount && mPlayCount > 5 )
		mHasRings = true;
	
	float hue		= Rand::randFloat( 0.15f, 0.75f );
	float sat		= Rand::randFloat( 0.15f, 0.25f );
	float val		= Rand::randFloat( 0.85f, 1.00f );
	mColor			= Color( CM_HSV, hue, sat, val );
	mGlowColor		= mColor;
	mAtmosphereColor = mParentNode->mColor;
	mEclipseColor	= mColor;
	
	mRadius			= math<float>::max( mRadius * pow( normPlayCount + 0.5f, 2.0f ), 0.0003f ) * 0.75;
	mSphere			= Sphere( mPos, mRadius * 8.5f );
	mIdealCameraDist = 0.004;//mRadius * 10.0f;
	mOrbitPeriod	= mTrackLength;
	mAxialTilt		= Rand::randFloat( 5.0f, 30.0f );
    mAxialVel       = Rand::randFloat( 10.0f, 45.0f );
	
	mVerts			= new float[18];
	mTexCoords		= new float[12];
	int i = 0;
	int t = 0;
	Vec3f corner;
	float w	= mRadius * 3.35f;
	
	corner			= Vec3f( -w, 0.0f, -w );
	mVerts[i++]			= corner.x;
	mVerts[i++]			= corner.y;
	mVerts[i++]			= corner.z;
	mTexCoords[t++]		= 0.0f;
	mTexCoords[t++]		= 0.0f;
	
	corner			= Vec3f( w, 0.0f, -w );
	mVerts[i++]			= corner.x;
	mVerts[i++]			= corner.y;
	mVerts[i++]			= corner.z;
	mTexCoords[t++]		= 1.0f;
	mTexCoords[t++]		= 0.0f;

	corner			= Vec3f( w, 0.0f, w );	
	mVerts[i++]			= corner.x;
	mVerts[i++]			= corner.y;
	mVerts[i++]			= corner.z;
	mTexCoords[t++]		= 1.0f;
	mTexCoords[t++]		= 1.0f;
	
	corner			= Vec3f( -w, 0.0f, -w );
	mVerts[i++]			= corner.x;
	mVerts[i++]			= corner.y;
	mVerts[i++]			= corner.z;
	mTexCoords[t++]		= 0.0f;
	mTexCoords[t++]		= 0.0f;
	
	corner			= Vec3f( w, 0.0f, w );	
	mVerts[i++]			= corner.x;
	mVerts[i++]			= corner.y;
	mVerts[i++]			= corner.z;
	mTexCoords[t++]		= 1.0f;
	mTexCoords[t++]		= 1.0f;

	corner			= Vec3f( -w, 0.0f, w );	
	mVerts[i++]			= corner.x;
	mVerts[i++]			= corner.y;
	mVerts[i++]			= corner.z;
	mTexCoords[t++]		= 0.0f;
	mTexCoords[t++]		= 1.0f;
}

void NodeTrack::update( const Matrix44f &mat )
{	
	double playbackTime		= app::getElapsedSeconds();
	double percentPlayed	= playbackTime/mOrbitPeriod;
	double orbitAngle		= percentPlayed * TWO_PI + mStartAngle;
	
	mPrevPos	= mTransPos;
	mRelPos		= Vec3f( cos( orbitAngle ), sin( orbitAngle ), 0.0f ) * mOrbitRadius;
	mPos		= mParentNode->mPos + mRelPos;

	if( mIsSelected ){
		mSphereRes		-= ( mSphereRes - 32 ) * 0.1f;
		mCamDistAlpha	-= ( mCamDistAlpha - 1.0f ) * 0.05f;
	} else {
		mSphereRes		-= ( mSphereRes - 14 ) * 0.1f;
		mCamDistAlpha	-= ( mCamDistAlpha - 0.0f ) * 0.05f;
	}
	
	if( mStarRating > 0 && mIsSelected ){
		vector<Orbiter>::iterator it;
		for( it = mOrbiters.begin(); it != mOrbiters.end(); ++it ){
			it->update( mat, mTransPos );
		}
	}
	
	float c = 1.0f;
	if( G_ZOOM == G_TRACK_LEVEL && mIsSelected ) c = 1.0f - ( mParentNode->mParentNode->mEclipsePer * 0.35f );
	mEclipseColor = mColor * c;
	
	
	Node::update( mat );
	
	mVel		= mTransPos - mPrevPos;	
}

void NodeTrack::drawAtmosphere()
{
	if( mCamDistAlpha > 0.05f && mPlanetTexIndex > 0 ){
		Vec3f perp = mBbRight.cross( mBbUp );
		gl::color( ColorA( mParentNode->mColor, mCamDistAlpha + mParentNode->mEclipsePer ) );
		Vec2f radius = Vec2f( mRadius, mRadius ) * 2.0f;
		gl::drawBillboard( mTransPos + perp * mRadius * 0.1f, radius, 0.0f, mBbRight, mBbUp );
	}
}

void NodeTrack::drawPlanet( const Matrix44f &accelMatrix, const vector<gl::Texture> &planets )
{	
	gl::pushModelView();
	gl::translate( mTransPos );
	gl::rotate( mMatrix );
	gl::rotate( Vec3f( 90.0f, app::getElapsedSeconds() * mAxialVel, mAxialTilt ) );
	
	gl::disableAlphaBlending();
	
	gl::color( mEclipseColor );
	planets[mPlanetTexIndex].enableAndBind();
	gl::drawSphere( Vec3f::zero(), mRadius, (int)mSphereRes );
	
	gl::popModelView();
	
    if( mIsSelected )
		drawOrbiters();
}

void NodeTrack::drawClouds( const Matrix44f &accelMatrix, const vector<gl::Texture> &clouds )
{
	if( mCamDistAlpha > 0.05f ){
		gl::pushModelView();
		gl::translate( mTransPos );
		gl::rotate( mMatrix );
		gl::rotate( Vec3f( 90.0f, app::getElapsedSeconds() * mAxialVel * 0.6f, mAxialTilt ) );
		gl::disableAlphaBlending();
		gl::enableAlphaBlending();
		
		clouds[mCloudTexIndex].enableAndBind();
		gl::color( ColorA( 0.0f, 0.0f, 0.0f, mCamDistAlpha * 0.66f ) );
		gl::drawSphere( Vec3f::zero(), mRadius + 0.000006f, (int)mSphereRes );

		gl::enableAdditiveBlending();
		gl::color( ColorA( mEclipseColor, mCamDistAlpha ) );
		gl::drawSphere( Vec3f::zero(), mRadius + 0.000012f, (int)mSphereRes );
		 
		gl::popModelView();
	}
}

void NodeTrack::drawRings( const gl::Texture &tex )
{
	if( mHasRings ){
		gl::pushModelView();
		gl::translate( mTransPos );
		gl::rotate( mMatrix );
		gl::rotate( Vec3f( 90.0f, app::getElapsedSeconds() * 10.0f, mAxialTilt ) );
		gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
		tex.enableAndBind();
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glVertexPointer( 3, GL_FLOAT, 0, mVerts );
		glTexCoordPointer( 2, GL_FLOAT, 0, mTexCoords );
		glDrawArrays( GL_TRIANGLES, 0, 6 );
		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		tex.disable();
		gl::popModelView();
	}
}

void NodeTrack::drawOrbitRing()
{
	gl::pushModelView();
	gl::translate( mParentNode->mTransPos );
	gl::rotate( mMatrix );


	int ringRes;
	if( mIsPlaying ){
		gl::color( ColorA( 0.15f, 0.2f, 0.4f, 0.5f ) );
		ringRes = 300;
	} else {
		gl::color( ColorA( 0.15f, 0.2f, 0.4f, 0.15f ) );
		ringRes = 150;
	}
	
	gl::drawStrokedCircle( Vec2f::zero(), mOrbitRadius, ringRes );
	gl::popModelView();
}

void NodeTrack::drawOrbiters()
{
	if( mStarRating > 0 ){
		float index = 0.0f;
		vector<Orbiter>::iterator it;
		for( it = mOrbiters.begin(); it != mOrbiters.end(); ++it ){
			it->draw( mMatrix, mTransPos, mBbRight, mBbUp );
			
			index ++;
		}
	}
}

void NodeTrack::setPlaying(bool playing)
{
	mIsPlaying = playing;
}

string NodeTrack::getName()
{
	return mTrack->getTitle();
}
