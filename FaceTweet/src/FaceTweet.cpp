/**
 * @Author: Craig Parkinson
 * @Title: PiFace
 * @Description: Use local binary histogram face recogintion run a script
 */

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "twitterClient.h"

#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

void detectAndDisplay( Mat frame, int& people, twitCurl& twitterObj, string userToTweet );
void printUsage();

String face_cascade_name = "haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";
RNG rng(12345);

int main(int argc, const char *argv[])
{
	/* Get username and password from command line args */
	string userName( "" );
	string passWord( "" );
	string userToTweet("");

	if( argc > 4 )
	{
		for( int i = 1; i < argc; i += 2 )
	    {
			if( 0 == strncmp( argv[i], "-u", strlen("-u") ) )
	        {
				userName = argv[i+1];
	        }
	        else if( 0 == strncmp( argv[i], "-p", strlen("-p") ) )
	        {
	        	passWord = argv[i+1];
	        }
	        else if( 0 == strncmp(argv[i], "-acc", strlen("-acc")))
	        {
	        	userToTweet = argv[i + 1];
	        	stringstream tmp;
	        	tmp << "@" << userToTweet;

	        	userToTweet = tmp.str();
	        }
	     }

		if( ( 0 == userName.length() ) || ( 0 == passWord.length() ) || ( 0 == userToTweet.length()))
		{
			printUsage();
	        return 0;
	    }
	 }
	 else
	 {
		 printUsage();
		 return 0;
	 }

	 twitCurl twitterObj;
	 string tmpStr, tmpStr2;
	 string replyMsg;
	 char tmpBuf[1024];

	 /* Set twitter username and password */
	 twitterObj.setTwitterUsername( userName );
	 twitterObj.setTwitterPassword( passWord );

	 /* OAuth flow begins */
	 /* Step 0: Set OAuth related params. These are got by registering your app at twitter.com */
	 twitterObj.getOAuth().setConsumerKey( string( "FEQHSR2XmXHE4BdvOnVGoMnmk" ) );
	 twitterObj.getOAuth().setConsumerSecret( string( "t8tDEK0pWwYLFTET7h84P9bVwUHCzg76eOt9jwrE4YGQMk5dr3" ) );

	 /* Step 1: Check if we alredy have OAuth access token from a previous run */
	 string myOAuthAccessTokenKey("");
	 string myOAuthAccessTokenSecret("");
	 ifstream oAuthTokenKeyIn;
	 ifstream oAuthTokenSecretIn;

	 oAuthTokenKeyIn.open( "twitterClient_token_key.txt" );
	 oAuthTokenSecretIn.open( "twitterClient_token_secret.txt" );

	 memset( tmpBuf, 0, 1024 );
	 oAuthTokenKeyIn >> tmpBuf;
	 myOAuthAccessTokenKey = tmpBuf;

	 memset( tmpBuf, 0, 1024 );
	 oAuthTokenSecretIn >> tmpBuf;
	 myOAuthAccessTokenSecret = tmpBuf;

	 oAuthTokenKeyIn.close();
	 oAuthTokenSecretIn.close();

	 if( myOAuthAccessTokenKey.size() && myOAuthAccessTokenSecret.size() )
	 {
		 /* If we already have these keys, then no need to go through auth again */
	     printf( "\nUsing:\nKey: %s\nSecret: %s\n\n", myOAuthAccessTokenKey.c_str(), myOAuthAccessTokenSecret.c_str() );

	     twitterObj.getOAuth().setOAuthTokenKey( myOAuthAccessTokenKey );
	     twitterObj.getOAuth().setOAuthTokenSecret( myOAuthAccessTokenSecret );

	 }
	 else
	 {
		 /* Step 2: Get request token key and secret */
	     string authUrl;
	     twitterObj.oAuthRequestToken( authUrl );

	     /* Else, pass auth url to twitCurl and get it via twitCurl PIN handling */
	     twitterObj.oAuthHandlePIN( authUrl );

	     /* Step 4: Exchange request token with access token */
	     twitterObj.oAuthAccessToken();

	     /* Step 5: Now, save this access token key and secret for future use without PIN */
	     twitterObj.getOAuth().getOAuthTokenKey( myOAuthAccessTokenKey );
	     twitterObj.getOAuth().getOAuthTokenSecret( myOAuthAccessTokenSecret );

	     /* Step 6: Save these keys in a file or wherever */
	     ofstream oAuthTokenKeyOut;
	     ofstream oAuthTokenSecretOut;

	     oAuthTokenKeyOut.open( "twitterClient_token_key.txt" );
	     oAuthTokenSecretOut.open( "twitterClient_token_secret.txt" );

	     oAuthTokenKeyOut.clear();
	     oAuthTokenSecretOut.clear();

	     oAuthTokenKeyOut << myOAuthAccessTokenKey.c_str();
	     oAuthTokenSecretOut << myOAuthAccessTokenSecret.c_str();

	     oAuthTokenKeyOut.close();
	     oAuthTokenSecretOut.close();
	   }
	    /* OAuth flow ends */

	    /* Account credentials verification */
	    if( twitterObj.accountVerifyCredGet() )
	    {
	        twitterObj.getLastWebResponse( replyMsg );
	        printf( "\ntwitterClient:: twitCurl::accountVerifyCredGet web response:\n%s\n", replyMsg.c_str() );
	    }
	    else
	    {
	        twitterObj.getLastCurlError( replyMsg );
	        printf( "\ntwitterClient:: twitCurl::accountVerifyCredGet error:\n%s\n", replyMsg.c_str() );
	    }

    CvCapture* capture;
    Mat frame;
    int people = 0;

    //-- 1. Load the cascades
    if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };
    if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

    //-- 2. Read the video stream
    capture = cvCaptureFromCAM( -1 );
    if( capture )
    {
    	while( true )
        {
    		frame = cvQueryFrame( capture );

    		//-- 3. Apply the classifier to the frame
    		if( !frame.empty() )
    		{
    			detectAndDisplay( frame, people, twitterObj, userToTweet );
    		}
    		else
    		{
    			printf(" --(!) No captured frame -- Break!");
    			break;
    		}

    		int c = waitKey(10);

    		if( (char)c == 'c' )
    		{
    			break;
    		}
          }
       }
       return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame, int& people, twitCurl& twitterObj, string userToTweet )
{
  std::vector<Rect> faces;
  Mat frame_gray;

  cvtColor( frame, frame_gray, CV_BGR2GRAY );
  equalizeHist( frame_gray, frame_gray );

  //-- Detect faces
  face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

  for( size_t i = 0; i < faces.size(); i++ )
  {
    Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
    ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

    Mat faceROI = frame_gray( faces[i] );
    std::vector<Rect> eyes;

    //-- In each face, detect eyes
    eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    if(people != (eyes.size() / 2))
    {
    	people = (eyes.size() / 2);
    	if(people > 0)
    	{
    		stringstream sstm;

    	    sstm <<  userToTweet << " " << people << " people are at the door";
    	    string tmpStr = sstm.str();

    	    string replyMsg = "";

    	    if( twitterObj.statusUpdate( tmpStr ) )
    	    {
    	    	twitterObj.getLastWebResponse( replyMsg );
    	    	printf( "\ntwitterClient:: twitCurl::statusUpdate web response:\n%s\n", replyMsg.c_str() );
    	    }
    	    else
    	    {
    	    	twitterObj.getLastCurlError( replyMsg );
    	    	printf( "\ntwitterClient:: twitCurl::statusUpdate error:\n%s\n", replyMsg.c_str() );
    	    }
    	}
    }

    for( size_t j = 0; j < eyes.size(); j++ )
    {
    	Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
    	int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
    	circle( frame, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
    }
  }
  //-- Show what you got
  imshow( window_name, frame );
 }

void printUsage()
{
    printf( "\nUsage:\ntwitterClient -u username -p password -acc accountToTweet\n" );
}
