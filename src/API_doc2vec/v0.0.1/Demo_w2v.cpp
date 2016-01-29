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

#include "API_doc2vec.h"
#include "API_commen.h"
#include "API_pca.h"
#include "Application.hpp"

using namespace CppJieba;
using namespace std;

const long long max_size = 2000;         // max length of strings
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 200;              // max length of vocabulary entries

int cutword_GetAdsString( char *szQueryList, char *szUrlResult, char *szAdsResult )
{
	/*****************************Init*****************************/
	char loadPath[4096];
	int nRet = 0;
	long long first,count = 0;
	FILE *fpListFile = 0, *fpUrlOut = 0, *fpAdsOut = 0;
	
	/*****************************Init*****************************/
	string docString,cutString,idString,urlString,adsString;
	
	/********************************Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	fpUrlOut = fopen(szUrlResult, "wt");
	if (!fpUrlOut)
	{
		cout << "Can't open szUrlResult file " << szUrlResult << endl;
		return -1;
	}

	fpAdsOut = fopen(szAdsResult, "wt");
	if (!fpAdsOut)
	{
		cout << "Can't open szAdsResult file " << szAdsResult << endl;
		return -1;
	}

	cout << "[Get Ads String]" << endl;
	count = 0;
	/*****************************Process one by one*****************************/
	while(EOF != fscanf(fpListFile, "%s", &loadPath ))
	{
		docString = loadPath;		
		if ( (docString == " ") || (docString == "	") )
			continue;

		count++;
		if ( count == 1 )
			continue;	
		if ( count % 1000 == 0 )
			printf( "load %d info...\n", count );

		//check
		if (count<5)
			printf( "loadString:%s\n", docString.c_str() );

		//cut
		first = docString.find_first_of(",");
		if(first == string::npos) {   
			printf( "load %s!! not find any characters!!\n", docString.c_str() );
            continue;  
        }   
		//id
		idString = docString.substr( 0, first );	
		cutString = docString.substr( first+1, docString.size() );

		//cut
		first = cutString.find_first_of(",");
		if(first == string::npos) {   
			printf( "load %s!! not find any characters!!\n", cutString.c_str() );
            continue;  
        }   
		//url string
		urlString = cutString.substr( 0, first );	
		urlString = "http://inimg01.jiuyan.info" + urlString;
		docString = cutString.substr( first+1, cutString.size() );

		//cut
		first = docString.find_first_of(",");
		if(first == string::npos) {   
			printf( "load %s!! not find any characters!!\n", docString.c_str() );
            continue;  
        }   
		//ads string
		adsString = docString.substr( 0, first );
		
		//check
		if (count<5)
		{
			printf( "idString:%s\n", idString.c_str() );
			printf( "urlString:%s\n", urlString.c_str() );
			printf( "adsString:%s\n", adsString.c_str() );
		}

		/************************Save String*****************************/
		fprintf(fpUrlOut, "%s %s\n", idString.c_str(), urlString.c_str() );
		fprintf(fpAdsOut, "%s %s 1\n", idString.c_str(), adsString.c_str() );

	}
	printf("count:%ld\n",count);

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpUrlOut) {fclose(fpUrlOut);fpUrlOut = 0;}
	if (fpAdsOut) {fclose(fpAdsOut);fpAdsOut = 0;}
	
	cout<<"Done!! "<<endl;
	
	return nRet;
}

