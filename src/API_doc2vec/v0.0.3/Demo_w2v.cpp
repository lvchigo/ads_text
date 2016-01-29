#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <boost/regex.hpp>	// regular expression

#include <vector>
#include <map>
#include <cv.h>

#include "API_doc2vec.h"
#include "Application.hpp"


using namespace CppJieba;
using namespace std;
using namespace cv;

const long long max_w = 4096;             // max length of vocabulary entries

/***********************************getRandomID***********************************/
void doc2vec_getRandomID( unsigned long long &randomID )
{
	int i,atom =0;
	randomID = 0;
	
	//time
	time_t nowtime = time(NULL);
	tm *now = localtime(&nowtime);

	char szTime[4096];
	sprintf(szTime, "%04d%02d%02d%02d%02d%02d%d",
			now->tm_year+1900, now->tm_mon+1, now->tm_mday,now->tm_hour, now->tm_min, now->tm_sec,(rand()%10000) );
	//printf("szTime Name:%s\n",szTime);

	string tmpID = szTime;
	for ( i=0;i<tmpID.size();i++)
	{
		atom = szTime[i] - '0';
		if (atom < 0 || atom >9)
			break;
		randomID = randomID * 10 + atom;
	}
}

/***********************************CountLines***********************************/
int doc2vec_CountLines(char *filename)
{
	ifstream ReadFile;
	int n=0;
	char line[4096];
	string temp;
	ReadFile.open(filename,ios::in);
	if(ReadFile.fail())
	{
	   return 0;
	}
	else
	{
		while(getline(ReadFile,temp))
		{
		   	n++;
		}
	}

	ReadFile.close();

	return n;
}


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

int cutword_GetNoAdsString( char *szQueryList, char *szResult, int binLabel )
{
	/*****************************Init*****************************/
	char loadPath[4096];
	int nRet = 0;
	long long count = 0;
	unsigned long long randomID;
	FILE *fpListFile = 0, *fpOut = 0;
	
	/*****************************Init*****************************/
	string docString;

	char tmpChar;
	char vocab[4096];
	string word;
	long long idQuary, labelQuary, i, a, b, w2v_words, w2v_size;

	vector< string > vecQuaryString;
	vector< string > quaryString;
	
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
	//Load Query
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
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

		if ( vecQuaryString.size() == 0 )
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
				
		count++;
		if ( count == 1 )
			continue;	
		if ( count % 1000 == 0 )
			printf( "load %d info...\n", count );

		doc2vec_getRandomID( randomID );

		/************************Save String*****************************/
		fprintf(fpOut, "%lld ", randomID );
		for (i = 0; i < vecQuaryString.size(); i++) {
			fprintf(fpOut, "%s ", vecQuaryString[i].c_str() );
		}
		fprintf(fpOut, "%lld\n", binLabel );

		if (b<4)
		{
			printf("vecQuaryString:");
			for ( a = 0; a < vecQuaryString.size(); a++ )
			{
				printf( "%s ", vecQuaryString[a].c_str() );
			}
			printf("\n");
		}
	}

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpOut) {fclose(fpOut);fpOut = 0;}
	
	cout<<"Done!! "<<endl;
	
	return nRet;
}

