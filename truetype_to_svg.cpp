// Read TrueType (R) outline, write SVG
// Copyright Don Bright 2013 <hugh.m.bright@gmail.com>
/*

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  License based on zlib license, by Jean-loup Gailly and Mark Adler
*/

#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

FT_Vector halfway_between( FT_Vector p1, FT_Vector p2 )
{
	FT_Vector newv;
	newv.x = p1.x + (p2.x-p1.x)/2;
	newv.y = p1.y + (p2.y-p1.y)/2;
	return newv;
}

int main( int argc, char * argv[] )
{
	if (argc!=3) {
		cerr << "usage: " << argv[0] << " file.ttf 0x0042\n";
		exit( 1 );
	}

	cout << "<!--";
	cout << "\nFreetype - example 5\n";

	unsigned char glyphname[64];
	string filename( argv[1] );
	string unicode_s( argv[2] );
	int unicode = strtol( unicode_s.c_str(), NULL, 0 );
  FT_Library library;
  FT_Face face;
	FT_Error error;
	error = FT_Init_FreeType( &library );
	cout << "Init error code: " << error;

	// Load a typeface
	error = FT_New_Face( library, filename.c_str(), 0, &face );
	cout << "\nFace load error code: " << error;
	cout << "\nTrueType filename: " << filename;
	if (error) {
		cerr << "problem loading file " << filename << "\n";
		exit(1);
	}
	cout << "\nFamily Name: " << face->family_name;
	cout << "\nStyle Name: " << face->style_name;
	cout << "\nNumber of faces: " << face->num_faces;
	cout << "\nNumber of glyphs: " << face->num_glyphs;

	// Load the Glyph into the face's Glyph Slot + print details
	FT_UInt glyph_index = FT_Get_Char_Index( face, unicode );
	cout << "\nUnicode requested: " << unicode_s;
	cout << " (decimal: " << unicode << " hex: 0x" << hex << unicode << dec << ")";
	cout << "\nGlyph index for unicode: " << glyph_index;
	error = FT_Load_Glyph( face, glyph_index, FT_LOAD_NO_SCALE );
	cout << "\nLoad Glyph into Face's glyph slot. error code: " << error;
	FT_GlyphSlot slot = face->glyph;

	char glyph_name[1024];
	FT_Get_Glyph_Name( face, glyph_index, glyph_name, 1024 );
	FT_Glyph_Metrics gm = slot->metrics;
	cout << "\nGlyph Name: " << glyph_name;
	cout << "\nGlyph Width: " << gm.width
		<< " Height: " << gm.height
		<< " Hor. Advance: " << gm.horiAdvance
		<< " Vert. Advance: " << gm.vertAdvance;

	// Print outline details, taken from the glyph in the slot.
	FT_Outline outline = slot->outline;
  cout << "\nNum contours: " << outline.n_contours;
  cout << "\nNum points: " << outline.n_points;
	FT_Vector* points = outline.points;
	char* tags = outline.tags;
	short* contours = outline.contours;
	cout << "\n-->\n";


	// SVG output. See these sites for more info.
	// Basic Terms: http://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html
	// FType + outlines: http://www.freetype.org/freetype2/docs/reference/ft2-outline_processing.html
	// FType + contours: http://www.freetype.org/freetype2/docs/glyphs/glyphs-6.html
	// TType contours: https://developer.apple.com/fonts/TTRefMan/RM01/Chap1.html
	// TType contours2: http://www.truetype-typography.com/ttoutln.htm
	// Non-zero winding rule: http://en.wikipedia.org/wiki/Nonzero-rule
	// SVG paths: http://www.w3schools.com/svg/svg_path.asp
	// SVG paths + nonzero: http://www.w3.org/TR/SVG/painting.html#FillProperties

	stringstream svg;
	svg << "\n\n<!-- Header + border -->";
	int bbheight = face->bbox.yMax - face->bbox.yMin;
	int bbwidth = face->bbox.xMax - face->bbox.xMin;
	svg << "\n<svg width='" << bbwidth << "px'"
		<< " height='" << bbheight << "px'"
    << " xmlns='http://www.w3.org/2000/svg' version='1.1'>";
	svg << "\n <rect fill='none' stroke='black'"
		<< " width='" << bbwidth - 1 << "'"
		<< " height='" << bbheight - 1 << "'/>";

	svg << "\n\n <!-- make sure glyph is visible within svg window -->";
	int yadj = gm.horiBearingY + gm.vertBearingY + 100;
	int xadj = 100;
	svg << "\n <g fill-rule='nonzero' "
		<< " transform='translate(" << xadj << " " << yadj << ")'"
		<< ">";

	svg << "\n\n <!-- draw axes --> ";
	svg << "\n <path stroke='blue' stroke-dasharray='5,5' d='"
		<< " M" << -bbwidth << "," << 0
		<< " L" <<  bbwidth << "," << 0
		<< " M" << 0 << "," << -bbheight
		<< " L" << 0 << "," << bbheight
		<< " '/>";

	svg << "\n\n <!-- draw bearing box --> ";

	int x1 = 0;
	int x2 = gm.horiAdvance;
	int y1 = -gm.vertBearingY-gm.height;
	int y2 = y1 + gm.vertAdvance;

	svg << "\n\n <!-- vertical bearing Y --> ";
	svg << "\n <path stroke='blue' fill='none' stroke-dasharray='10,16' d='"
		<< " M" << x1 << "," << y1
		<< " M" << x1 << "," << y2
		<< " L" << x2 << "," << y2
		<< " L" << x2 << "," << y1
		<< " L" << x1 << "," << y1
		<< " '/>";

	// Invert y coordinates (SVG = neg at top, TType = neg at bottom)
	for ( int i = 0 ; i < outline.n_points ; i++ )
		points[i].y *= -1;

	svg << "\n\n<!-- draw points as circles -->";
	int contour_counter = 0;
	for ( int i = 0 ; i < outline.n_points ; i++ ) {
		int radius = 5;
		if ( i == 0 ) radius = 10;
		string color;
		if (tags[i] & 1) color = "blue"; else color = "none";
		svg << "\n <circle "
			<< " fill='" << color << "'"
			<< " stroke='black'"
			<< " cx='" << points[i].x << "' cy='" << points[i].y << "'"
			<< " r='" << radius << "'"
			<< "/>";
	}

	svg << "<!-- draw straight lines between points -->";
	svg << "\n <path fill='none' stroke='green' d='";
	svg << "\n   M " << points[0].x << "," << points[0].y << " L ";
	for ( int i = 0 ; i < outline.n_points ; i++ ) {
		svg << " " << points[i].x << "," << points[i].y;
	}
	svg << "'/>";



	svg << "<!-- draw actual outline using lines and Bezier curves-->";
	contour_counter = 0;
	svg << "\n\n  <path fill='none' stroke='black' d='\n";
	svg << "      M" << points[0].x << "," << points[0].y;;
	FT_Vector contour_startp = points[0];

	for ( int i = 1 ; i < outline.n_points ; i++ ) {
		// bit 1 indicates whether its a control point on a bez curve or not
		// two control points in a row imply another point halfway between them
		short prevtag = tags[i-1] & 1;
		short curtag = tags[i] & 1;
		short nexttag = tags[i+1] & 1;
		if ( !curtag ) {
			svg << " Q";
			FT_Vector nextp;
			if ( (i+1) > contours[contour_counter] ) nextp = contour_startp;
			else nextp = points[i+1];
			if ( !nexttag ) {
				nextp = halfway_between( points[i], nextp );
			}
			svg << " " << points[i].x << "," << points[i].y;
			svg << " " << nextp.x << "," << nextp.y;
		} else {
			svg << " L" << points[i].x << "," << points[i].y;
		}

		if ( i == contours[contour_counter] ) {
			contour_startp = points[i+1];
			svg << " Z";
			contour_counter++;
			if ( contour_counter < outline.n_contours )
				svg << "\n      M" << points[i+1].x << "," << points[i+1].y;
		}
	}
	svg << "\n  '/>";
	svg << "\n\n </g>";
	svg << "\n</svg>";

	cout << svg.str();

	// free memory
	cout << "\n<!--";
	error = FT_Done_Face( face );
	cout << "\nFree face. error code: " << error;
	error = FT_Done_FreeType( library );
	cout << "\nFree library. error code: " << error;
	cout << "\n-->\n";

  return 0;
}