int cutword_GetNoAdsString( char *szQueryList, char *szResult )
{
	/*****************************Init*****************************/
	char loadPath[4096];
	int nRet = 0;
	long long count = 0;
	unsigned long long randomID;
	FILE *fpListFile = 0, *fpOut = 0;
	API_COMMEN api_commen;
	
	/*****************************Init*****************************/
	string docString;
	
	/********************************Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	fpOut = fopen(szResult, "wt");
	if (!fpOut)
	{
		cout << "Can't open szUrlResult file " << szResult << endl;
		return -1;
	}

	cout << "[Get No Ads String ]" << endl;
	count = 0;
	/*****************************Process one by one*****************************/
	while(EOF != fscanf(fpListFile, "%s", &loadPath ))
	{
		docString = loadPath;		
		if ( (docString == " ") || (docString == "	") )
			continue;

		count++;
		if ( count == 1 )
			continue;	
		if ( count % 1000 == 0 )
			printf( "load %d info...\n", count );

		api_commen.getRandomID( randomID );

		//check
		if (count<5)
			printf( "%lld %s 0\n", randomID, docString.c_str() );

		/************************Save String*****************************/
		fprintf(fpOut, "%lld %s 0\n", randomID, docString.c_str() );

	}
	printf("count:%ld\n",count);

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpOut) {fclose(fpOut);fpOut = 0;}
	
	cout<<"Done!! "<<endl;
	
	return nRet;
}

int cutword_RemoveSimilarString( char *szQueryList, char *szResult, int binString )
{
	/*****************************Init*****************************/
	char loadPath[4096];
	char idChar[4096];
	char labelChar[4096];
	int nRet = 0;
	long long count = 0;
	FILE *fpListFile = 0, *fpFeatOut = 0;
	
	API_COMMEN api_commen;
	
	/*****************************Init*****************************/
	string docString,queryString,id,label;
	map< string,int > docCount;
	map< string,int >::iterator itDocCount;
	
	/********************************Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	fpFeatOut = fopen(szResult, "wt");
	if (!fpFeatOut)
	{
		cout << "Can't open result file " << szResult << endl;
		return -1;
	}

	cout << "[Remove Similar String]" << endl;
	count = 0;
	/*****************************Process one by one*****************************/
	if ( binString == 1 )	//id string label
	{
		while(EOF != fscanf(fpListFile, "%s %s %s", &idChar, &loadPath, &labelChar ))
		{
			id = idChar;
			docString = loadPath;		
			label = labelChar;
			if ( (docString == " ") || (docString == "	") || (!api_commen.IsDigit2(id)) || (!api_commen.IsDigit2(label)) )
				continue;
			
			count++;
			if ( count % 1000 == 0 )
				printf( "load %d info...\n", count );

			//find
			itDocCount = docCount.find(docString);
			if (itDocCount == docCount.end()) // not find
			{
				docCount[docString] = 1;
				/************************Save String*****************************/
				fprintf(fpFeatOut, "%s %s %s\n", id.c_str(), docString.c_str(), label.c_str() );
			}
			else
			{
				docCount[docString]++;
			}
		}
	}
	else	//all are string
	{
		while(EOF != fscanf(fpListFile, "%s", &loadPath ))
		{
			docString = loadPath;		
			if ( (docString == " ") || (docString == "	") )
				continue;

			count++;
			if ( count == 1 )
				continue;	
			if ( count % 1000 == 0 )
				printf( "load %d info...\n", count );

			//find
			itDocCount = docCount.find(docString);
			if (itDocCount == docCount.end()) // not find
			{
				docCount[docString] = 1;
				/************************Save String*****************************/
				fprintf(fpFeatOut, "%s\n", docString.c_str() );
			}
			else
			{
				docCount[docString]++;
			}
		}
	}
	
	printf("sumWord:%ld\n",docCount.size());

	docCount.clear();

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}
	
	cout<<"Done!! "<<endl;
	
	return nRet;
}