int cutword_ClearSourceData( char *szQueryList, char *szResult, int binString )
{
	/*****************************Init*****************************/
	char loadPath[4096];
	char idChar[4096];
	char labelChar[4096];
	int nRet = 0;
	FILE *fpListFile = 0, *fpFeatOut = 0;

	char tmpChar;
	char vocab[4096];
	string word,finalWord;
	long long idQuary, labelQuary, i, a, b, w2v_words, w2v_size, finalWordLength;

	vector< string > vecQuaryString;
	vector< string > quaryString;
	
	/*****************************Init*****************************/
	string docString,queryString,id,label;
	map< vector< string >, int > docCount;
	map< vector< string >, int >::iterator itDocCount;

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
	/*****************************Process one by one*****************************/
	//Load Query
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
		//load word to vector
		a = 0;
		vecQuaryString.clear();
		while (1) {
			vocab[a] = fgetc(fpListFile);	
			tmpChar = vocab[a];
			if ((a < max_w) && (vocab[a] != '\n')) a++;
			if ( feof(fpListFile) || (vocab[a] == '\n') )
			{
				vocab[a] = 0;
				word = vocab;
				finalWordLength = word.size();

				while (1) {
					if( word[finalWordLength-1] == ',' )
					{
						finalWord = word.substr(0, finalWordLength-1);
						finalWordLength = finalWord.size();
					}
					else
						break;
				}

				if (vocab[0] != '\n')
					vecQuaryString.push_back( finalWord );

				a = 0;
			}
			if (feof(fpListFile) || (tmpChar == '\n')) break;
		}

		if ( vecQuaryString.size() == 0 )
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
		if ( binString == 1 )	//id string label
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

		//find
		itDocCount = docCount.find(quaryString);
		if (itDocCount == docCount.end()) // not find
		{
			docCount[quaryString] = 1;
			
			/************************Save String*****************************/
			if ( binString == 1 )
				fprintf(fpFeatOut, "%lld ", idQuary );
			for (i = 0; i < quaryString.size(); i++) {
				fprintf(fpFeatOut, "%s ", quaryString[i].c_str() );
			}
			if ( binString == 1 )
				fprintf(fpFeatOut, "%lld ", labelQuary );
			fprintf(fpFeatOut, "\n" );
		}

		if ( docCount.size() % 1000 == 0 )
			printf( "load %d info...\n", docCount.size() );
	}
	
	printf("sumWord:%ld\n",docCount.size());

	docCount.clear();

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}
	
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
	FILE *fpListFile = 0, *fpFeatOut = 0;

	char tmpChar;
	char vocab[4096];
	string word;
	long long idQuary, labelQuary, i, a, b, w2v_words, w2v_size;

	vector< string > vecQuaryString;
	vector< string > quaryString;
	
	/*****************************Init*****************************/
	string docString,queryString,id,label;
	map< vector< string >, int > docCount;
	map< vector< string >, int >::iterator itDocCount;

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
	/*****************************Process one by one*****************************/
	//Load Query
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
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

		if ( vecQuaryString.size() == 0 )
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
		if ( binString == 1 )	//id string label
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

		//find
		itDocCount = docCount.find(quaryString);
		if (itDocCount == docCount.end()) // not find
		{
			docCount[quaryString] = 1;
			
			/************************Save String*****************************/
			if ( binString == 1 )
				fprintf(fpFeatOut, "%lld ", idQuary );
			for (i = 0; i < quaryString.size(); i++) {
				fprintf(fpFeatOut, "%s ", quaryString[i].c_str() );
			}
			if ( binString == 1 )
				fprintf(fpFeatOut, "%lld ", labelQuary );
			fprintf(fpFeatOut, "\n" );
		}

		if ( docCount.size() % 1000 == 0 )
			printf( "load %d info...\n", docCount.size() );
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
	int nRet = 0;
	long nCount;
	FILE *fpListFile = 0, *fpStopWord = 0, *fpFeatOut = 0;

	API_DOC2VEC api_doc2vec;

	char tmpChar;
	char vocab[4096];
	string word;
	long long idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;

	vector< string > vecQuaryString;
	vector< string > quaryString;

	map< vector< string >, int > docCount;
	map< vector< string >, int >::iterator itDocCount;
	
	/*****************************Init*****************************/
	vector<string> words;
	vector<string> MergeWords;
	vector<string> outputWords;
	map< string,int > stopwords;
	map< string,int >::iterator itStopwords;
	string s;
	string result;

	/*****************************Init:CutWord*****************************/
	CppJieba::Application app(
		"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/jieba.dict.utf8",	//35w
		"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/hmm_model.utf8",
//		"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/dict.367W.utf8",	//367w
		"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/user.dict.utf8",
		"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/idf.utf8",
		"/home/chigo/working/word2vec/word2vec/keyfile/cutword-geiba/stop_words.utf8");

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
	/*****************************Process one by one*****************************/
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Cut Words!!Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
		//load word to vector
		a = 0;
		vecQuaryString.clear();
		while (1) {
			vocab[a] = fgetc(fpListFile);	
			tmpChar = vocab[a];
			if ((a < max_w) && (vocab[a] != ' ') && (vocab[a] != '?') && (vocab[a] != ';') && (vocab[a] != '\n')) a++;
			if ( ( vocab[a] == ' ' ) || (vocab[a] == '?') || (vocab[a] == ';') || feof(fpListFile) || (vocab[a] == '\n') )
			{
				vocab[a] = 0;
				word = vocab;

				if ( ( vocab[0] != ' ' ) && (vocab[a] != '?') && (vocab[a] != ';') && (vocab[0] != '\n') )
					vecQuaryString.push_back( word );

				a = 0;
			}
			if (feof(fpListFile) || (tmpChar == '\n')) break;
		}

		if ( vecQuaryString.empty() )
		{
			printf( "vecQuaryString.size():%ld, do not find info!!b:%lld\n", vecQuaryString.size(), b );
			continue;
		}

		//Merge Words
		MergeWords.clear();
		for (i = 0; i < vecQuaryString.size(); i++) 
		{
			/************************Cut Word*****************************/
			words.clear();
			app.cut(vecQuaryString[i], words, METHOD_MIX);
			MergeWords.insert( MergeWords.end(), words.begin(), words.end() ) ;
		}

		outputWords.clear();
		/************************judge word*****************************/
		for ( i=0;i<MergeWords.size();i++ )
		{
			//remove digit || illegal word
			if ((MergeWords[i].empty()) || (MergeWords[i] == " ") || (MergeWords[i] == "	") || 
				(api_doc2vec.IsDigit2(MergeWords[i])) || (MergeWords[i].size()<6) || (!api_doc2vec.IsAllChinese(MergeWords[i])) )
				continue;

			//remove stopword
			itStopwords = stopwords.find(MergeWords[i]);
			if (itStopwords == stopwords.end()) // not find
			{
				outputWords.push_back( MergeWords[i] );
			}
		}	

		if ( outputWords.size() == 0 )
			continue;

		nCount++;

