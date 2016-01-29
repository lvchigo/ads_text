#pragma once
#include <vector>
#include <cv.h>

#include "Application.hpp"
#include "API_commen.h"
#include "API_pca.h"
#include "API_linearsvm.h"

using namespace std;
using namespace cv;
using namespace CppJieba;

class API_DOC2VEC
{

typedef unsigned long long UInt64;
const long long max_size = 2000;         // max length of strings
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 200;              // max length of vocabulary entries

/***********************************public***********************************/
public:

	/// construct function 
    API_DOC2VEC();
    
	/// distruct function
	~API_DOC2VEC(void);

	/***********************************Init*************************************/
	int Init( const char *Keyfile );

	/******************************Process:Feat by doc2vec****************************/
	int doc2vec( 
		vector< string > 							vecQuaryWord,	//[In]quary word
		map< vector< string >, vector< float > > 	&mapQuaryDoc );	//[Out]quary Doc

	/******************************ExtractFeat:Feat by PCA***************************/
	int ExtractFeat( 
		vector< string > 							vecQuaryWord,	//[In]quary word
		map< vector< string >, vector< float > > 	&mapPCAFeat );	//[Out]Feat by PCA

	/***********************************Predict*************************************/
	int Predict(
		vector< string > 										vecQuaryWord,	//[In]quary word
		map< vector< string >, vector< pair< int, float > > > 	&resPredict);	//[Out]Res:string-useful words,int-label,float-score

	/***********************************Release*************************************/
	void Release();

/***********************************private***********************************/
private:

	/***********************************Init**********************************/
	API_COMMEN 		api_commen;
	API_PCA 		api_pca;
	API_LINEARSVM 	api_svm;

	/***********************************Init**********************************/
	map< string,int > stopwords;
	map< string, vector< float > >					mapDicWord; 	//[In]dic word
	map< string, float >							mapDicWordDist; //[In]dic word distance
	map< string, float >							mapDicIdf;		//[In]dic IDF
	map< UInt64, vector< string > >					mapDicCode;		//[In]dic code-words
	map< string, long >								mapDicIndex;	//[In]dic word Index

	/***********************************Process*************************************/	
	int doc2vec_word2doc( 
		map< string, vector< float > > 					mapQuaryWord,	//[In]quary word
		map< string, float > 							mapQuaryTf,		//[In]quary TF
		map< UInt64, vector< string > >					mapQuaryCode,	//[In]dic code-words
		map< vector< string >, vector< float > > 		&mapQuaryDoc );	//[Out]quary Doc

	int Feat_PCA(
		map< vector< string >, vector< float > > 	mapQuaryDoc,
		map< vector< string >, vector< float > > 	&mapPCAFeat);


};


	