int cutword_cutword( char *szQueryList, char *szResult )
{
	/*****************************Init*****************************/
	char loadPath[4096];
	int i, j, nRet = 0;
	long nCount;
	FILE *fpListFile = 0, *fpStopWord = 0, *fpFeatOut = 0;

	API_COMMEN api_commen;
	
	/*****************************Init*****************************/
	CppJieba::Application app("/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/jieba.dict.utf8",
	                        "/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/hmm_model.utf8",
	                        "/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/user.dict.utf8",
	                        "/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/idf.utf8",
	                        "/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/stop_words.utf8");
	vector<string> words;
	vector<string> outputWords;
	map< string,int > stopwords;
	map< string,int >::iterator itStopwords;
	string s;
	string result;
	
	/********************************Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	char FileStopwords[1024] = "/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/stop_words.utf8";
	fpStopWord = fopen(FileStopwords,"r");
	if (!fpStopWord) 
	{
		cout << "Can't open " << FileStopwords << endl;
		return -1;
	}

	fpFeatOut = fopen(szResult, "wt");
	if (!fpFeatOut)
	{
		cout << "Can't open result file " << szResult << endl;
		return -1;
	}

	/*****************************Load FileStopwords*****************************/
	stopwords.clear();
	nCount = 0;
	while(EOF != fscanf(fpStopWord, "%s", &loadPath ))
	{
		s = loadPath;		
		if ( s == " " )
			continue;

		nCount++;

		itStopwords = stopwords.find(s);
		if (itStopwords == stopwords.end()) // not find
		{
			stopwords[s] = nCount;
		}
	}

	nCount = 0;
	cout << "[demo] METHOD_MIX" << endl;
	/*****************************Process one by one*****************************/
	while(EOF != fscanf(fpListFile, "%s", &loadPath ))
	{
		words.clear();
		s = loadPath;		
		if ( s == " " )
			continue;

		/************************Cut Word*****************************/
		app.cut(s, words, METHOD_MIX);

		outputWords.clear();
		/************************judge word*****************************/
		for ( i=0;i<words.size();i++ )
		{
			//remove digit || illegal word
			if ((words[i].empty()) || (words[i] == " ") || (words[i] == "	") || 
				(api_commen.IsDigit2(words[i])) || (words[i].size()<2) || (!api_commen.IsAllChinese(words[i])) )
			//if ((words[i].empty()) || (words[i] == " ") || (words[i] == "	") )
				continue;

			//remove stopword
			itStopwords = stopwords.find(words[i]);
			if (itStopwords == stopwords.end()) // not find
			{
				outputWords.push_back( words[i] );
			}
		}	

		if ( outputWords.size() == 0 )
			continue;

		/************************Save GetFeat*****************************/
		for ( i=0;i<outputWords.size();i++ )
		{
			fprintf(fpFeatOut, "%s ", outputWords[i].c_str() );
		}
		fprintf(fpFeatOut, "\n");

		nCount++;
		if( nCount%1000 == 0 )
		{
			printf("Loaded %ld info...\n",nCount);
			printf( "input[%ld]:%s\n", nCount, s.c_str() );
			for ( i=0;i<outputWords.size();i++ )
			{
				printf("%s ", outputWords[i].c_str() );
			}
			printf("\n");
		}
	}

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpStopWord) {fclose(fpStopWord);fpStopWord = 0;}
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}
	
	cout<<"Done!! "<<endl;
	
	return nRet;
}

int cutword_countidf( char *szQueryList, char *szResult )
{
	/*****************************Init*****************************/
	char loadPath[4096];
	int i, j, nRet = 0;
	long long count = 0;
	FILE *fpListFile = 0, *fpFeatOut = 0;

	API_COMMEN api_commen;
	
	/*****************************Init*****************************/
	string word;
	double idf = 0;
	map< string,int > wordCount;
	map< string,int >::iterator itWordCount;
	
	/********************************Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	fpFeatOut = fopen(szResult, "wt");
	if (!fpFeatOut)
	{
		cout << "Can't open result file " << szResult << endl;
		return -1;
	}

	cout << "[Count Idf]" << endl;
	/*****************************Process one by one*****************************/
	while(EOF != fscanf(fpListFile, "%s", &loadPath ))
	{
		word = loadPath;		
		if ( (word == " ") || (word == "	") )
			continue;

		itWordCount = wordCount.find(word);
		if (itWordCount == wordCount.end()) // not find
		{
			wordCount[word] = 1;
		}
		else
		{
			wordCount[word]++;
		}
	}

	count = 0;
	/************************Count Idf*****************************/
	for(itWordCount = wordCount.begin(); itWordCount != wordCount.end(); itWordCount++)
	{
		word = itWordCount->first;
		idf = log10( wordCount.size()*1.0/(1+itWordCount->second) );

		//remove digit || illegal word
		if ( (word.empty()) || (word == " ") || (word == "	") || 
			(api_commen.IsDigit2(word)) || (word.size()<2) || (!api_commen.IsAllChinese(word)) )
			continue;

		/************************Save Idf*****************************/
		fprintf(fpFeatOut, "%s %f\n", word.c_str(), idf );
		count++;
	}
	printf("sumWord:%ld\n",count);

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}
	
	cout<<"Done!! "<<endl;
	
	return nRet;
}

