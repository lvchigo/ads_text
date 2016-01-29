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

#include <list>
#include <vector>
#include <map>

#include "Application.hpp"
#include "API_doc2vec.h"
#include "API_linearsvm.h"
#include "TErrorCode.h"
#include <boost/regex.hpp>	// regular expression

using namespace std;
using namespace CppJieba;

/***********************************ImgSortComp*************************************/
static bool ImgSortComp(const pair< string, float > elem1, const pair< string, float > elem2)
{
	return (elem1.second > elem2.second);
}

/***********************************ImgSortComp2*************************************/
static bool ImgSortComp2(const pair< unsigned long long, int > elem1, const pair< unsigned long long, int > elem2)
{
	return (elem1.second < elem2.second);
}


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
	int label,nRet = 0;
	char loadFile[4096];
	char loadPath[4096];
	char vocab[4096];
	char charIdf[4096];
	string word;
	float tmp, idf = 0;
	long long Count, i, j, a, b, w2v_words, w2v_size;
	UInt64 code = 0;
	
	FILE *fpW2VList = 0, *fpIDFList = 0, *fpStopWord = 0, *fpSensitiveWord = 0;

	/*****************************Init:Cutword*****************************/
	map< string,int >::iterator 				itStopwords;
	map< UInt64, vector< string > >::iterator 	itDicCode;		//[In]dic word
	
	/*****************************Init:Doc2Vec*****************************/
	vector< float > vecTmp;
	vector< float > NormVec;
	vector< string > codeString;

	/*****************************Init:CppJieba*****************************/
	char File_jieba_dict[4096];
	char File_hmm_model[4096];
	char File_user_dict[4096];
	char File_idf[4096];
	char File_stop_words[4096];
	sprintf(File_jieba_dict, 	"%s/cutword-geiba/jieba.dict.utf8",Keyfile);//File_jieba_dict
	sprintf(File_hmm_model, 	"%s/cutword-geiba/hmm_model.utf8",Keyfile);	//File_hmm_model
	sprintf(File_user_dict, 	"%s/cutword-geiba/user.dict.utf8",Keyfile);	//File_user_dict
	sprintf(File_idf, 			"%s/cutword-geiba/idf.utf8",Keyfile);		//File_idf
	sprintf(File_stop_words, 	"%s/cutword-geiba/stop_words.utf8",Keyfile);//File_stop_words
	app = new CppJieba::Application( File_jieba_dict, File_hmm_model, File_user_dict, File_idf, File_stop_words );
	printf("load File_jieba_dict:%s\n",	File_jieba_dict);
	printf("load File_hmm_model:%s\n",	File_hmm_model);
	printf("load File_user_dict:%s\n",	File_user_dict);
	printf("load File_idf:%s\n",		File_idf);
	printf("load File_stop_words:%s\n",	File_stop_words);

	/***********************************Init Model_SVM*************************************/
	//sprintf(loadFile, "%s/w2v/Alldata_rmsim_20150902_4/liblinear_text2class_100w_20150902_4.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_text2class_13k_20151010.2.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_adstext2class_20151012.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/alldata_20151013/liblinear_adstext2class_20151013.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_adstext2class_20151016.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_adstext2class_20151017.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_adstext2class_20151019.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_adstext2class_2015101902.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_adstext2class_2015102001.model",Keyfile);	//linearSVM
	//sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_adstext2class_2015102002.model",Keyfile);	//linearSVM
	sprintf(loadFile, "%s/w2v/seed_20150902/liblinear_adstext2class_2015102003.model",Keyfile);	//linearSVM
	printf("load Model_SVM:%s\n",loadFile);
	nRet = api_svm.Init(loadFile); 
	if (nRet != 0)
	{
	   cout<<"Fail to initialization "<<endl;
	   return TEC_INVALID_PARAM;
	}
	
	/*****************************Load:Cutword FileStopwords*****************************/
	fpStopWord = fopen(File_stop_words,"r");
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
	//sprintf(loadFile, "%s/w2v/Alldata_rmsim_20150902_4/desc.vec",Keyfile);
	//sprintf(loadFile, "%s/w2v/Alldata_rmsim_20150902_4/in_sogou_book.vec",Keyfile);
	sprintf(loadFile, "%s/w2v/alldata_20151013/in_sogou_book.vec",Keyfile);
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
			if ((a < 4096) && (vocab[a] != '\n')) a++;
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
		nRet = Normal_L2( vecTmp, NormVec );
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

		//Distance_L2
		nRet = Distance_L2( NormVec, tmp );	//quary distance : L1
		if ( nRet != 0 ) 
		{
			printf("doc2vec_train:Distance_L2 err!!\n");
			return nRet;
		}

		//save data
		mapDicWord[word] = NormVec;
		mapDicWordDist[word] = tmp;

		//Encode Hamming Feature
		code = 0;
		nRet = EncodeFeature( NormVec, code );	//encode feat
		if ( nRet != 0 ) 
		{
			printf("doc2vec:EncodeFeature err!!\n");
			return nRet;
		}
		itDicCode = mapDicCode.find(code);		
		if (itDicCode != mapDicCode.end()) // find it
			itDicCode->second.push_back(word);
		else
		{
			codeString.clear();
			codeString.push_back(word);
			mapDicCode[code] = codeString;		//[In]dic code-words
		}

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
	//sprintf(loadFile, "%s/w2v/Alldata_rmsim_20150902_4/desc_idf.txt",Keyfile);
	//sprintf(loadFile, "%s/w2v/Alldata_rmsim_20150902_4/in_sogou_book_idf.txt",Keyfile);
	sprintf(loadFile, "%s/w2v/alldata_20151013/in_sogou_book_idf.txt",Keyfile);
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

	/*****************************Load SensitiveWords*****************************/
	sprintf(loadFile, "%s/w2v/Alldata_rmsim_20150902_4/in_ads_SensitiveWords.csv",Keyfile);
	printf("Load SensitiveWords:%s\n",loadFile);
	fpSensitiveWord = fopen(loadFile,"r");
	if (!fpSensitiveWord) 
	{
		cout << "Can't open " << loadFile << endl;
		return -1;
	}
	
	Count = 0;
	while(EOF != fscanf(fpSensitiveWord, "%s %s", &loadPath, &charIdf ))
	{
		word = loadPath;
		label = atoi(charIdf);
		if ( (word == " ") || (word == "	") || ( label<1 ) || ( label>3 ) )
			continue;

		SensitiveWords[word] = label;

		//check data 
		if (Count<3)
		{
			printf("word:%s,label:%d\n",word.c_str(),label);
		}
		Count++;
	}
	printf("SensitiveWords.size():%ld\n",SensitiveWords.size());

	/*********************************close file*************************************/
	if (fpW2VList) {fclose(fpW2VList);fpW2VList = 0;}
	if (fpIDFList) {fclose(fpIDFList);fpIDFList = 0;}
	if (fpStopWord) {fclose(fpStopWord);fpStopWord = 0;}
	if (fpSensitiveWord) {fclose(fpSensitiveWord);fpSensitiveWord = 0;}

	return nRet;
}

