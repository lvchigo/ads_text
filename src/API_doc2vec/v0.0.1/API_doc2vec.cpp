#pragma once
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <iostream>
#include <unistd.h>

#include <vector>
#include <map>

#include <cv.h>
#include "Application.hpp"
#include "API_doc2vec.h"
#include "API_commen.h"
#include "API_pca.h"
#include "API_linearsvm.h"
#include "TErrorCode.h"

using namespace std;
using namespace cv;
using namespace CppJieba;

/***********************************Init*************************************/

CppJieba::Application app( "/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/jieba.dict.utf8",
							"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/hmm_model.utf8",
							"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/user.dict.utf8",
							"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/idf.utf8",
							"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/stop_words.utf8");


/// construct function 
API_DOC2VEC::API_DOC2VEC()
{
}

/// destruct function 
API_DOC2VEC::~API_DOC2VEC(void)
{
}

/***********************************Init*************************************/
int API_DOC2VEC::Init( const char *Keyfile )
{
	/*****************************Init:Commen*****************************/
	int nRet = 0;
	char loadFile[1024];
	char loadPath[4096];
	char vocab[256];
	char charIdf[256];
	string word;
	float tmp, idf = 0;
	long long Count, i, j, a, b, w2v_words, w2v_size;
	
	FILE *fpW2VList = 0, *fpIDFList = 0, *fpStopWord = 0;

	/*****************************Init:Cutword*****************************/
	map< string,int >::iterator 		itStopwords;
	
	/*****************************Init:Doc2Vec*****************************/
	vector< float > vecTmp;
	vector< float > NormVec;

	/***********************************Init api_pca*************************************/
	sprintf(loadFile, "%s/w2v/pca_text2class_66_050827.model",Keyfile);
	printf("load api_pca:%s\n",loadFile);
	api_pca.PCA_Feat_Init(loadFile); 

	/***********************************Init Model_SVM*************************************/
	sprintf(loadFile, "%s/w2v/liblinear_text2class_050827_5000.model",Keyfile);	//linearSVM
	printf("load Model_SVM:%s\n",loadFile);
	nRet = api_svm.Init(loadFile); 
	if (nRet != 0)
	{
	   cout<<"Fail to initialization "<<endl;
	   return TEC_INVALID_PARAM;
	}
	
	/*****************************Load:Cutword FileStopwords*****************************/
	sprintf(loadFile, "%s/cutword-geiba/stop_words.utf8",Keyfile);
	printf("Load:Cutword FileStopwords:%s\n",loadFile);
	fpStopWord = fopen(loadFile,"r");
	if (!fpStopWord) 
	{
		cout << "Can't open " << loadFile << endl;
		return -1;
	}
	
	stopwords.clear();
	Count = 0;
	while(EOF != fscanf(fpStopWord, "%s", &loadPath ))
	{
		word = loadPath;		
		if ( word == " " )
			continue;

		Count++;

		itStopwords = stopwords.find(word);
		if (itStopwords == stopwords.end()) // not find
		{
			stopwords[word] = Count;
		}
	}
	
	/*****************************Load W2V.txt*****************************/
	sprintf(loadFile, "%s/w2v/desc.vec",Keyfile);
	printf("Load W2V.txt:%s\n",loadFile);
	fpW2VList = fopen(loadFile,"r");
	if (!fpW2VList) 
	{
		cout << "Can't open " << loadFile << endl;
		return -1;
	}
	
	mapDicWord.clear();			//[In]dic word
	mapDicWordDist.clear();		//[In]dic word distance
	fscanf(fpW2VList, "%lld", &w2v_words);
	fscanf(fpW2VList, "%lld", &w2v_size);
	printf("Load W2V.txt...w2v_words-%ld,w2v_size-%ld\n",w2v_words,w2v_size);
	for (b = 0; b < w2v_words; b++) {
		//load word
		a = 0;
		while (1) {
			vocab[a] = fgetc(fpW2VList);
			if (feof(fpW2VList) || (vocab[a] == ' ')) break;
			if ((a < max_w) && (vocab[a] != '\n')) a++;
		}
		vocab[a] = 0;
		word = vocab;

		//load vector
		vecTmp.clear();
		for (i = 0; i < w2v_size; i++) 
		{
			tmp = 0;
			fread(&tmp, sizeof(float), 1, fpW2VList);
			vecTmp.push_back( tmp );
		}

		if ( vecTmp.size() == 0 )
		{
			printf( "doc2vec_train:vecTmp.size():%ld, do not find info!!\n", vecTmp.size() );
			continue;
		}

		//normal
		NormVec.clear();
		nRet = api_commen.Normal_L2( vecTmp, NormVec );
		if ( nRet != 0 ) 
		{
			printf("doc2vec_train:Normal_L2 err!!\n");
			return nRet;
		}

		if ( NormVec.size() == 0 )
		{
			printf( "doc2vec_train:NormVec.size():%ld, do not find info!!\n", NormVec.size() );
			continue;
		}

		//Distance_L1
		nRet = api_commen.Distance_L1( NormVec, tmp );	//quary distance : L1
		if ( nRet != 0 ) 
		{
			printf("doc2vec_train:Distance_L1 err!!\n");
			return nRet;
		}

		//save data
		mapDicWord[word] = NormVec;
		mapDicWordDist[word] = tmp;

		//check data 
		if (b<3)
		{
			tmp = (vecTmp.size()>5)?5:vecTmp.size();
			printf("word:%s,vec:",word.c_str());
			for(i=0;i<tmp;i++)
				printf("%f:%f ",vecTmp[i],NormVec[i]);
			printf("\n");
		}
	}
	printf("mapWord2Vec.size():%ld\n",mapDicWord.size());

	/*****************************Load IDF.txt*****************************/
	sprintf(loadFile, "%s/w2v/desc_idf.txt",Keyfile);
	printf("Load IDF.txt:%s\n",loadFile);
	fpIDFList = fopen(loadFile,"r");
	if (!fpIDFList) 
	{
		cout << "Can't open " << loadFile << endl;
		return -1;
	}
	
	Count = 0;
	while(EOF != fscanf(fpIDFList, "%s %s", &loadPath, &charIdf ))
	{
		word = loadPath;
		idf = atof(charIdf);
		if ( (word == " ") || (word == "	") )
			continue;

		mapDicIdf[word] = idf;

		//check data 
		if (Count<3)
		{
			printf("word:%s,idf:%f\n",word.c_str(),idf);
		}
		Count++;
	}
	printf("mapIdf.size():%ld\n",mapDicIdf.size());

	/*********************************close file*************************************/
	if (fpW2VList) {fclose(fpW2VList);fpW2VList = 0;}
	if (fpIDFList) {fclose(fpIDFList);fpIDFList = 0;}
	if (fpStopWord) {fclose(fpStopWord);fpStopWord = 0;}

	return nRet;
}