int cutword_PCA_LearnModel( char *szQueryList,  char *Keyfile, char *outPCAModel, char *outPCAFeat )
{
	/*****************************Init:Commen*****************************/
	char loadPath[4096];
	int nRet = 0;
	char tmpChar;
	char vocab[256];
	string word;
	long long nCount, idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;
	FILE *fpListFile = 0, *fpFeatOut = 0;

	API_PCA api_pca;
	API_DOC2VEC api_doc2vec;
	API_COMMEN api_commen;
	
	/*****************************Init:Doc2Vec*****************************/
	vector< string > vecQuaryString;
	vector< string > quaryString;
	map< string, vector< float > > 							mapQuaryWord;	//[In]quary word
	map< vector< string >, vector< float > > 				mapQuaryDoc;	//[Out]quary Doc
	map< vector< string >, vector< float > >::iterator 		itQuaryDoc;		//[In]dic word

	vector< pair< int, vector<float> > > PCA_InFeat;
	
	/********************************Load:Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	fpFeatOut = fopen(outPCAFeat, "wt");
	if (!fpFeatOut)
	{
		cout << "Can't open result file " << outPCAFeat << endl;
		return -1;
	}

	api_doc2vec.Init( Keyfile );
	
	/*****************************Load Query*****************************/
	fscanf(fpListFile, "%lld", &w2v_words);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	nCount = 0;
	for (b = 0; b < w2v_words+1; b++) {
		//load word to vector
		a = 0;
		vecQuaryString.clear();
		while (1) {
			vocab[a] = fgetc(fpListFile);	
			tmpChar = vocab[a];
			if ((a < max_w) && (vocab[a] != ' ') && (vocab[a] != '\n')) a++;
			if ( ( vocab[a] == ' ' ) || feof(fpListFile) || (vocab[a] == '\n') )
			{
				vocab[a] = 0;
				word = vocab;

				if ( ( vocab[0] != ' ' ) && (vocab[0] != '\n') )
					vecQuaryString.push_back( word );

				a = 0;
			}
			if (feof(fpListFile) || (tmpChar == '\n')) break;
		}
		
		if ( vecQuaryString.size() < 3 )
		{
			printf( "vecQuaryString.size():%ld, do not find info!!\n", vecQuaryString.size() );
			continue;
		}
				
		//Quary info
		quaryString.clear();
		idQuary = atol( vecQuaryString[0].c_str() );	//id
		labelQuary = atol( vecQuaryString[vecQuaryString.size()-1].c_str() ); //label
		for ( a = 1; a < vecQuaryString.size()-1; a++ )
		{
			quaryString.push_back(vecQuaryString[a]);
		}

		//word2doc
		mapQuaryDoc.clear();
		nRet = api_doc2vec.doc2vec( quaryString, mapQuaryDoc );
		if ( nRet != 0 ) 
		{
			printf("doc2vec_train:word2doc err!!\n");
			continue;
		}

		if ( mapQuaryDoc.size() == 0 )
		{
			printf( "mapQuaryDoc.size():%ld, do not find info!!\n", vecQuaryString.size() );
			continue;
		}

		//save docfeat
		for(itQuaryDoc = mapQuaryDoc.begin(); itQuaryDoc != mapQuaryDoc.end(); itQuaryDoc++)
		{
			/************************PCA_InFeat*****************************/
			PCA_InFeat.push_back( std::make_pair( int(labelQuary), itQuaryDoc->second ) );
		}
		
		//printf
		nCount++;
		if( nCount%50 == 0 )
			printf("Loaded %ld DocInfo...\n",nCount);

		//check data 
		if (b<4)
		{
			for(i=0;i<vecQuaryString.size();i++)
				printf( "%s ", vecQuaryString[i].c_str() );
			printf( "\n" );
			
			printf("idQuary:%lld,labelQuary:%lld,last words:",idQuary,labelQuary);
			for(itQuaryDoc = mapQuaryDoc.begin(); itQuaryDoc != mapQuaryDoc.end(); itQuaryDoc++)
			{
				//quary info
				for(j=0;j<itQuaryDoc->first.size();j++)
					printf( "%s ", itQuaryDoc->first[j].c_str() );
			}
			printf( "\n" );
		}
	}

	/************************PCA*****************************/
	nRet = api_pca.PCA_Feat_Learn(PCA_InFeat, outPCAModel);
	if (nRet != 0)
	{
	   cout<<"Fail to PCA_Feat_Learn!! "<<endl;
	   return nRet;
	}

	/************************PCA*****************************/
	api_pca.PCA_Feat_Init( outPCAModel );
	
	for ( i=0;i<PCA_InFeat.size();i++ )
	{
		/************************PCA_InFeat*****************************/
		vector<float> EncodeFeat;
		nRet = api_pca.PCA_Feat_Encode( PCA_InFeat[i].second, EncodeFeat );
		if (nRet != 0)
		{
		   cout<<"Fail to PCA_Feat_Encode!! "<<endl;
		   continue;
		}

		vector<float> NormEncodeFeat;
		nRet = api_commen.Normal_L2( EncodeFeat, NormEncodeFeat );
		if (nRet != 0)
		{
		   cout<<"Fail to PCA_Feat_Normal!! "<<endl;
		   continue;
		}

		/************************Save GetFeat*****************************/
		fprintf(fpFeatOut, "%d ", PCA_InFeat[i].first );
		for ( j=0;j<NormEncodeFeat.size();j++ )
		{
			fprintf(fpFeatOut, "%d:%.6f ", j+1, NormEncodeFeat[j]+0.00000001 );
		}
		fprintf(fpFeatOut, "\n");

		/************************Check PCA Model*****************************/
		if (i<3)
		{
			vector<float> DecodeFeat;
			nRet = api_pca.PCA_Feat_Decode( EncodeFeat, PCA_InFeat[i].second.size(), DecodeFeat );
			if (nRet != 0)
			{
			   cout<<"Fail to PCA_Feat_Decode!! "<<endl;
			   return nRet;
			}
			printf("Check PCA Model:");
			//for ( j=0;j<PCA_InFeat[i].second.size();j++ )
			for ( j=0;j<10;j++ )
			{
				printf("%.6f-%.6f ", PCA_InFeat[i].second[j]+0.00000001, DecodeFeat[j]+0.00000001 );
			}
			printf("\n");
		}	
	}
	
	/*********************************Release*************************************/
	api_pca.PCA_Feat_Release();
	api_doc2vec.Release();

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}
	
	cout<<"Done!! "<<endl;
	
	return nRet;
}

