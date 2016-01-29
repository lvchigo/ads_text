#pragma once
#include <vector>

#include "Application.hpp"
#include "API_linearsvm.h"

using namespace std;
using namespace CppJieba;

class API_DOC2VEC
{

typedef unsigned long long UInt64;
#define DIM_DOC2VEC 200						//doc dim

/***********************************public***********************************/
public:

	/// construct function 
    API_DOC2VEC();
    
	/// distruct function
	~API_DOC2VEC(void);

	/***********************************Commen*************************************/
	bool IsDigit2(string str);
	bool IsAllChinese(string str);
	int Normal_L2( vector<float> inFeat, vector<float> &NormFeat );
	int Distance_L2( vector< float > quaryWord, float&dist );

	/***********************************URL Encode/Decode*************************************/
	short int hexChar2dec(char c);
	char dec2hexChar(short int n);
	string URL_Encode(const string &URL);
	string URL_Decode(const string &URL);

	/***********************************Init*************************************/
	int Init( const char *Keyfile );

	/***********************************FilterQuaryString*************************************/
	int FilterQuaryString( 
		vector< string > 	vecQuaryWord,		//[In]quary word
		vector< string > 	&vecFilterWord );	//[Out]filter word

	/******************************Process:Feat by doc2vec****************************/
	int doc2vec( 
		vector< string > 							vecQuaryWord,	//[In]quary word
		map< vector< string >, vector< float > > 	&mapQuaryDoc );	//[Out]quary Doc

	/******************************ExtractFeat:Feat by PCA***************************/
	int ExtractFeat( 
		vector< string > 							vecQuaryWord,	//[In]quary word
		map< vector< string >, vector< float > > 	&mapFeat );		//[Out]Feat

	/***********************************Predict*************************************/
	int Predict(
		vector< string > 										vecQuaryWord,	//[In]quary word
		map< vector< string >, vector< pair< int, float > > > 	&resPredict);	//[Out]Res:string-useful words,int-label,float-score

	/***********************************FindSimilarWord*************************************/
	int FindSimilarWord(
		vector< string > 									vecQuaryWord,	//[In]quary word
		map< string , vector< pair< string, float > > > 	&resSimWord);	//[Out]Res:string-useful words,int-label,float-score

	int FindSimilarWord_HammingCode(
		vector< string > 									vecQuaryWord,	//[In]quary word
		map< string , vector< pair< string, float > > > 	&resSimWord);	//[Out]Res:string-useful words,int-label,float-score

	/***********************************ExtractFeat_Entropy*************************************/
	int ExtractFeat_Entropy( vector< float > feat, double &entropy );
	
	/***********************************Release*************************************/
	void Release();

/***********************************private***********************************/
private:

	/***********************************Init**********************************/
	API_LINEARSVM 	api_svm;
	CppJieba::Application *app;

	/***********************************Init**********************************/
	map< string, int > stopwords;
	map< string, int > SensitiveWords;
	map< string, vector< float > >					mapDicWord; 	//[In]dic word
	map< string, float >							mapDicWordDist; //[In]dic word distance
	map< string, float >							mapDicIdf;		//[In]dic IDF
	map< UInt64, vector< string > >					mapDicCode;		//[In]dic code-words

	/***********************************Process*************************************/	
	int doc2vec_word2doc( 
		map< string, vector< float > > 					mapQuaryWord,	//[In]quary word
		map< string, float > 							mapQuaryTf,		//[In]quary TF
		map< vector< string >, vector< float > > 		&mapQuaryDoc );	//[Out]quary Doc

	/***********************************ErrPredict*************************************/	
	void ErrPredict( map< vector< string >, vector< pair< int, float > > > &resPredict );	//[Out]quary Doc

	/***********************************Distance_Cosine*************************************/	
	int Distance_Cosine( 
		vector< float > quaryWord,		//[In]quary word
		vector< float > dicWord,		//[In]dic word
		float			queryWordDist,	//[In]query word Dist
		float			dicWordDist,	//[In]dic word Dist
		float			&dist ); 		//[Out]cosine distance

	/**********************************Normal_MinMax*********************************/
	int Normal_MinMax( vector<float> inFeat, vector<float> &NormFeat );
	/**********************************encode feat to hamming code*********************************/
	int EncodeFeature( vector< float > pFeat, unsigned long long &code );
	/**********************************Distance_Hamming*********************************/
	int Distance_Hamming( unsigned long long code1, unsigned long long code2 );

	/***********************************Merge*************************************/	
	int Merge(
		map< vector< string >, vector< pair< int, float > > > 	inPredict,
		long 							queryStringSize,
		map< vector< string >, vector< pair< int, float > > > 	&outPredict);
};


	

	

