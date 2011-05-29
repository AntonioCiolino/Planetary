//
//  TextLabel.cpp
//  Kepler
//
//  Created by Tom Carden on 5/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TextLabel.h"

void TextLabel::setText(string text)
{ 
    if (mText != text) { 
        mText = text; 
        updateTexture(); 
    }
}

void TextLabel::updateTexture()
{
    TextLayout layout;
    layout.setFont( mFont );
    layout.setColor( mColor );
    layout.addLine( mText );
    mTexture = layout.render( true, false );
}

void TextLabel::draw()
{
    if (mTexture) {
        gl::draw(mTexture, mRect.getUpperLeft());
    }
}