/***************************************************************/
/*****************Ni=sum(TF*IDF*distance(Ni,nj))/n**************/
/***************************************************************/
int API_DOC2VEC::doc2vec_word2doc( 
	map< string, vector< float > > 					mapQuaryWord,	//[In]quary word
	map< string, float > 							mapQuaryTf,		//[In]quary TF
	map< vector< string >, vector< float > > 		&mapQuaryDoc )	//[Out]quary Doc
{
	/*****************************Init*****************************/
	int i, j, nRet = 0;
	float quaryTf,dicDist,dicIdf,distCosine,docFeat;
	string wordQuary,wordDic,docString;

	vector< float > vecQuary;
	vector< float > vecDic;
	vector< float > vecDocFeat;
	vector< float > vecDocNormFeat;
	vector< string > vecDocString;
	map< string, vector< float > >::iterator 	itQuaryWord;	//[In]quary word
	map< string, float >::iterator 				itQuaryTf;		//[In]quary TF
	map< string, vector< float > >::iterator 	itDicWord;		//[In]dic word
	map< string, float >::iterator 				itDicWordDist;	//[In]dic word distance
	map< string, float >::iterator 				itDicIdf;		//[In]dic IDF

	/*****************************word2doc*****************************/
	for(itDicWord = mapDicWord.begin(); itDicWord != mapDicWord.end(); itDicWord++)
	{
		//dic info
		vecDic.clear();
		wordDic = itDicWord->first;
		vecDic = itDicWord->second;

		//dic distance
		itDicWordDist = mapDicWordDist.find(wordDic);
		if (itDicWordDist != mapDicWordDist.end()) // find it
			dicDist = itDicWordDist->second;
		else
			continue;

		//dic idf
		itDicIdf = mapDicIdf.find(wordDic);
		if (itDicIdf != mapDicIdf.end()) // find it
			dicIdf = itDicIdf->second;
		else
			continue;

		//Read Quary Info
		docFeat = 0;
		for(itQuaryWord = mapQuaryWord.begin(); itQuaryWord != mapQuaryWord.end(); itQuaryWord++)
		{
			//quary info
			vecQuary.clear();
			wordQuary = itQuaryWord->first;
			vecQuary = itQuaryWord->second;

			//quary tf
			itQuaryTf = mapQuaryTf.find(wordQuary);
			if (itQuaryTf != mapQuaryTf.end()) // find it
				quaryTf = itQuaryTf->second;
			else
				continue;

			//Distance_Cosine
			nRet = api_commen.Distance_Cosine( vecQuary, vecDic, dicDist, distCosine );
			if ( nRet != 0 ) 
			{
				printf("word2doc:Distance_Cosine err!!\n");
				return nRet;
			}

			//doc Feat
			docFeat += quaryTf*dicIdf*distCosine;
		}
		docFeat = docFeat*1.0/mapQuaryWord.size();
		vecDocFeat.push_back( docFeat );
/*		nRet = Normal_L1( vecDocFeat, vecDocNormFeat );
		if ( nRet != 0 ) 
		{
			printf("Normal_L1 err!!\n");
			return nRet;
		}*/
	}

	/*****************************word2doc*****************************/
	vecDocString.clear();
	for(itQuaryWord = mapQuaryWord.begin(); itQuaryWord != mapQuaryWord.end(); itQuaryWord++)
	{
		wordQuary = itQuaryWord->first;
		vecDocString.push_back( wordQuary );
	}

	//write info
	mapQuaryDoc[vecDocString] = vecDocFeat;

	return nRet;
}