int doc2vec_train( char *szQueryList, char *Keyfile, char *szFeat, int binFeat )
{
	/*****************************Init:Commen*****************************/
	char loadPath[4096];
	int nRet = 0;
	char tmpChar;
	char vocab[256];
	string word;
	long long idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;
	FILE *fpListFile = 0, *fpFeatOut = 0;

	API_DOC2VEC api_doc2vec;
	
	/*****************************Init:Doc2Vec*****************************/
	vector< string > vecQuaryString;
	vector< string > quaryString;
	map< string, vector< float > > 							mapQuaryWord;	//[In]quary word
	map< vector< string >, vector< float > > 				mapQuaryDoc;	//[Out]quary Doc
	map< vector< string >, vector< float > >::iterator 		itQuaryDoc;		//[In]dic word
	
	/********************************Load:Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	fpFeatOut = fopen(szFeat, "wt");
	if (!fpFeatOut)
	{
		cout << "Can't open result file " << szFeat << endl;
		return -1;
	}

	api_doc2vec.Init( Keyfile );

	/*****************************Load Query*****************************/
	printf("Load Query...\n");
	fscanf(fpListFile, "%lld", &w2v_words);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words+1; b++) {
		//load word to vector
		a = 0;
		vecQuaryString.clear();
		while (1) {
			vocab[a] = fgetc(fpListFile);	
			tmpChar = vocab[a];
			if ((a < max_w) && (vocab[a] != ' ') && (vocab[a] != '\n')) a++;
			if ( ( vocab[a] == ' ' ) || feof(fpListFile) || (vocab[a] == '\n') )
			{
				vocab[a] = 0;
				word = vocab;

				if ( ( vocab[0] != ' ' ) && (vocab[0] != '\n') )
					vecQuaryString.push_back( word );

				a = 0;
			}
			if (feof(fpListFile) || (tmpChar == '\n')) break;
		}
		
		if ( vecQuaryString.size() < 3 )
		{
			printf( "vecQuaryString.size():%ld, do not find info!!\n", vecQuaryString.size() );
			continue;
		}
				
		//Quary info
		quaryString.clear();
		idQuary = atol( vecQuaryString[0].c_str() );	//id
		labelQuary = atol( vecQuaryString[vecQuaryString.size()-1].c_str() ); //label
		for ( a = 1; a < vecQuaryString.size()-1; a++ )
		{
			quaryString.push_back(vecQuaryString[a]);
		}

		//word2doc
		mapQuaryDoc.clear();
		if ( binFeat == 1 )	//for train
			nRet = api_doc2vec.ExtractFeat( quaryString, mapQuaryDoc );
		else if ( binFeat == 0 ) //for pca
			nRet = api_doc2vec.doc2vec( quaryString, mapQuaryDoc );
		if ( nRet != 0 ) 
		{
			printf("doc2vec_train:word2doc err!!\n");
			continue;
		}

		if ( mapQuaryDoc.size() == 0 )
		{
			printf( "mapQuaryDoc.size():%ld, do not find info!!\n", vecQuaryString.size() );
			continue;
		}

		//save docfeat
		for(itQuaryDoc = mapQuaryDoc.begin(); itQuaryDoc != mapQuaryDoc.end(); itQuaryDoc++)
		{
			/************************Save GetFeat*****************************/
			fprintf(fpFeatOut, "%ld ", labelQuary );
			for ( i=0;i<itQuaryDoc->second.size();i++ )
			{
				fprintf(fpFeatOut, "%ld:%.6f ", i+1, (itQuaryDoc->second[i]+0.00000001) );
			}	
			fprintf(fpFeatOut, "\n");
		}

		//check data 
		if (b<4)
		{
			for(i=0;i<vecQuaryString.size();i++)
				printf( "%s ", vecQuaryString[i].c_str() );
			printf( "\n" );
			
			printf("idQuary:%lld,labelQuary:%lld,last words:",idQuary,labelQuary);
			for(itQuaryDoc = mapQuaryDoc.begin(); itQuaryDoc != mapQuaryDoc.end(); itQuaryDoc++)
			{
				//quary info
				for(j=0;j<itQuaryDoc->first.size();j++)
					printf( "%s ", itQuaryDoc->first[j].c_str() );
			}
			printf( "\n" );
		}

		//printf
		if( b%50 == 0 )
			printf("Loaded %ld DocInfo...\n",b);
	}

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}
	
	api_doc2vec.Release();

	cout<<"Done!! "<<endl;
	
	return nRet;
}