int API_DOC2VEC::FilterQuaryString( 
	vector< string > 	vecQuaryWord,		//[In]quary word
	vector< string > 	&vecFilterWord )	//[Out]filter word
{
	if ( vecQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::FilterQuaryString err!!vecQuaryWord.size:%ld!!\n", vecQuaryWord.size() );
		return -1; 
	}	

	int i,j,nRet = 0;

	std::string testString;
	std::string regstr = "(.*?)(\\d{5,}|[\uFF00-\uFFFF]+)";
	//std::string regstr_2 = "(.*?)([\u4e00-\u9fa5]+)";
	//std::string regstr = "(.*?)(\\d{5,}|[\u3002\u3001\uff01\uff1f\uff1a\uff1b\ufe51\u2022\uff02\u2026\u2018\u2019\u201c\u201d\u301d\u301e\u2215\u2016\u2014\u3008\u3009\ufe5e\ufe5d\u300c\u300d\u2039\u203a\u3016\u3017\u3011\u3010\u300f\u300e\u3015\u3014\u300b\u300a\ufe50\ufe55\ufe30\ufe54\uff1f\ufe56\ufe4c\ufe4f\ufe4b\uff07\u02ca\u02cb\u2015\ufe6b\ufe33\ufe34\uff3f\uffe3\ufe62\ufe66\ufe64\u2010\u02dc\ufe5f\ufe69\ufe60\ufe6a\ufe61\ufe68\ufe4d\ufe49\ufe4e\ufe4a\u02c7\ufe35\ufe36\ufe37\ufe38\ufe39\ufe3f\ufe40\ufe3a\ufe3d\ufe3e\u02c9\ufe41\ufe42\ufe43\ufe44\ufe3b\ufe3c\uff0c\uff08\uff09]+)";
	boost::regex expression(regstr);

	vecFilterWord.clear();
	for(i=0;i<vecQuaryWord.size();i++)
	{
		testString = vecQuaryWord[i];
		boost::smatch what;
		std::string::const_iterator start = testString.begin();
		std::string::const_iterator end = testString.end();
		while( boost::regex_search(start, end, what, expression) )
		{
		    std::string FilterString(what[2].first, what[2].second);
		    //std::cout<< "FilterString:" <<FilterString.c_str() << std::endl;		    

			vecFilterWord.push_back(FilterString);
			start = what[0].second;
		}
	}
	
	return 0;
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
	//vector< float > vecDocFeat;
	vector< float > vecDocFeat( DIM_DOC2VEC, 0 );			//set 0
	vector< float > vecDocNormFeat;
	vector< string > vecDocString;
	map< int, float > mapDocFeat;
	map< int, float >::iterator itDocFeat;
	map< string, vector< float > >::iterator 	itQuaryWord;	//[In]quary word
	map< string, float >::iterator 				itQuaryTf;		//[In]quary TF
	map< string, vector< float > >::iterator 	itDicWord;		//[In]dic word
	map< string, float >::iterator 				itDicWordDist;	//[In]dic word distance
	map< string, float >::iterator 				itDicIdf;		//[In]dic IDF

	/*****************************word2doc*****************************/
	//Read Quary Info
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

		//dic idf
		itDicIdf = mapDicIdf.find(wordQuary);
		if (itDicIdf != mapDicIdf.end()) // find it
			dicIdf = itDicIdf->second;
		else
			continue;

		//doc Feat
		for(i=0; i<vecQuary.size(); i++)
		{
			vecDocFeat[i] += quaryTf*dicIdf*vecQuary[i];
		}			
	}
	for(i=0; i<vecQuary.size(); i++)
	{
		vecDocFeat[i] = vecDocFeat[i]*1.0/mapQuaryWord.size();
	}	
	nRet = Normal_L2( vecDocFeat, vecDocNormFeat );
	if ( nRet != 0 ) 
	{
		printf("Normal_L2 err!!\n");
		return nRet;
	}

	/*****************************word2doc*****************************/
	vecDocString.clear();
	for(itQuaryWord = mapQuaryWord.begin(); itQuaryWord != mapQuaryWord.end(); itQuaryWord++)
	{
		wordQuary = itQuaryWord->first;
		vecDocString.push_back( wordQuary );
	}

	//write info
	mapQuaryDoc[vecDocString] = vecDocNormFeat;

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
	map< string, int >::iterator 					itSensitiveWords;	//[In]dic word
	
	//doc2vec_cutword
	outQueryWords.clear();
	for ( j=0;j<vecQuaryWord.size();j++ )
	{
		queryCutWords.clear();
		//Cut Word
		app->cut( vecQuaryWord[j], queryCutWords, METHOD_MIX );

		//Check && Save GetFeat
		for ( i=0;i<queryCutWords.size();i++ )
		{
			if ( (queryCutWords[i].empty()) || (queryCutWords[i] == " ") || (queryCutWords[i] == "	") 
					|| (IsDigit2(queryCutWords[i])) || (queryCutWords[i].size()<6) 
					|| (!IsAllChinese(queryCutWords[i])) )
			{
				continue;
			}
			
			itStopwords = stopwords.find(queryCutWords[i]);
			if (itStopwords == stopwords.end()) // not find
			{
				//remove 3rd Sensitive Words
				itSensitiveWords = SensitiveWords.find(queryCutWords[i]);
				if ( (itSensitiveWords == SensitiveWords.end()) || 
					( (itSensitiveWords != SensitiveWords.end()) && ( itSensitiveWords->second != 3 ) ) ) 
				{
					outQueryWords.push_back( queryCutWords[i] );
				}
			}			
		}	
	}

	//check words
	if ( outQueryWords.size() < 3 )
	{
		outQueryWords.clear();
		mapQuaryDoc.clear();
		//printf( "outQueryWords.size()<3!!\n" );
		return -1; 
	}

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
		//printf( "idQuarySize:%ld, do not find KeyWord!!\n", outQueryWords.size() );
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

