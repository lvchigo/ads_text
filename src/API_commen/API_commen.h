#pragma once
#include <vector>
#include <cv.h>

using namespace cv;
using namespace std;

class API_COMMEN
{

/***********************************Common***********************************/
typedef unsigned char uchar;
typedef unsigned long long UInt64;

#define WIDTH 256
#define HEIGHT 256
#define BLOB_WIDTH 224
#define BLOB_HEIGHT 224
#define CHANNEL 3

#define BLOCKNUM 9
/***********************************public***********************************/
public:

	/// construct function 
    API_COMMEN();
    
	/// distruct function
	~API_COMMEN(void);

	/***********************************PadEnd*************************************/
	void PadEnd(char *szPath);

	/***********************************String IsDigit2*************************************/
	bool IsDigit2(string str);

	/***********************************String Is AllChinese*************************************/
	bool IsAllChinese(string str);

	/***********************************GetIDFromFilePath*************************************/
	long GetIDFromFilePath(const char *filepath);

	/***********************************split***********************************/
	void split(const string& src, const string& separator, vector<string>& dest);

	/***********************************GetIDFromFilePath*************************************/
	void getRandomID( UInt64 &randomID );

	/***********************************loadWordDict*************************************/
	void loadWordDict(const char *filePath, vector< string > &labelWords);
	
	/***********************************Img_GetMutiRoi*************************************/
	int Img_GetMutiRoi( IplImage *MainBody, UInt64 ImageID, vector<Mat_<Vec3f> > &OutputImg );
	int Img_GetMutiRoi_ImageQuality( IplImage *MainBody, UInt64 ImageID, vector<Mat_<Vec3f> > &OutputImg );
	void Img_Get10MutiRoi( IplImage *MainBody, UInt64 ImageID, vector<Mat_<Vec3f> > &OutputImg );

	/***********************************Normalization*************************************/
	int Normal_MinMax( vector<float> inFeat, vector<float> &NormFeat );
	int Normal_L1( vector<float> inFeat, vector<float> &NormFeat );
	int Normal_L2( vector<float> inFeat, vector<float> &NormFeat );

	/***********************************Distance*************************************/
	int Distance_L1( vector< float > quaryWord, float &dist );	
	int Distance_Cosine( 
		vector< float > quaryWord, 		//[In]quary word
		vector< float > dicWord, 		//[In]dic word
		float 			dicWordDist, 	//[In]dic word Dist
		float			&dist );		//[Out]cosine distance

	/***********************************Image Format Change*************************************/
	uchar* ipl2mat(IplImage* image);			//for matlab
	uchar* ipl2uchar(IplImage* image);		//for c/c++
	uchar* ipl2rgb(IplImage* image);	
	uchar* ipl2gray(IplImage* image);
	float* ipl2gray_f(IplImage* image);
	IplImage* uchar2ipl(uchar* bgr, int width, int height, int channel);			//for check img

	/***********************************Resize Image width && height*****************************/
	int GetReWH( int width, int height, float maxLen, int &rWidth, int &rHeight );
	int GetReWH_Minlen( int width, int height, float minLen, int &rWidth, int &rHeight );

	/***********************************ImgProcess_gaussianFilter*****************************/
	void ImgProcess_gaussianFilter(uchar* data, int width, int height, int channel);

	/***********************************Creat Sample**********************************/
	IplImage* RemoveWhitePart( IplImage *image, UInt64 ImageID );
	int CreatSample_LowContrast( IplImage *image, UInt64 ImageID, double gamma = 3.0 );
	int CreatSample_LowResolution( IplImage *image, UInt64 ImageID );
	int CreatSample_smooth( IplImage *image, UInt64 ImageID );
	int CreatSample_addGaussNoise( IplImage *image, UInt64 ImageID );

	/***********************************Extract Feat**********************************/
	int ExtractFeat_Blur( IplImage* pSrcImg, vector< float > &fBlur );				//5D
	int ExtractFeat_Constract( IplImage* pSrcImg, vector< float > &fContrast );	//4D
	double ExtractFeat_Constract_GetBlockDev(IplImage* ScaleImg);

	/***********************************Encode Feature**********************************/
	int EncodeFeature( vector< float > pFeat, unsigned long long &code );
	/***********************************Hamming Distance**********************************/
	int Distance_Hamming( unsigned long long code1, unsigned long long code2 );

	/***********************************URL Encode/Decode*************************************/
	short int hexChar2dec(char c);
	char dec2hexChar(short int n);
	string URL_Encode(const string &URL);
	string URL_Decode(const string &URL);

/***********************************private***********************************/
private:

	/***********************************Img_hMirrorTrans*************************************/
	void Img_hMirrorTrans(const Mat &src, Mat &dst);

	/***********************************Creat Sample**********************************/
	int CreatSample_ImageAdjust( IplImage* src, IplImage* dst, double low, double high, double bottom, double top, double gamma ); 

	

};


	