/*		//Save String
		for ( i=0;i<outputWords.size();i++ )
		{
			fprintf(fpFeatOut, "%s ", outputWords[i].c_str() );
		}
		fprintf(fpFeatOut, "\n");*/

		//remove same doc
		itDocCount = docCount.find(outputWords);
		if (itDocCount == docCount.end()) // not find
		{
			docCount[outputWords] = 1;
			
			//Save String
			for ( i=0;i<outputWords.size();i++ )
			{
				fprintf(fpFeatOut, "%s ", outputWords[i].c_str() );
			}
			fprintf(fpFeatOut, "\n");
		}

		/************************Check*****************************/
		if ( nCount % 1000 == 0 )
		{
			printf("Loaded %ld info...\n",nCount);
			for ( i=0;i<outputWords.size();i++ )
			{
				printf("%s ", outputWords[i].c_str() );
			}
			printf("\n");
		}
	}
	printf("sumWord:%ld\n",nCount);

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
	int nRet = 0;
	long long count = 0;
	FILE *fpListFile = 0, *fpFeatOut = 0;

	API_DOC2VEC api_doc2vec;

	char tmpChar;
	char vocab[4096];
	string word;
	long long idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;

//	vector< string > vecQuaryString;
	map< string, int > mapQuaryString;
	map< string, int >::iterator itQuaryString;

	vector< string > quaryString;
	
	/*****************************Init*****************************/
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
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Cut Words!!Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
		//load word to vector
		a = 0;
		mapQuaryString.clear();
		while (1) {
			vocab[a] = fgetc(fpListFile);	
			tmpChar = vocab[a];
			if ((a < max_w) && (vocab[a] != ' ') && (vocab[a] != '\n')) a++;
			if ( ( vocab[a] == ' ' ) || feof(fpListFile) || (vocab[a] == '\n') )
			{
				vocab[a] = 0;
				word = vocab;

				if ( ( vocab[0] != ' ' ) && (vocab[0] != '\n') )
				{
					itQuaryString = mapQuaryString.find(word);
					if (itQuaryString == mapQuaryString.end()) // not find
					{
						mapQuaryString[word] = 1;
					}
					else
					{
						mapQuaryString[word]++;
					}
				}

				a = 0;
			}
			if (feof(fpListFile) || (tmpChar == '\n')) break;
		}

		if ( mapQuaryString.empty() )
		{
			printf( "mapQuaryString.size():%ld, do not find info!!b:%lld\n", mapQuaryString.size(), b );
			continue;
		}

		//every word just add 1/0 in one text
		for(itQuaryString = mapQuaryString.begin(); itQuaryString != mapQuaryString.end(); itQuaryString++)
		{
			word = itQuaryString->first;
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

		/************************Check*****************************/
		if ( wordCount.size() % 1000 == 0 )
		{
			printf( "Get %ld Word idf info...\n", wordCount.size() );
		}
	}

	/************************Count Idf*****************************/
	for(itWordCount = wordCount.begin(); itWordCount != wordCount.end(); itWordCount++)
	{
		word = itWordCount->first;
		idf = log10( wordCount.size()*1.0/(1+itWordCount->second) );

		//remove digit || illegal word
		if ( (word.empty()) || (word == " ") || (word == "	") || 
			(api_doc2vec.IsDigit2(word)) || (word.size()<6) || (!api_doc2vec.IsAllChinese(word)) )
			continue;

		/************************Save Idf*****************************/
		fprintf(fpFeatOut, "%s %f\n", word.c_str(), idf );
	}
	printf("sumWord:%ld\n",wordCount.size());

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
	char vocab[4096];
	string word;
	long QuerySize[12] = {0};
	long long QueryAllSize, idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;
	FILE *fpListFile = 0, *fpFeatOut = 0;

	API_DOC2VEC api_doc2vec;
	
	/*****************************Init:Doc2Vec*****************************/
	vector< string > vecQuaryString;
	vector< string > quaryString;
	map< string, vector< float > > 							mapQuaryWord;	//[In]quary word
	map< vector< string >, vector< float > > 				mapQuaryDoc;	//[Out]quary Doc
	map< vector< string >, vector< float > >::iterator 		itQuaryDoc;		//[In]dic word

	map< vector< string >, int > wordCount;
	map< vector< string >, int >::iterator itWordCount;
	
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
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
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
		QueryAllSize = 0;
		quaryString.clear();
		idQuary = atol( vecQuaryString[0].c_str() );	//id
		labelQuary = atol( vecQuaryString[vecQuaryString.size()-1].c_str() ); //label
		for ( a = 1; a < vecQuaryString.size()-1; a++ )
		{
			quaryString.push_back(vecQuaryString[a]);
			QueryAllSize += vecQuaryString[a].size();
		}

		if ( labelQuary == 1)
		{
			if ( QueryAllSize < 15 )
				QuerySize[0]++;
			else if ( QueryAllSize < 30 )
				QuerySize[1]++;
			else if ( QueryAllSize < 45 )
				QuerySize[2]++;
			else if ( QueryAllSize < 60 )
				QuerySize[3]++;
			else if ( QueryAllSize < 90 )
				QuerySize[4]++;
			else
				QuerySize[5]++;
		}
		else
		{
			if ( QueryAllSize < 15 )
				QuerySize[6]++;
			else if ( QueryAllSize < 30 )
				QuerySize[7]++;
			else if ( QueryAllSize < 45 )
				QuerySize[8]++;
			else if ( QueryAllSize < 60 )
				QuerySize[9]++;
			else if ( QueryAllSize < 90 )
				QuerySize[10]++;
			else
				QuerySize[11]++;
		}