int API_DOC2VEC::ExtractFeat( 
	vector< string > 								vecQuaryWord,	//[In]quary word
	map< vector< string >, vector< float > > 		&mapFeat )		//[Out]Feat
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
	nRet = doc2vec( vecQuaryWord, mapFeat );
	if (nRet != 0)
	{	
	   	//cout<<"Fail to ExtractFeat:doc2vec !! "<<endl; 
	   	return nRet;
	}	

	return nRet;
}

void API_DOC2VEC::ErrPredict( map< vector< string >, vector< pair< int, float > > > &resPredict )
{
	string str = "Do Not Find KeyWord!!";
	int label = 0;
	float score = 2.0;

	vector< string > vecStr;
	vector< pair< int, float > > vecLabelScore;

	vecStr.push_back( str );
	vecLabelScore.push_back( std::make_pair( label, score ) );

	resPredict.clear();
	resPredict[vecStr] = vecLabelScore;
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
	int i,nRet = 0;
	long queryStringSize = 0;
	map< vector< string >, vector< float > > mapPCAFeat;
	map< vector< string >, vector< float > >::iterator itPCAFeat;

	vector< string > vecFilterWord;
	vector< vector< float > > 			PredictFeat;
	vector< pair< int, float > > 		tmpPredict;

	map< vector< string >, vector< pair< int, float > > > 	inMergePredict;

	/*****************************FilterQuaryString*****************************/
	nRet = FilterQuaryString( vecQuaryWord, vecFilterWord );
	if ( ( nRet != 0 ) || ( vecFilterWord.size() == 0 ) )
	{	
		ErrPredict(resPredict);
	   	return 0;
	}	
	
	/*****************************Process*****************************/
	nRet = ExtractFeat( vecFilterWord, mapPCAFeat );
	if ( ( nRet != 0 ) || ( mapPCAFeat.size() == 0 ) )
	{	
		ErrPredict(resPredict);
	   	return 0;
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
	   	//cout<<"Fail to Predict SVM!! "<<endl; 
	   	return nRet;
	}	

	/************************get info*****************************/
	inMergePredict.clear();
	for(itPCAFeat = mapPCAFeat.begin(); itPCAFeat != mapPCAFeat.end(); itPCAFeat++)
	{
		inMergePredict[itPCAFeat->first] = tmpPredict;
	}

	/************************queryStringSize*****************************/	
	queryStringSize = 0;
	for ( i = 0; i < vecFilterWord.size(); i++ )
	{
		queryStringSize += vecFilterWord[i].size();
	}

	/************************Merge*****************************/	
	resPredict.clear();
	nRet = Merge( inMergePredict, queryStringSize, resPredict );
	if ( nRet != 0 )
	{	
	   	//cout<<"Fail to Predict SVM!! "<<endl; 
	   	return nRet;
	}

	return nRet;
}