int doc2vec_predict( char *szQueryList, char *Keyfile, char *szResult, int binFeat )
{
	/*****************************Init:Commen*****************************/
	char loadPath[4096];
	int nRet = 0;
	char tmpChar;
	char vocab[256];
	string word;
	double allPredictTime,tPredictTime;
	long long idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;
	FILE *fpListFile = 0, *fpFeatOut = 0;

	API_DOC2VEC api_doc2vec;
	vector< string > vecQuaryString;
	vector< string > quaryString;
	map< vector< string >, vector< pair< int, float > > > 			resPredict;
	map< vector< string >, vector< pair< int, float > > >::iterator itPredict;
	
	/********************************Load:Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	fpFeatOut = fopen(szResult, "wt");
	if (!fpFeatOut)
	{
		cout << "Can't open result file " << szResult << endl;
		return -1;
	}

	api_doc2vec.Init( Keyfile );
	tPredictTime = 0.0;
	allPredictTime = 0.0;

	/*****************************Load Query*****************************/
	fscanf(fpListFile, "%lld", &w2v_words);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words+1; b++) {
		//load word to vector
		a = 0;
		vecQuaryString.clear();
		while (1) {
			vocab[a] = fgetc(fpListFile);	
			tmpChar = vocab[a];
			if ((a < max_w) && (vocab[a] != ' ') && (vocab[a] != '\n')) a++;
			if ( ( vocab[a] == ' ' ) || feof(fpListFile) || (vocab[a] == '\n') )
			{
				vocab[a] = 0;
				word = vocab;

				if ( ( vocab[0] != ' ' ) && (vocab[0] != '\n') )
					vecQuaryString.push_back( word );

				a = 0;
			}
			if (feof(fpListFile) || (tmpChar == '\n')) break;
		}
		
		if ( ( vecQuaryString.size() == 0 ) || ( b == 0 ) )
		{
			printf( "vecQuaryString.size():%ld, do not find info!!b:%lld\n", vecQuaryString.size(), b );
			printf("vecQuaryString:");
			for ( a = 0; a < vecQuaryString.size(); a++ )
			{
				printf( "%s ", vecQuaryString[a].c_str() );
			}
			printf("\n");
			continue;
		}
				
		//Quary info
		quaryString.clear();
		if ( binFeat == 1 )	//id string label
		{
			idQuary = atol( vecQuaryString[0].c_str() );	//id
			labelQuary = atol( vecQuaryString[vecQuaryString.size()-1].c_str() ); //label
			for ( a = 1; a < vecQuaryString.size()-1; a++ )
			{
				quaryString.push_back(vecQuaryString[a]);
			}
		}
		else	//string
		{
			for ( a = 0; a < vecQuaryString.size(); a++ )
			{
				quaryString.push_back(vecQuaryString[a]);
			}
		}

		if ( quaryString.size() == 0 )
		{
			continue;
		}

		//predict
		resPredict.clear();
		tPredictTime = (double)getTickCount();
 		nRet = api_doc2vec.Predict( quaryString, resPredict );	//[Out]Res:string-useful words,int-label,float-scores
 		tPredictTime = (double)getTickCount() - tPredictTime;
		tPredictTime = tPredictTime*1000./cv::getTickFrequency();
		allPredictTime += tPredictTime;
		if ( ( nRet != 0 ) || ( resPredict.size() == 0 ) )
		{
			printf("doc2vec_predict:Predict err!!\n");
			continue;
		}

		//save docfeat
		/************************Save GetFeat*****************************/
		for(i=0;i<quaryString.size();i++)
			fprintf( fpFeatOut, "%s ", quaryString[i].c_str() );
		fprintf(fpFeatOut, "\n" );
			
		for(itPredict = resPredict.begin(); itPredict != resPredict.end(); itPredict++)
		{
			for ( i=0;i<itPredict->first.size();i++ )
			{
				fprintf(fpFeatOut, "%s ", itPredict->first[i].c_str() );
			}
			for ( i=0;i<itPredict->second.size();i++ )
			{
				fprintf(fpFeatOut, "label:%d,score:%.4f ", itPredict->second[i].first, itPredict->second[i].second );
			}	
			fprintf(fpFeatOut, "\n\n");
		}

		//check data 
		//if (b<4)
		{
			for(i=0;i<quaryString.size();i++)
				printf( "%s ", quaryString[i].c_str() );
			printf("\n" );
				
			for(itPredict = resPredict.begin(); itPredict != resPredict.end(); itPredict++)
			{
				for ( i=0;i<itPredict->first.size();i++ )
				{
					printf("%s ", itPredict->first[i].c_str() );
				}
				for ( i=0;i<itPredict->second.size();i++ )
				{
					printf("label:%d,score:%.4f ", itPredict->second[i].first, itPredict->second[i].second );
				}	
				printf("\n\n");
			}
		}

		//printf
		if( b%50 == 0 )
			printf("Loaded %ld DocInfo...\n",b);
	}

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}

	/*********************************Release*************************************/
	api_doc2vec.Release();

	/*********************************Print Info*********************************/
	if ( b != 0 ) 
	{
		printf( "nCount:%lld,PredictTime:%.4fms\n", b, allPredictTime*1.0/b );
	}

	cout<<"Done!! "<<endl;
	
	return nRet;
}