int API_DOC2VEC::doc2vec( 
	vector< string > 							vecQuaryWord,	//[In]quary word
	map< vector< string >, vector< float > > 	&mapQuaryDoc )	//[Out]quary Doc
{
	if ( vecQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::doc2vec err!!vecQuaryWord.size:%ld, do not find KeyWord!!\n", vecQuaryWord.size() );
		return -1; 
	}	
	
	/*****************************Init:Commen*****************************/
	int nRet = 0;
	string word;
	long long i, j, Count;

	/*****************************Init*****************************/
	vector<string> outQueryWords;
	vector<string> queryCutWords;
	map< string, vector< float > > 					mapQuaryWord;	//[In]quary word
	map< string, float >							mapQuaryTf; 	//[In]quary TF
	map< string,int >::iterator 					itStopwords;
	map< string, float >::iterator 					itQuaryTf;		//[In]dic word
	map< string, vector< float > >::iterator 		itDicWord;		//[In]dic word
	
	//doc2vec_cutword
	outQueryWords.clear();
	for ( j=0;j<vecQuaryWord.size();j++ )
	{
		queryCutWords.clear();
		//Cut Word
		app.cut( vecQuaryWord[j], queryCutWords, METHOD_MIX );

		//Check && Save GetFeat
		for ( i=0;i<queryCutWords.size();i++ )
		{
			if ( (queryCutWords[i].empty()) || (queryCutWords[i] == " ") || (queryCutWords[i] == "	") 
					|| (api_commen.IsDigit2(queryCutWords[i])) || (queryCutWords[i].size()<2) )
			{
				continue;
			}
			
			itStopwords = stopwords.find(queryCutWords[i]);
			if (itStopwords == stopwords.end()) // not find
			{
				outQueryWords.push_back( queryCutWords[i] );
			}
		}	
	}

/*	//check info
	printf("Cut Words:");
	for(i=0;i<outQueryWords.size();i++)
		printf( "%s ", outQueryWords[i].c_str() );
	printf( "\n" );*/

	mapQuaryWord.clear();	//[In]quary word
	mapQuaryTf.clear();		//[In]quary TF
	//Count Quary TF by Quary Vector 
	for (i = 0; i < outQueryWords.size(); i++) 
	{
		word = outQueryWords[i];
		
		//Quary Vector
		itDicWord = mapDicWord.find(word);
		if (itDicWord != mapDicWord.end()) // find it
			mapQuaryWord[word] = itDicWord->second;
		else
			continue;

		//Quary TF
		itQuaryTf = mapQuaryTf.find(word);
		if (itQuaryTf != mapQuaryTf.end()) // find it
			mapQuaryTf[word]++;
		else
			mapQuaryTf[word] = 1;
	}

	//Quary TF
	Count = mapQuaryTf.size();
	if ( Count == 0 )
	{
		printf( "idQuarySize:%ld, do not find KeyWord!!\n", outQueryWords.size() );
		return -1; 
	}		
	for(itQuaryTf = mapQuaryTf.begin(); itQuaryTf != mapQuaryTf.end(); itQuaryTf++)
	{
		mapQuaryTf[itQuaryTf->first] = itQuaryTf->second*1.0/Count;
	}

	//word2doc
	mapQuaryDoc.clear();
	nRet = doc2vec_word2doc( mapQuaryWord, mapQuaryTf, mapQuaryDoc );
	if ( nRet != 0 ) 
	{
		printf("doc2vec_train:word2doc err!!\n");
		return nRet;
	}

	return nRet;
}

