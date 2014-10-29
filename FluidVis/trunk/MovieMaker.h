#ifndef _MOVIEMAKER_H
#define _MOVIEMAKER_H

// You should be able to include this in non-Windows code, since this
//    #ifdef will essentially comment the whole class out!

#include <windows.h>
#include <vfw.h>

#define BUFSIZE 260

// Define the MovieMaker class...
class MovieMaker {
public:
	// Initialize the class and necessary internal variables
	MovieMaker();
    ~MovieMaker();

	// Start capturing a new video with a given filename and specific
	//    framerate.  Unfortunately, you cannot give a per-frame length,
	//    which means every frame in the video will be the same length.
	void StartCapture(const char *name, int Wwidth, int Wheight, int framesPerSecond=30 );

	// Add the current frame to the video.
	inline bool AddCurrentFrame( void ) { return Snap(); }

	// Finish up capturing.  Close the file.  Other cleanup.
	void EndCapture( void );

	// Check if the StartCapture() worked correctly.
    inline bool IsOK( void ) const { return bOK; }

	// Returns the number of frames in the currently captured movie.
	inline int GetFrameCount( void ) const { return nFrames; }


/***************************************************************************/
/*                            PRIVATE STUFF                                */
/* --   should not need to modify, unless you know what you're doing!   -- */ 
/***************************************************************************/
public:
    char fname[64];
    int width, height, nFrames, ready, estFramesPerSecond;
	WORD wVer;

  	AVISTREAMINFO strhdr;
	PAVIFILE pfile;
	PAVISTREAM ps;
	PAVISTREAM psCompressed;
	PAVISTREAM psText;
	AVICOMPRESSOPTIONS opts;
	AVICOMPRESSOPTIONS FAR * aopts[1];
	DWORD dwTextFormat;
	char szText[BUFSIZE];
	bool bOK;

	HDC hdcScreen;
    HDC hdcCompatible; 
	HBITMAP hbmScreen; 

	bool Snap();
	void PrepareForCapture();
	HBITMAP LoadBMPFromFB( int w, int h );
	void PrintErrorMesage( void );
	bool CaptureError( char *errMsg, LPBITMAPINFOHEADER alpbi );
};


// #include <windows.h> pollutes the namespace (with MACROS!) clean up a bit
#undef CreateWindow
#undef AVIFileOpen

// We'll need to link in the library, too
#pragma comment(lib, "vfw32.lib")

// Define some macros used by the code
#define TEXT_HEIGHT	20
#define AVIIF_KEYFRAME	0x00000010L // this frame is a key frame.

#endif