/*		if ( ( QueryAllSize < 45 ) && ( labelQuary == 1) )	//query string.size>=45
		{
			//printf( "QueryAllSize:%ld<45\n", QueryAllSize );
			continue;
		}*/

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
			itWordCount = wordCount.find(itQuaryDoc->first);
			if (itWordCount == wordCount.end()) // not find
			{
				wordCount[itQuaryDoc->first] = 1;

				if ( itQuaryDoc->first.size() > 1 )	//query useful string>1
				{
					/************************Save GetFeat*****************************/
					fprintf(fpFeatOut, "%ld ", labelQuary );
					for ( i=0;i<itQuaryDoc->second.size();i++ )
					{
						fprintf(fpFeatOut, "%ld:%.6f ", i+1, (itQuaryDoc->second[i]+0.00000001) );
					}	
					fprintf(fpFeatOut, "\n");
				}
			}	
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
	printf("All Load %d strings!!\n",wordCount.size());

	printf("QuerySize:");
	for(i=0;i<12;i++)
		printf("%ld ", QuerySize[i]);
	printf("\n");

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}	
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}
	
	api_doc2vec.Release();

	cout<<"Done!! "<<endl;
	
	return nRet;
}

int doc2vec_predict( char *szQueryList, char *Keyfile, char *szResult, char *szResult2, int binFeat )
{
	/*****************************Init:Commen*****************************/
	char loadPath[4096];
	int nRet = 0;
	char tmpChar;
	char vocab[4096];
	string word;
	double allPredictTime,tPredictTime;
	long long QueryAllSize, idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;
	FILE *fpListFile = 0, *fpFeatOut = 0, *fpFeatOut2 = 0;

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

	fpFeatOut2 = fopen(szResult2, "wt");
	if (!fpFeatOut2)
	{
		cout << "Can't open result file " << szResult2 << endl;
		return -1;
	}

	api_doc2vec.Init( Keyfile );
	tPredictTime = 0.0;
	allPredictTime = 0.0;

	/*****************************Load Query*****************************/
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
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
		
		if ( vecQuaryString.size() == 0 )
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
		QueryAllSize = 0;
		quaryString.clear();
		if ( binFeat == 1 )	//id string label
		{
			idQuary = atol( vecQuaryString[0].c_str() );	//id
			labelQuary = atol( vecQuaryString[vecQuaryString.size()-1].c_str() ); //label
			for ( a = 1; a < vecQuaryString.size()-1; a++ )
			{
				quaryString.push_back(vecQuaryString[a]);
				//QueryAllSize += vecQuaryString[a].size();
			}
		}
		else	//string
		{
			for ( a = 0; a < vecQuaryString.size(); a++ )
			{
				quaryString.push_back(vecQuaryString[a]);
				QueryAllSize += vecQuaryString[a].size();
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
		for(itPredict = resPredict.begin(); itPredict != resPredict.end(); itPredict++)
		{
			//if ( itPredict->first.size()>1 )
			if ( ( itPredict->first.size()>1 ) && ( ( itPredict->second[0].second >=0.99995 ) || ( QueryAllSize>=45 ) ) )
			{
				fprintf(fpFeatOut, "label:%d,score:%.4f,str:%ld,size:%ld ", 
					itPredict->second[0].first, itPredict->second[0].second, itPredict->first.size(), QueryAllSize );
				for(i=0;i<quaryString.size();i++)
					fprintf( fpFeatOut, "%s ", quaryString[i].c_str() );
				fprintf(fpFeatOut, "'");
				for ( i=0;i<itPredict->first.size();i++ )
				{
					fprintf(fpFeatOut, "%s ", itPredict->first[i].c_str() );
				}
				fprintf(fpFeatOut, "'\n");
			}
			else
			{
				fprintf(fpFeatOut2, "label:%d,score:%.4f,str:%ld,size:%ld ", 
					itPredict->second[0].first, itPredict->second[0].second, itPredict->first.size(), QueryAllSize );
				for(i=0;i<quaryString.size();i++)
					fprintf( fpFeatOut2, "%s ", quaryString[i].c_str() );
				fprintf(fpFeatOut2, "'");
				for ( i=0;i<itPredict->first.size();i++ )
				{
					fprintf(fpFeatOut2, "%s ", itPredict->first[i].c_str() );
				}
				fprintf(fpFeatOut2, "'\n");
			}
		}

		//check data 
		if (b<4)
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
	if (fpFeatOut2) {fclose(fpFeatOut2);fpFeatOut2 = 0;}

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

int doc2vec_addsample( char *szQueryList, char *Keyfile, char *szpos, char *szneg, int binFeat )
{
	/*****************************Init:Commen*****************************/
	char loadPath[4096];
	int nRet = 0;
	char tmpChar;
	char vocab[4096];
	string word;
	double allPredictTime,tPredictTime;
	long long idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;
	FILE *fpListFile = 0, *fpFeatOut = 0, *fpFeatOutNeg = 0;

	API_DOC2VEC api_doc2vec;
	vector< string > vecQuaryString;
	vector< string > quaryString;
	map< vector< string >, vector< pair< int, float > > > 			resPredict;
	map< vector< string >, vector< pair< int, float > > >::iterator itPredict;

	map< vector< string >, int > wordCount;
	map< vector< string >, int >::iterator itWordCount;
	
	/********************************Load:Open Query List*****************************/
	fpListFile = fopen(szQueryList,"r");
	if (!fpListFile) 
	{
		cout << "Can't open " << szQueryList << endl;
		return -1;
	}

	fpFeatOut = fopen(szpos, "wt");
	if (!fpFeatOut)
	{
		cout << "Can't open result file " << szpos << endl;
		return -1;
	}

	fpFeatOutNeg = fopen(szneg, "wt");
	if (!fpFeatOutNeg)
	{
		cout << "Can't open result file " << szneg << endl;
		return -1;
	}

	api_doc2vec.Init( Keyfile );
	tPredictTime = 0.0;
	allPredictTime = 0.0;

	/*****************************Load Query*****************************/
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
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
		
		if ( vecQuaryString.size() == 0 )
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
		for(itPredict = resPredict.begin(); itPredict != resPredict.end(); itPredict++)
		{
			if ( (itPredict->second[0].first == 0) && (itPredict->second[0].second >= 0.9) && (itPredict->first.size()>=2) )
			{
				itWordCount = wordCount.find(itPredict->first);
				if (itWordCount == wordCount.end()) // not find
				{
					wordCount[itPredict->first] = 1;

					/************************Save GetFeat*****************************/
					for ( i=0;i<quaryString.size();i++ )
					{
						fprintf(fpFeatOutNeg, "%s ", quaryString[i].c_str() );
					}
					fprintf(fpFeatOutNeg, "\n");
				}	
			}
			else if ( (itPredict->second[0].first == 1) && (itPredict->second[0].second >= 0.9) && (itPredict->first.size()>=2) )
			{
				itWordCount = wordCount.find(itPredict->first);
				if (itWordCount == wordCount.end()) // not find
				{
					wordCount[itPredict->first] = 1;

					/************************Save GetFeat*****************************/
					for ( i=0;i<quaryString.size();i++ )
					{
						fprintf(fpFeatOut, "%s ", quaryString[i].c_str() );
					}
					fprintf(fpFeatOut, "\n");
				}
			}
		}

		//check data 
		if (b<4)
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
	if (fpFeatOutNeg) {fclose(fpFeatOutNeg);fpFeatOutNeg = 0;}

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

int doc2vec_findsimword( char *szQueryList, char *Keyfile, char *szResult, char *szWordResult, int binFeat )
{	
	/*****************************Init:Commen*****************************/
	char loadPath[4096];
	int nRet = 0;
	char tmpChar;
	char vocab[4096];
	string word;
	double allPredictTime,tPredictTime;
	long long QueryAllSize, idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size;
	FILE *fpListFile = 0, *fpFeatOut = 0, *fpWordOut = 0;

	API_DOC2VEC api_doc2vec;
	vector< string > vecQuaryString;
	vector< string > quaryString;
	map< string , vector< pair< string, float > > > 	resSimWord;
	map< string , vector< pair< string, float > > >::iterator 	itSimWord;

	map< string, int > mapKeyWord;
	map< string, int >::iterator itKeyWord;

	map< string, int > mapResWord;
	map< string, int >::iterator itResWord;
	
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

	fpWordOut = fopen(szWordResult, "wt");
	if (!fpWordOut)
	{
		cout << "Can't open word result file " << szWordResult << endl;
		return -1;
	}

	api_doc2vec.Init( Keyfile );
	tPredictTime = 0.0;
	allPredictTime = 0.0;

	/*****************************Load Query*****************************/
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
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
		
		if ( vecQuaryString.size() == 0 )
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
		QueryAllSize = 0;
		quaryString.clear();
		if ( binFeat == 1 )	//id string label
		{
			idQuary = atol( vecQuaryString[0].c_str() );	//id
			labelQuary = atol( vecQuaryString[vecQuaryString.size()-1].c_str() ); //label
			for ( a = 1; a < vecQuaryString.size()-1; a++ )
			{
				quaryString.push_back(vecQuaryString[a]);
				//QueryAllSize += vecQuaryString[a].size();
			}
		}
		else	//string
		{
			for ( a = 0; a < vecQuaryString.size(); a++ )
			{
				quaryString.push_back(vecQuaryString[a]);
				QueryAllSize += vecQuaryString[a].size();
			}
		}

		if ( quaryString.size() == 0 )
		{
			continue;
		}

		//write keyword
		for(i=0;i<quaryString.size();i++)
		{
			itKeyWord = mapKeyWord.find(quaryString[i]);
			if (itKeyWord == mapKeyWord.end()) // no find
				mapKeyWord[quaryString[i]] = 1;
		}

		/***********************************FindSimilarWord*************************************/
		resSimWord.clear();
		nRet = api_doc2vec.FindSimilarWord( quaryString,resSimWord);	//[Out]Res:string-useful words,int-label,float-score
		//nRet = api_doc2vec.FindSimilarWord_HammingCode( quaryString,resSimWord);	//[Out]Res:string-useful words,int-label,float-score
		if ( ( nRet != 0 ) || ( resSimWord.size() == 0 ) )
		{
			printf("doc2vec_predict:FindSimilarWord err!!\n");
			continue;
		}

		//save docfeat
		/************************Save GetFeat*****************************/	
		fprintf( fpFeatOut, "input:");
		for(i=0;i<quaryString.size();i++)
			fprintf( fpFeatOut, "%s ", quaryString[i].c_str() );
		fprintf( fpFeatOut, "\n");
		for(itSimWord = resSimWord.begin(); itSimWord != resSimWord.end(); itSimWord++)
		{
			fprintf(fpFeatOut, "%s '",itSimWord->first.c_str() );
			for ( i=0;i<itSimWord->second.size();i++ )
			{
				fprintf(fpFeatOut, "%s %.4f ", itSimWord->second[i].first.c_str(),itSimWord->second[i].second );

				//save resWord
				itKeyWord = mapKeyWord.find(itSimWord->second[i].first);
				itResWord = mapResWord.find(itSimWord->second[i].first);
				if ( (itKeyWord == mapKeyWord.end()) && (itResWord == mapResWord.end()) && (itSimWord->second[i].second>=0.75) )
				{
					mapResWord[itSimWord->second[i].first] = 1;
				}
				
			}
			fprintf(fpFeatOut, "'\n");
		}
		fprintf(fpFeatOut, "\n");	

		//printf
		if( b%50 == 0 )
			printf("Loaded %ld DocInfo...\n",b);
	}

	//write resWord
	for(itResWord = mapResWord.begin(); itResWord != mapResWord.end(); itResWord++)
	{	
		fprintf( fpWordOut, "%s\n", itResWord->first.c_str() );
	}

	/*********************************close file*************************************/
	if (fpListFile) {fclose(fpListFile);fpListFile = 0;}
	if (fpFeatOut) {fclose(fpFeatOut);fpFeatOut = 0;}
	if (fpWordOut) {fclose(fpWordOut);fpWordOut = 0;}
	
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


int doc2vec_filterString(  char *szQueryList, char *szResult )
{
	/*****************************Init:Commen*****************************/
	char loadPath[4096];
	int nRet = 0;
	char tmpChar;
	char vocab[4096];
	string word;
	double allPredictTime,tPredictTime;
	long long QueryAllSize, idQuary, labelQuary, i, j, a, b, w2v_words, w2v_size, wordCount;
	FILE *fpListFile = 0, *fpFeatOut = 0;

	API_DOC2VEC api_doc2vec;
	vector< string > vecQuaryString;
	vector< string > quaryString;
	vector< string > vecFilterWord;
	
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

	tPredictTime = 0.0;
	allPredictTime = 0.0;

	/*****************************Load Query*****************************/
	//fscanf(fpListFile, "%lld", &w2v_words);
	w2v_words = doc2vec_CountLines(szQueryList);
	printf("Load Query.txt...w2v_words-%lld\n",w2v_words);
	for (b = 0; b < w2v_words; b++) {
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
		
		if ( vecQuaryString.size() == 0 )
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
		QueryAllSize = 0;
		quaryString.clear();
		for ( a = 0; a < vecQuaryString.size(); a++ )
		{
			quaryString.push_back(vecQuaryString[a]);
			QueryAllSize += vecQuaryString[a].size();
		}

		if ( quaryString.size() == 0 )
		{
			continue;
		}
		
		//FilterQuaryString
		vecFilterWord.clear();
		tPredictTime = (double)getTickCount();
		nRet = api_doc2vec.FilterQuaryString( quaryString, vecFilterWord );
 		tPredictTime = (double)getTickCount() - tPredictTime;
		tPredictTime = tPredictTime*1000./cv::getTickFrequency();
		allPredictTime += tPredictTime;
		if ( ( nRet != 0 ) || ( vecFilterWord.size() == 0 ) )
		{
			printf("doc2vec_filterString:FilterQuaryString err!!\n");
			continue;
		}

		//save docfeat
		/************************Save GetFeat*****************************/	
		for(i=0;i<quaryString.size();i++)
			fprintf( fpFeatOut, "%s ", quaryString[i].c_str() );
		fprintf(fpFeatOut, "'");
		for(i=0;i<vecFilterWord.size();i++)
		{
			fprintf(fpFeatOut, "%s ", vecFilterWord[i].c_str() );
		}
		fprintf(fpFeatOut, "'\n");

		//printf
		if( b%1000 == 0 )
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

int test()
{
	
}


int main(int argc, char* argv[])
{
	int  ret = 0;

	if (argc == 5 && strcmp(argv[1],"-getadsstring") == 0)
	{
		ret = cutword_GetAdsString(argv[2],argv[3],argv[4]);
	}
	else if (argc == 5 && strcmp(argv[1],"-getnoadsstring") == 0)
	{
		ret = cutword_GetNoAdsString(argv[2],argv[3],atoi(argv[4]));
	}
	else if (argc == 5 && strcmp(argv[1],"-clearsourcedata") == 0)
	{
		ret = cutword_ClearSourceData(argv[2],argv[3],atoi(argv[4]));
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
	else if (argc == 6 && strcmp(argv[1],"-train") == 0)
	{
		ret = doc2vec_train(argv[2],argv[3],argv[4],atol(argv[5]));
	}
	else if (argc == 7 && strcmp(argv[1],"-predict") == 0)
	{
		ret = doc2vec_predict(argv[2],argv[3],argv[4],argv[5],atol(argv[6]));
	}
	else if (argc == 7 && strcmp(argv[1],"-addsample") == 0)
	{
		ret = doc2vec_addsample(argv[2],argv[3],argv[4],argv[5],atol(argv[6]));
	}
	else if (argc == 7 && strcmp(argv[1],"-findsimword") == 0)
	{
		ret = doc2vec_findsimword(argv[2],argv[3],argv[4],argv[5],atol(argv[6]));
	}
	else if (argc == 4 && strcmp(argv[1],"-filterstring") == 0)
	{
		ret = doc2vec_filterString(argv[2],argv[3]);
	}
	else if (argc == 2 && strcmp(argv[1],"-test") == 0)
	{
		ret = test();
	}
	else
	{
		cout << "usage:\n" << endl;
		cout << "\tdemo -getadsstring List.txt UrlResult.txt AdsResult.txt\n" << endl;
		cout << "\tdemo -getnoadsstring List.txt Result.txt binLabel\n" << endl;
		cout << "\tdemo -clearsourcedata List.txt Res.txt label\n" << endl;		
		cout << "\tdemo -rmsimstring List.txt Res.txt label\n" << endl;		
		cout << "\tdemo -cutword List.txt Res.txt\n" << endl;
		cout << "\tdemo -countidf List.txt Res.txt\n" << endl;
		cout << "\tdemo -train List.txt Keyfile Feat binFeat\n" << endl;
		cout << "\tdemo -predict List.txt Keyfile Result Result2 binFeat\n" << endl;
		cout << "\tdemo -addsample List.txt Keyfile pos neg binFeat\n" << endl;
		cout << "\tdemo -findsimword List.txt Keyfile Result simword binFeat\n" << endl;
		cout << "\tdemo -filterstring List.txt Result\n" << endl;
		cout << "\tdemo -test\n" << endl;
		return ret;
	}
	return ret;
}