int API_DOC2VEC::Feat_PCA(
	map< vector< string >, vector< float > > 	mapQuaryDoc,
	map< vector< string >, vector< float > > 	&mapPCAFeat)
{
	if ( mapQuaryDoc.size() == 0 )
	{
		printf( "API_DOC2VEC::Feat_PCA err!!mapQuaryDoc.size:%ld!!\n", mapQuaryDoc.size() );
		return -1; 
	}

	/*****************************Init*****************************/
	int nRet = 0;
	vector<float> EncodeFeat;
	vector<float> NormalFeat;
	map< vector< string >, vector< float > >::iterator itQuaryDoc;

	/*****************************Process*****************************/
	for(itQuaryDoc = mapQuaryDoc.begin(); itQuaryDoc != mapQuaryDoc.end(); itQuaryDoc++)
	{
		EncodeFeat.clear();
		NormalFeat.clear();
		/************************PCA_Feat_Encode*****************************/
		nRet = api_pca.PCA_Feat_Encode( itQuaryDoc->second, EncodeFeat);
		if (nRet != 0)
		{
		   cout<<"Fail to PCA_Feat_Encode!! "<<endl; 
		   return nRet;
		}

		/************************PCA Feat Normal*****************************/
		nRet = api_commen.Normal_L2(EncodeFeat,NormalFeat);
		if (nRet != 0)
		{
		   cout<<"Fail to Normal_L2!! "<<endl; 
		   return nRet;
		}

		mapPCAFeat[itQuaryDoc->first] = NormalFeat;
	}

	return nRet;
}

int API_DOC2VEC::ExtractFeat( 
	vector< string > 								vecQuaryWord,	//[In]quary word
	map< vector< string >, vector< float > > 		&mapPCAFeat )	//[Out]Feat by PCA
{
	if ( vecQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::ExtractFeat err!!vecQuaryWord.size:%ld!!\n", vecQuaryWord.size() );
		return -1; 
	}

	/*****************************Init*****************************/
	int nRet = 0;
	map< vector< string >, vector< float > > mapQuaryDoc;

	/*****************************doc2vec*****************************/
	nRet = doc2vec( vecQuaryWord, mapQuaryDoc );
	if (nRet != 0)
	{	
	   	cout<<"Fail to ExtractFeat:doc2vec !! "<<endl; 
	   	return nRet;
	}	

	/*****************************Feat_PCA*****************************/
	mapPCAFeat.clear();
	nRet = Feat_PCA( mapQuaryDoc, mapPCAFeat );
	if (nRet != 0)
	{	
	   	cout<<"Fail to ExtractFeat:Feat_PCA !! "<<endl; 
	   	return nRet;
	}	

	return nRet;
}

int API_DOC2VEC::Predict(
	vector< string > 										vecQuaryWord,	//[In]quary word
	map< vector< string >, vector< pair< int, float > > > 	&resPredict)	//[Out]Res:string-useful words,int-label,float-score
{
	if ( vecQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::Predict err!!vecQuaryWord.size:%ld!!\n", vecQuaryWord.size() );
		return -1; 
	}

	/*****************************Init*****************************/
	int nRet = 0;
	map< vector< string >, vector< float > > mapPCAFeat;
	map< vector< string >, vector< float > >::iterator itPCAFeat;

	vector< vector< float > > 			PredictFeat;
	vector< pair< int, float > > 		tmpPredict;

	/*****************************Process*****************************/
	nRet = ExtractFeat( vecQuaryWord, mapPCAFeat );
	if ( ( nRet != 0 ) || ( mapPCAFeat.size() == 0 ) )
	{	
	   	cout<<"Fail to Predict:ExtractFeat !! "<<endl; 
	   	return nRet;
	}	

	/************************Feat Exchange*****************************/	
	PredictFeat.clear();
	for(itPCAFeat = mapPCAFeat.begin(); itPCAFeat != mapPCAFeat.end(); itPCAFeat++)
	{
		PredictFeat.push_back( itPCAFeat->second );
	}

	/************************SVM Predict*****************************/	
	tmpPredict.clear();
	nRet = api_svm.Predict( PredictFeat, tmpPredict );
	if ( ( nRet != 0 ) || ( tmpPredict.size() == 0 ) )
	{	
	   	cout<<"Fail to Predict SVM!! "<<endl; 
	   	return nRet;
	}	

	/************************save info*****************************/
	resPredict.clear();
	for(itPCAFeat = mapPCAFeat.begin(); itPCAFeat != mapPCAFeat.end(); itPCAFeat++)
	{
		resPredict[itPCAFeat->first] = tmpPredict;
	}

	return nRet;
}

/***********************************Release*************************************/
void API_DOC2VEC::Release()
{
	/***********************************PCA Model**********************************/
	api_pca.PCA_Feat_Release();

	/***********************************SVM Model******libsvm3.2.0********************/
	api_svm.Release();
}