int API_DOC2VEC::Merge(
	map< vector< string >, vector< pair< int, float > > > 	inPredict,
	long 													queryStringSize,
	map< vector< string >, vector< pair< int, float > > > 	&outPredict)
{
	if ( inPredict.size() == 0 )
	{
		printf( "API_DOC2VEC::Merge err!!inPredict.size:%ld!!\n", inPredict.size() );
		return -1; 
	}

	/*****************************Init*****************************/
	int nlabel,nRet = 0;
	map< vector< string >, vector< pair< int, float > > >::iterator itInPredict;

	vector< string > vecString;
	vector< pair< int, float > > vecRes;

	/*****************************Merge*****************************/
	outPredict.clear();
	for(itInPredict = inPredict.begin(); itInPredict != inPredict.end(); itInPredict++)
	{
		//if ( itInPredict->first.size()>1 )
		//if ( ( itInPredict->first.size()>1 ) && ( ( itInPredict->second[0].second >=0.99995 ) || ( queryStringSize>=45 ) ) )
		if ( ( itInPredict->first.size()>2 ) && ( queryStringSize>=45 ) )
		{
			outPredict[itInPredict->first] = itInPredict->second;
		}
		else
		{
			ErrPredict(outPredict);
		}
	}

	return nRet;
}

int API_DOC2VEC::FindSimilarWord(
	vector< string > 									vecQuaryWord,	//[In]quary word
	map< string , vector< pair< string, float > > > 	&resSimWord)	//[Out]Res:string-useful words,int-label,float-score
{
	if ( vecQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::doc2vec err!!vecQuaryWord.size:%ld, do not find KeyWord!!\n", vecQuaryWord.size() );
		return -1; 
	}	
	
	/*****************************Init:Commen*****************************/
	int nRet = 0;
	float queryDist,dicDist, distCosine;
	string word,wordQuary,wordDic;
	long long i, j, Count;

	/*****************************Init*****************************/
	vector< float > vecQuary;
	vector< float > vecDic;
	vector<string> outQueryWords;
	vector<string> queryCutWords;
	vector< pair< string, float > > distQueryCode; //every query 
	vector< pair< string, float > > res; //every query 
	map< string, vector< float > > 					mapQuaryWord;	//[In]quary word
	map< string,int >::iterator 					itStopwords;
	map< string, vector< float > >::iterator 		itQuaryWord;	//[In]quary word
	map< string, vector< float > >::iterator 		itDicWord;		//[In]dic word
	map< string, float >::iterator 					itDicWordDist;	//[In]dic word distance
	
	//doc2vec_cutword
	outQueryWords.clear();
	for ( j=0;j<vecQuaryWord.size();j++ )
	{
		queryCutWords.clear();
		//Cut Word
		app->cut( vecQuaryWord[j], queryCutWords, METHOD_MIX );

		//Check && Save GetFeat
		for ( i=0;i<queryCutWords.size();i++ )
		{
			if ( (queryCutWords[i].empty()) || (queryCutWords[i] == " ") || (queryCutWords[i] == "	") 
					|| (IsDigit2(queryCutWords[i])) || (queryCutWords[i].size()<4) 
					|| (!IsAllChinese(queryCutWords[i])) )
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

	mapQuaryWord.clear();	//[In]quary word
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
	}

	//check
	if ( mapQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::doc2vec err!!vecQuaryWord.size:%ld, do not find KeyWord!!\n", mapQuaryWord.size() );
		return -1; 
	}	

	/*****************************find*****************************/
	resSimWord.clear();
	for(itQuaryWord = mapQuaryWord.begin(); itQuaryWord != mapQuaryWord.end(); itQuaryWord++)
	{
		//quary info
		vecQuary.clear();
		wordQuary = itQuaryWord->first;
		vecQuary = itQuaryWord->second;
		/************************Distance_L2*****************************/
		nRet = Distance_L2( vecQuary, queryDist );	//quary distance : L1
		if ( nRet != 0 ) 
		{
			printf("word2doc:Distance_Cosine err!!\n");
			continue;
		}

		/************************Distance_Cosine*****************************/
		distQueryCode.clear();
		for(itDicWord = mapDicWord.begin(); itDicWord != mapDicWord.end(); itDicWord++)
		{
			//dic info
			vecDic.clear();
			wordDic = itDicWord->first;
			vecDic = itDicWord->second;

			//find dicDist
			dicDist = 0;
			itDicWordDist = mapDicWordDist.find(wordDic);
			if (itDicWordDist != mapDicWordDist.end()) // find
				dicDist = itDicWordDist->second;
			else
				continue;
		
			//Distance_Cosine
			distCosine = 0;
			nRet = Distance_Cosine( vecQuary, vecDic, queryDist, dicDist, distCosine );
			if ( nRet != 0 ) 
			{
				printf("word2doc:Distance_Cosine err!!\n");
				return nRet;
			}
			//push data
			distQueryCode.push_back( make_pair( wordDic, distCosine ) );
		}

		//sort label result
		sort(distQueryCode.begin(), distQueryCode.end(),ImgSortComp);	
		
		res.clear();
		res.assign( distQueryCode.begin(), distQueryCode.begin()+100 );
		resSimWord[wordQuary] = res;

	}

	return nRet;
}

int API_DOC2VEC::FindSimilarWord_HammingCode(
	vector< string > 									vecQuaryWord,	//[In]quary word
	map< string , vector< pair< string, float > > > 	&resSimWord)	//[Out]Res:string-useful words,int-label,float-score
{
	if ( vecQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::doc2vec err!!vecQuaryWord.size:%ld, do not find KeyWord!!\n", vecQuaryWord.size() );
		return -1; 
	}	
	
	/*****************************Init:Commen*****************************/
	int distance, nRet = 0;
	float queryDist,dicDist, distCosine;
	string word,wordQuary,wordDic;
	long long i, j, Count, sumStringCount;
	UInt64 code;

	/*****************************Init*****************************/
	vector< float > vecQuary;
	vector< float > vecDic;
	vector<string> outQueryWords;
	vector<string> queryCutWords;
	vector< pair< UInt64, int > > distQueryCode; //every query 
	vector< pair< string, float > > distQuery; //every query 
	vector< pair< string, float > > res; //every query 
	map< string, vector< float > > 					mapQuaryWord;	//[In]quary word
	map< string,int >::iterator 					itStopwords;
	map< string, vector< float > >::iterator 		itQuaryWord;	//[In]quary word
	map< string, vector< float > >::iterator 		itDicWord;		//[In]dic word
	map< string, float >::iterator 					itDicWordDist;	//[In]dic word distance
	map< UInt64, vector< string > >::iterator 		itDicCode;		//[In]dic word
	vector< string > vecQuaryDocString;
	
	//doc2vec_cutword
	outQueryWords.clear();
	for ( j=0;j<vecQuaryWord.size();j++ )
	{
		queryCutWords.clear();
		//Cut Word
		app->cut( vecQuaryWord[j], queryCutWords, METHOD_MIX );

		//Check && Save GetFeat
		for ( i=0;i<queryCutWords.size();i++ )
		{
			if ( (queryCutWords[i].empty()) || (queryCutWords[i] == " ") || (queryCutWords[i] == "	") 
					|| (IsDigit2(queryCutWords[i])) || (queryCutWords[i].size()<4) 
					|| (!IsAllChinese(queryCutWords[i])) )
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

	mapQuaryWord.clear();	//[In]quary word
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
	}

	//check
	if ( mapQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::doc2vec err!!vecQuaryWord.size:%ld, do not find KeyWord!!\n", mapQuaryWord.size() );
		return -1; 
	}	

	/*****************************find*****************************/
	resSimWord.clear();
	for(itQuaryWord = mapQuaryWord.begin(); itQuaryWord != mapQuaryWord.end(); itQuaryWord++)
	{
		//quary info
		vecQuary.clear();
		wordQuary = itQuaryWord->first;
		vecQuary = itQuaryWord->second;
		/************************Distance_L2*****************************/
		nRet = Distance_L2( vecQuary, queryDist );	//quary distance : L1
		if ( nRet != 0 ) 
		{
			printf("word2doc:Distance_Cosine err!!\n");
			continue;
		}
		
		//get Distance_Hamming
		distQueryCode.clear();
		for(itDicCode = mapDicCode.begin(); itDicCode != mapDicCode.end(); itDicCode++)
		{
			code = 0;
			nRet = EncodeFeature( vecQuary, code );	//encode feat
			if ( nRet != 0 ) 
			{
				printf("doc2vec:EncodeFeature err!!\n");
				continue;
			}
		
			distance = Distance_Hamming( code, itDicCode->first );
			distQueryCode.push_back( make_pair( itDicCode->first, distance ) );
		}

		//sort label result
		sort(distQueryCode.begin(), distQueryCode.end(),ImgSortComp2);	

		vecQuaryDocString.clear();
		for(i=0;i<distQueryCode.size();i++)
		{
			//save string	
			itDicCode = mapDicCode.find(distQueryCode[i].first);	
			if ( itDicCode != mapDicCode.end() )  // find it	
			{
				for(j=0;j<itDicCode->second.size();j++)
					vecQuaryDocString.push_back(itDicCode->second[j]);
			}

			//query strings Use 1000 doc strings
			if ( vecQuaryDocString.size()>1000 )
				break;
		}

		/************************Distance_Cosine*****************************/
		distQuery.clear();
		for(i=0;i<vecQuaryDocString.size();i++)
		{
			//dic info
			wordDic = vecQuaryDocString[i];
			
			//find dicWord
			vecDic.clear();
			itDicWord = mapDicWord.find(wordDic);
			if (itDicWord != mapDicWord.end()) // find
				vecDic.assign( itDicWord->second.begin(), itDicWord->second.end() );
			else
				continue;

			//find dicDist
			dicDist = 0;
			itDicWordDist = mapDicWordDist.find(wordDic);
			if (itDicWordDist != mapDicWordDist.end()) // find
				dicDist = itDicWordDist->second;
			else
				continue;
		
			//Distance_Cosine
			distCosine = 0;
			nRet = Distance_Cosine( vecQuary, vecDic, queryDist, dicDist, distCosine );
			if ( nRet != 0 ) 
			{
				printf("word2doc:Distance_Cosine err!!\n");
				return nRet;
			}
			
			//push data
			distQuery.push_back( make_pair( wordDic, distCosine ) );
		}
		
		//sort label result
		sort(distQuery.begin(), distQuery.end(),ImgSortComp);	

		res.clear();
		res.assign( distQuery.begin(), distQuery.begin()+100 );
		resSimWord[wordQuary] = res;
	}

	return nRet;
}



/***********************************Release*************************************/
void API_DOC2VEC::Release()
{
	/***********************************SVM Model******libsvm3.2.0********************/
	api_svm.Release();

	/***********************************Release**********************************/
	stopwords.clear();
	SensitiveWords.clear();
	mapDicWord.clear(); 	//[In]dic word
	mapDicWordDist.clear(); //[In]dic word distance
	mapDicIdf.clear();		//[In]dic IDF
	mapDicCode.clear();		//[In]dic code-words
}

/***********************************String IsDigit2*************************************/
bool API_DOC2VEC::IsDigit2(string str) 
{ 
	for(int i=0;i<str.size();i++) 
	{   
		if ( ((str.at(i)>='0') && (str.at(i)<='9')) || (str.at(i)=='.') )
			continue;
		else
			return false; 
	} 
	return true; 
} 

/***********************************String Is AllChinese*************************************/
bool API_DOC2VEC::IsAllChinese(string str) 
{ 
	for(int i=0;i<str.size();i++) 
	{   
		if ( (str.at(i)&0x80) && ((unsigned)str.at(i)>=0) ) {	
			i++;
			continue;
		} 
		else
			return false; 
	} 
	return true; 
}

/***********************************Normal_L2*************************************/
int API_DOC2VEC::Normal_L2( vector<float> inFeat, vector<float> &NormFeat )
{
	if (inFeat.size()==0)
		return TEC_INVALID_PARAM;
		
	int j;
	double Sum = 0.0;

	/************************Normalization*****************************/
	for ( j=0;j < inFeat.size();j++ )
	{
		Sum += pow(inFeat[j],2);
	}
	Sum = sqrt(Sum);

	/************************Normalization*****************************/
	NormFeat.clear();
	for (j = 0; j < inFeat.size(); j++) 
	{
		if ( Sum == 0 )
			NormFeat.push_back( 0 );
		else
	  		NormFeat.push_back( inFeat[j]*1.0/Sum );
	}

	return TOK;
}

/***********************************Distance_L2*************************************/
int API_DOC2VEC::Distance_L2( vector< float > quaryWord, float&dist )
{
	if ( quaryWord.size() == 0 )
		return -1;
		
	int j, nRet = 0;
	float SumQuary = 0.0;

	for ( j=0;j < quaryWord.size();j++ )
	{
		SumQuary += pow(quaryWord[j],2);
	}
	dist = sqrt(SumQuary);

	return nRet;
}


/***********************************Distance_Cosine*************************************/
int API_DOC2VEC::Distance_Cosine( 
	vector< float > quaryWord, 		//[In]quary word
	vector< float > dicWord, 		//[In]dic word
	float 			queryWordDist, 	//[In]query word Dist
	float 			dicWordDist, 	//[In]dic word Dist
	float			&dist )			//[Out]cosine distance
{
	if ( (quaryWord.size()==0) || (dicWord.size()==0) || (quaryWord.size()!=dicWord.size()) || (queryWordDist<=0) || (dicWordDist<=0) )
		return -1;
		
	int i, j, nRet = 0;
	float tmpDistance,SumQuary = 0.0;
	SumQuary = queryWordDist * dicWordDist;			//quary distance * dic distance
	if ( SumQuary == 0 )
		return -1;

	/************************cosine distance*****************************/
	tmpDistance = 0;
	for (j = 0; j < quaryWord.size(); j++) 
	{
		tmpDistance += quaryWord[j]*dicWord[j];
	}
	dist = tmpDistance*1.0/SumQuary;

	//printf("Distance_Cosine:queryWordDist-%.4f,dicWordDist-%.4f,tmpDistance-%.4f,dist-%.4f\n",
	//	queryWordDist,dicWordDist,tmpDistance,dist);

	return nRet;
}

/**********************************Normal_MinMax*********************************/
int API_DOC2VEC::Normal_MinMax( vector<float> inFeat, vector<float> &NormFeat )
{
	if (inFeat.size()==0)
		return TEC_INVALID_PARAM;
		
	int j;
	float tmpMin = 1000000.0;
	float tmpMax = -1000000.0;

	/************************Normalization*****************************/
	for ( j=0;j < inFeat.size();j++ )
	{
		if ( inFeat[j] < tmpMin )
			tmpMin = inFeat[j];
		if ( inFeat[j] > tmpMax )
			tmpMax = inFeat[j];
	}

	/************************Normalization*****************************/
	NormFeat.clear();
	for (j = 0; j < inFeat.size(); j++) 
	{
		if ( tmpMax == tmpMin )
			NormFeat.push_back( 0 );
		else
	  		NormFeat.push_back( (inFeat[j]-tmpMin)*1.0/(tmpMax-tmpMin) );
	}

	return TOK;
}

/**********************************encode feat to hamming code*********************************/
int API_DOC2VEC::EncodeFeature( vector< float > pFeat, unsigned long long &code )
{
	int i, nDim, nRet=TOK;;
	vector<float> NormFeat;

	if (pFeat.size()==0)
		return TEC_INVALID_PARAM;

	NormFeat.clear();
	nRet = Normal_MinMax( pFeat, NormFeat );
	if ( nRet != 0)
	{
		return TEC_INVALID_PARAM;
	}

	code = 0;
	nDim = (NormFeat.size()>8)?8:(NormFeat.size());
	for (i = 0; i < nDim; i++) {
		code <<= 1;
		code |= ((NormFeat[i]>0.5)?1:0);
	}
	
	return nRet;
}

/**********************************Distance_Hamming*********************************/
int API_DOC2VEC::Distance_Hamming( unsigned long long code1, unsigned long long code2 )
{
	int distance = 0;
   	unsigned long long val = code1 ^ code2;
 
	// Count the number of set bits
	while( val )
	{
		++distance; 
		val &= val - 1;
	}

	return distance;
}


char API_DOC2VEC::dec2hexChar(short int n) {  
 if ( 0 <= n && n <= 9 ) {	
  return char( short('0') + n );  
 } else if ( 10 <= n && n <= 15 ) {  
  return char( short('A') + n - 10 );  
 } else {  
  return char(0);  
 }	
}  
  
short int API_DOC2VEC::hexChar2dec(char c) {  
 if ( '0'<=c && c<='9' ) {	
  return short(c-'0');	
 } else if ( 'a'<=c && c<='f' ) {  
  return ( short(c-'a') + 10 );  
 } else if ( 'A'<=c && c<='F' ) {  
  return ( short(c-'A') + 10 );  
 } else {  
  return -1;  
 }	
}  
  
string API_DOC2VEC::URL_Encode(const string &URL)  
{  
 string result = "";  
 for ( unsigned int i=0; i<URL.size(); i++ ) {	
  char c = URL[i];	
  if (	
   ( '0'<=c && c<='9' ) ||	
   ( 'a'<=c && c<='z' ) ||	
   ( 'A'<=c && c<='Z' ) ||	
   c=='/' || c=='.'  
   ) {	
   result += c;  
  } else {	
   int j = (short int)c;  
   if ( j < 0 ) {  
	j += 256;  
   }  
   int i1, i0;	
   i1 = j / 16;  
   i0 = j - i1*16;	
   result += '%';  
   result += dec2hexChar(i1);  
   result += dec2hexChar(i0);  
  }  
 }	
 return result;  
}  
  
string API_DOC2VEC::URL_Decode(const string &URL) 
{  
 string result = "";  
 for ( unsigned int i=0; i<URL.size(); i++ ) {	
  char c = URL[i];	
  if ( c != '%' ) {  
   result += c;  
  } else {	
   char c1 = URL[++i];	
   char c0 = URL[++i];	
   int num = 0;  
   num += hexChar2dec(c1) * 16 + hexChar2dec(c0);  
   result += char(num);  
  }  
 }	
 return result;  
}  


// calculate entropy
int API_DOC2VEC::ExtractFeat_Entropy( vector< float > feat, double &entropy )
{
	int i,nRet=0;
	vector<float> vecNormFeat;

	//
	nRet = Normal_L2( feat, vecNormFeat );
	if ( nRet != 0 ) 
	{
		printf("Normal_L2 err!!\n");
		return nRet;
	}

	entropy = 0;
	for( i =0;i<vecNormFeat.size();i++ )
	{
		if(vecNormFeat[i]==0.0)
			entropy = entropy;
		else
			entropy -= vecNormFeat[i]*(log(vecNormFeat[i])/log(2.0));
	}
	
	return nRet; 
}
/*
// calculate entropy
int API_DOC2VEC::ExtractFeat_Entropy( 
	vector< string > 								vecQuaryWord, 
	double 											MaxEntropy[2],
	pair< int, double > 							pairRes )
{
	if ( vecQuaryWord.size() == 0 )
	{
		printf( "API_DOC2VEC::Predict err!!vecQuaryWord.size:%ld!!\n", vecQuaryWord.size() );
		return -1; 
	}
	
	int labelPredict, nRet = 0;
	double entropy, maxEntropy, resRelativity;
	vector< string > vecFilterWord;
	map< vector< string >, vector< float > > mapFeat;
	map< vector< string >, vector< float > >::iterator itFeat;

	map< vector< string >, vector< pair< int, float > > > 			resPredict;
	map< vector< string >, vector< pair< int, float > > >::iterator itPredict;
	
	//
	vecFilterWord.clear();
	nRet = FilterQuaryString( vecQuaryWord, vecFilterWord );
	if ( ( nRet != 0 ) || ( vecFilterWord.size() == 0 ) )
	{	
		return -1; 
	}	
	
	//
	mapFeat.clear();
	nRet = ExtractFeat( vecFilterWord, mapFeat );
	if ( ( nRet != 0 ) || ( mapFeat.size() == 0 ) )
	{	
		return -1; 
	}	

	//
	for(itFeat = mapFeat.begin(); itFeat != mapFeat.end(); itFeat++)
	{
		entropy = 0;
		nRet = ExtractFeat_Entropy( itFeat->second, entropy );
		if ( ( nRet != 0 ) || ( mapFeat.size() == 0 ) )
		{	
			return -1; 
		}
	}

	//
	resPredict.clear();
	nRet = Predict( vecQuaryWord, resPredict );
	if ( ( nRet != 0 ) || ( resPredict.size() == 0 ) )
	{	
		return -1; 
	}	

	//
	for(itPredict = resPredict.begin(); itPredict != resPredict.end(); itPredict++)
	{
		labelPredict = itPredict->second[0].first;
		if ( labelPredict == 1 )
			maxEntropy = MaxEntropy[0];
		else if ( labelPredict == 0 )
			maxEntropy = MaxEntropy[1];
	}

	//resRelativity
	resRelativity = 1 - entropy*1.0/maxEntropy;
	
	return nRet; 
}*/