int main(int argc, char* argv[])
{
	int  ret = 0;

	if (argc == 5 && strcmp(argv[1],"-getadsstring") == 0)
	{
		ret = cutword_GetAdsString(argv[2],argv[3],argv[4]);
	}
	else if (argc == 4 && strcmp(argv[1],"-getnoadsstring") == 0)
	{
		ret = cutword_GetNoAdsString(argv[2],argv[3]);
	}
	else if (argc == 5 && strcmp(argv[1],"-rmsimstring") == 0)
	{
		ret = cutword_RemoveSimilarString(argv[2],argv[3],atoi(argv[4]));
	}
	else if (argc == 4 && strcmp(argv[1],"-cutword") == 0)
	{
		ret = cutword_cutword(argv[2],argv[3]);
	}
	else if (argc == 4 && strcmp(argv[1],"-countidf") == 0)
	{
		ret = cutword_countidf(argv[2],argv[3]);
	}
	else if (argc == 6 && strcmp(argv[1],"-pca") == 0)
	{
		ret = cutword_PCA_LearnModel(argv[2],argv[3],argv[4],argv[5]);
	}
	else if (argc == 6 && strcmp(argv[1],"-train") == 0)
	{
		ret = doc2vec_train(argv[2],argv[3],argv[4],atol(argv[5]));
	}
	else if (argc == 6 && strcmp(argv[1],"-predict") == 0)
	{
		ret = doc2vec_predict(argv[2],argv[3],argv[4],atol(argv[5]));
	}
	else
	{
		cout << "usage:\n" << endl;
		cout << "\tdemo -getadsstring List.txt UrlResult.txt AdsResult.txt\n" << endl;
		cout << "\tdemo -getnoadsstring List.txt Result.txt\n" << endl;
		cout << "\tdemo -rmsimstring List.txt Res.txt label\n" << endl;		
		cout << "\tdemo -cutword List.txt Res.txt\n" << endl;
		cout << "\tdemo -countidf List.txt Res.txt\n" << endl;
		cout << "\tdemo -pca List.txt Keyfile outPCAModel outPCAFeat\n" << endl;
		cout << "\tdemo -train List.txt Keyfile Feat binFeat\n" << endl;
		cout << "\tdemo -predict List.txt Keyfile Result binFeat\n" << endl;
		return ret;
	}
	return ret;
}

