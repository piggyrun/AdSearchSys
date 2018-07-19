#define _FILE_OFFSET_BITS 64

#include <stdio.h>
//#include <string.h>
#include <sys/time.h>
//#include <assert.h>

#include <iostream>
//#include <string>
#include <list>
//#include <ext/hash_map>

#include "AdSearchDef.h"

using namespace std; 

/////////////////////////////////////////
// gloabal hash maps
HashTestAccType hashTestAcc;

HashStringCntType hashQueryCnt;
HashIntCntType hashAdidCnt;
HashStringCntType hashRgCnt;
HashIntCntType hashPosCnt;
HashIntCntType hashAccidCnt;
HashStringCntType hashSrcCnt;
HashStringCntType hashSrcCnt_tmp; // 用于合并sogou-netb-xxxxx-xxxx这样的PID
HashStringCntType hashSrc4Cnt;
HashIntCntType hashCateCnt;
HashStringCntType hashAdkeyCnt;

int parse_string(const char * str, const char * split, char dest[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH], int * size)
{       
	//char stoken[2] = "\t";   
	char stoken[2];
	strcpy(stoken,split);
	char *p;
	int i = 0; 
	p = strtok((char*)str,stoken);
	if(p == NULL)
		return -1;
	while(p != NULL && i < MAX_KEYWORD_LENGTH - 1)  //最多读入MAX_KEYWORD_LEN个域，多余丢弃  
	{
		if(strlen(p) > MAX_STRING_LENGTH)           //如果一个域过长，则截断    
			p[MAX_STRING_LENGTH-1] = '\0';
		strcpy(dest[i++], p);
		p = strtok(NULL, stoken);
	}               
	if(i > 0)
	{
		*size = --i;
		return 0;
	}
	else
		return -1;
}

int load_testAcc(const char *file) // load test account list, and write to global variable hashTestAcc
{
	// read the test account list, and hash it
	FILE *testAccFile;
	// open test account file
	testAccFile = fopen(file,"r");
	if (testAccFile == NULL)
	{
		printf("Can't open test account file\n");
		return -1;
	}

	while (!feof(testAccFile))
	{
		int AccID;
		fscanf(testAccFile, "%d\n", &AccID);
		if (hashTestAcc.find(AccID) == hashTestAcc.end())
		{
			hashTestAcc[AccID] = 1;
		}
	}
	fclose(testAccFile);// finish test account hash
	
	std::cout<<"test account file load complete!\n";
	return 0;
}

// 边读边写，减少内存消耗
int load_ad(const char *file, const char * date)
{
	FILE *InFile;

	// open log file
	InFile = fopen(file,"r");
	if (InFile == NULL)
	{
		//printf("Can't open ad list file\n");
		return -1;
	}

	//int i; // loop variable
	/*/ clear all hash map
	hashID2AD.clear();
	for (i=0; i<FEATURE_DIM; i++)
	{
		hashFeatures[i].clear();
	}//*/

	// record structure: pv, click, ctr, query, adid, pos, accountid, ana_flag, ground_flag
	char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
	char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
	int size = 0;
	// hash map
	HashStringCntType::iterator itStringCnt;
	HashIntCntType::iterator itIntCnt;
	//HashStringType::iterator itString;
	//HashIntType::iterator itInt;

	// 第一轮读文件，确定各hash表大小
	int ii;

	ii = 0;
	printf("1st term scan to find index.\n");
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%50000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;

		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			printf("missing field\n%s\n", rawRecord);
			continue;
		}

		// query hash
		itStringCnt = hashQueryCnt.find(tokens[3]);
		if (itStringCnt != hashQueryCnt.end())
		{
			itStringCnt->second.len++;
		}
		else // new item need to added
		{
			hashQueryCnt[tokens[3]].len = 1;
		} 
		// adid hash
		unsigned int adid = atoi(tokens[4]);
		itIntCnt = hashAdidCnt.find(adid);
		if (itIntCnt != hashAdidCnt.end())
		{
			itIntCnt->second.len++;
		}
		else // new item need to added
		{
			hashAdidCnt[adid].len = 1;
		}
		// region hash
		itStringCnt = hashRgCnt.find(tokens[5]);
		if (itStringCnt != hashRgCnt.end())
		{
			itStringCnt->second.len++;
		}
		else // new item need to added
		{
			hashRgCnt[tokens[5]].len = 1;
		}
		// pos hash
		unsigned int pos = atoi(tokens[6]);
		/*/ debug
		if (pos > 32)
			printf("err position\n%s\t%s\t%s\t%s\n",
					tokens[0],tokens[1],tokens[2],tokens[3]);
		//*/
		itIntCnt = hashPosCnt.find(pos);
		if (itIntCnt != hashPosCnt.end())
		{
			itIntCnt->second.len++;
		}
		else // new item need to added
		{
			hashPosCnt[pos].len = 1;
		}
		// accid hash
		unsigned int accid = atoi(tokens[7]);
		itIntCnt = hashAccidCnt.find(accid);
		if (itIntCnt != hashAccidCnt.end())
		{
			itIntCnt->second.len++;
		}
		else // new item need to added
		{
			hashAccidCnt[accid].len = 1;
		}
		// src hash
		// sogou-*的聚合类作hash key
		string src = tokens[8];
		string src_mid;
		if (src.size() == 32)
		{
			src_mid = src.substr(0,27)+"-*"; // 将最后-xxxx的尾去掉，生成一个新的hash项
			itStringCnt = hashSrcCnt.find(src_mid);
			if (itStringCnt != hashSrcCnt.end())
			{
				itStringCnt->second.len++;
			}
			else // new item need to added
			{
				hashSrcCnt[src_mid].len = 1;
			}
		}
		if (src.size() == 27)// 先写到一个备用hash里，然后去与主hash对一下，这个大类下面有没有子pid，有的话，并入-*的项里
		{
			src_mid = src + "-*";
			itStringCnt = hashSrcCnt_tmp.find(src_mid);
			if (itStringCnt != hashSrcCnt_tmp.end())
			{
				itStringCnt->second.len++;
			}
			else
			{
				hashSrcCnt_tmp[src_mid].len = 1;
			}
		}
		// 全部的PID作hash key
		itStringCnt = hashSrcCnt.find(tokens[8]);
		if (itStringCnt != hashSrcCnt.end())
		{
			itStringCnt->second.len++;
		}
		else // new item need to added
		{
			hashSrcCnt[tokens[8]].len = 1;
		}

		// src4 hash
		itStringCnt = hashSrc4Cnt.find(tokens[9]);
		if (itStringCnt != hashSrc4Cnt.end())
		{
			itStringCnt->second.len++;
		}
		else // new item need to added
		{
			hashSrc4Cnt[tokens[9]].len = 1;
		}
		// cate hash
		unsigned int cate, cate3; // cate3是类别号的前三位
		if (string(tokens[10]) == "cate_not_found")
		{
			cate = 0;
			cate3 = 0;
		}
		else
		{
			cate = atoi(tokens[10]);
			if (cate<1000000) // 类别号一定有七位
				cate = 0;
			cate3 = cate/10000;
		}
		// 分类号前三位也做hash的key
		if (cate3 != 0)
		{
			itIntCnt = hashCateCnt.find(cate3);
			if (itIntCnt != hashCateCnt.end())
			{
				itIntCnt->second.len++;
			}
			else // new item need to added
			{
				hashCateCnt[cate3].len = 1;
			}
		}
		itIntCnt = hashCateCnt.find(cate);
		if (itIntCnt != hashCateCnt.end())
		{
			itIntCnt->second.len++;
		}
		else // new item need to added
		{
			hashCateCnt[cate].len = 1;
		}
		// adkey hash
		itStringCnt = hashAdkeyCnt.find(tokens[11]);
		if (itStringCnt != hashAdkeyCnt.end())
		{
			itStringCnt->second.len++;
		}
		else // new item need to added
		{
			hashAdkeyCnt[tokens[11]].len = 1;
		} //*/
	}
	// 并一下src的两个hash表
	for ( itStringCnt = hashSrcCnt_tmp.begin(); itStringCnt != hashSrcCnt_tmp.end(); ++itStringCnt)
	{
		if (hashSrcCnt.find(itStringCnt->first) != hashSrcCnt.end()) // 存在，要合并
		{
			hashSrcCnt[itStringCnt->first].len += itStringCnt->second.len;
		}
	}

	printf("1st scan finished, now building index files.\n");

	// 生成索引文件
	string IndexFileName;
	int start;
	FILE * pIndex;
	// query
	IndexFileName = string("log/") + date + string("/query.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create query index file\n";
		return -1;
	}
	start = 0;
	for (itStringCnt = hashQueryCnt.begin(); itStringCnt != hashQueryCnt.end(); ++itStringCnt)
	{
		itStringCnt->second.curr = 0;
		itStringCnt->second.start = start;
		start += itStringCnt->second.len;
		fprintf(pIndex, "%s\t%d\t%d\n", itStringCnt->first.c_str(), itStringCnt->second.start, itStringCnt->second.len);
	}
	fclose(pIndex);

	// adid
	IndexFileName = string("log/") + date + string("/adid.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create adid index file\n";
		return -1;
	}
	start = 0;
	for (itIntCnt = hashAdidCnt.begin(); itIntCnt != hashAdidCnt.end(); ++itIntCnt)
	{
		itIntCnt->second.curr = 0;
		itIntCnt->second.start = start;
		start += itIntCnt->second.len;
		fprintf(pIndex, "%d\t%d\t%d\n", itIntCnt->first, itIntCnt->second.start, itIntCnt->second.len);
	}
	fclose(pIndex);

	// region
	IndexFileName = string("log/") + date + string("/region.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create region index file\n";
		return -1;
	}
	start = 0;
	for (itStringCnt = hashRgCnt.begin(); itStringCnt != hashRgCnt.end(); ++itStringCnt)
	{
		itStringCnt->second.curr = 0;
		itStringCnt->second.start = start;
		start += itStringCnt->second.len;
		fprintf(pIndex, "%s\t%d\t%d\n", itStringCnt->first.c_str(), itStringCnt->second.start, itStringCnt->second.len);
	}
	fclose(pIndex);

	// pos
	IndexFileName = string("log/") + date + string("/pos.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create pos index file\n";
		return -1;
	}
	start = 0;
	for (itIntCnt = hashPosCnt.begin(); itIntCnt != hashPosCnt.end(); ++itIntCnt)
	{
		itIntCnt->second.curr = 0;
		itIntCnt->second.start = start;
		start += itIntCnt->second.len;
		fprintf(pIndex, "%d\t%d\t%d\n", itIntCnt->first, itIntCnt->second.start, itIntCnt->second.len);
	}
	fclose(pIndex);

	// acc id
	IndexFileName = string("log/") + date + string("/accid.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create acc id index file\n";
		return -1;
	}
	start = 0;
	for (itIntCnt = hashAccidCnt.begin(); itIntCnt != hashAccidCnt.end(); ++itIntCnt)
	{
		itIntCnt->second.curr = 0;
		itIntCnt->second.start = start;
		start += itIntCnt->second.len;
		fprintf(pIndex, "%d\t%d\t%d\n", itIntCnt->first, itIntCnt->second.start, itIntCnt->second.len);
	}
	fclose(pIndex);

	// src
	IndexFileName = string("log/") + date + string("/src.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create source index file\n";
		return -1;
	}
	start = 0;
	for (itStringCnt = hashSrcCnt.begin(); itStringCnt != hashSrcCnt.end(); ++itStringCnt)
	{
		itStringCnt->second.curr = 0;
		itStringCnt->second.start = start;
		start += itStringCnt->second.len;
		fprintf(pIndex, "%s\t%d\t%d\n", itStringCnt->first.c_str(), itStringCnt->second.start, itStringCnt->second.len);
	}
	fclose(pIndex);

	// src4
	IndexFileName = string("log/") + date + string("/src4.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create source (big) index file\n";
		return -1;
	}
	start = 0;
	for (itStringCnt = hashSrc4Cnt.begin(); itStringCnt != hashSrc4Cnt.end(); ++itStringCnt)
	{
		itStringCnt->second.curr = 0;
		itStringCnt->second.start = start;
		start += itStringCnt->second.len;
		fprintf(pIndex, "%s\t%d\t%d\n", itStringCnt->first.c_str(), itStringCnt->second.start, itStringCnt->second.len);
	}
	fclose(pIndex);

	// cate
	IndexFileName = string("log/") + date + string("/cate.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create category index file\n";
		return -1;
	}
	start = 0;
	for (itIntCnt = hashCateCnt.begin(); itIntCnt != hashCateCnt.end(); ++itIntCnt)
	{
		itIntCnt->second.curr = 0;
		itIntCnt->second.start = start;
		start += itIntCnt->second.len;
		fprintf(pIndex, "%d\t%d\t%d\n", itIntCnt->first, itIntCnt->second.start, itIntCnt->second.len);
	}
	fclose(pIndex);

	// ad key
	IndexFileName = string("log/") + date + string("/adkey.index");
	pIndex = fopen(IndexFileName.c_str(), "w");
	if (pIndex == NULL)
	{
		std::cout<<"can't create ad key index file\n";
		return -1;
	}
	start = 0;
	for (itStringCnt = hashAdkeyCnt.begin(); itStringCnt != hashAdkeyCnt.end(); ++itStringCnt)
	{
		itStringCnt->second.curr = 0;
		itStringCnt->second.start = start;
		start += itStringCnt->second.len;
		fprintf(pIndex, "%s\t%d\t%d\n", itStringCnt->first.c_str(), itStringCnt->second.start, itStringCnt->second.len);
	}
	fclose(pIndex);

	/* test
	FILE * tmpxx = fopen("count.txt", "w");
	for (itIntCnt = hashAccidCnt.begin(); itIntCnt != hashAccidCnt.end(); ++itIntCnt)
	{
		fprintf(tmpxx, "%d\t%d\n", itIntCnt->first, itIntCnt->second.len);
	}
	fclose(tmpxx);
	std::cout<<"count finished.\n";//*/


	printf("2nd scan starts, now writing invert list to hard drive.\n");

	// 第二轮在已知大小的情况下，边读边写，不占内存空间
	FILE *pData;
	string DataFileName;
	string names[FEATURE_DIM] = {
		"/query.dat",
		"/adid.dat",
		"/region.dat",
		"/pos.dat",
		"/accid.dat",
		"/src.dat",
		"/src4.dat",
		"/cate.dat",
		"/adkey.dat"
	};

	// 打开文件
	DataFileName = string("log/") + date + names[0];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//int fd = open(DataFileName.c_str(), O_WRONLY | O_CREAT);
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		//if (ii>150000)
		//	printf("read record\n");
		// 写入硬盘
		HashCnt cnt;
		// query
		itStringCnt = hashQueryCnt.find(record.query);
		cnt = itStringCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		//if (ii>150000)
		//	printf("seek finished\n");
		//lseek(fd, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		//fpos_t fpos = (cnt.start+cnt.curr)*sizeof(AdSearchRecord);
		//fsetpos(pData, &fpos);

		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		//write(fd, &record, sizeof(AdSearchRecord));
		itStringCnt->second.curr++;
	}
	fclose(pData);
	//close(fd);

	// 打开文件
	DataFileName = string("log/") + date + names[1];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		// 写入硬盘
		HashCnt cnt;
		// adid
		itIntCnt = hashAdidCnt.find(record.adid);
		cnt = itIntCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		itIntCnt->second.curr++;
	}
	fclose(pData);

	// 打开文件
	DataFileName = string("log/") + date + names[2];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		// 写入硬盘
		HashCnt cnt;
		// region
		itStringCnt = hashRgCnt.find(record.region);
		cnt = itStringCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		itStringCnt->second.curr++;
	}
	fclose(pData);

	// 打开文件
	DataFileName = string("log/") + date + names[3];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		// 写入硬盘
		HashCnt cnt;
		// pos
		itIntCnt = hashPosCnt.find(record.pos);
		cnt = itIntCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		itIntCnt->second.curr++;
	}
	fclose(pData);

	// 打开文件
	DataFileName = string("log/") + date + names[4];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		// 写入硬盘
		HashCnt cnt;
		// acc id
		itIntCnt = hashAccidCnt.find(record.accid);
		cnt = itIntCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		itIntCnt->second.curr++;
	}
	fclose(pData);

	// 打开文件
	DataFileName = string("log/") + date + names[5];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		// 判断src
		string src = tokens[8];
		string src_mid;
		if (src.size() == 32)
		{
			// 将最后-xxxx的尾去掉，生成一个新的hash项
			src_mid = src.substr(0,27);
			src_mid += "-*";
		}
		if (src.size() == 27)
		{
			src_mid = src+"-*";
		}
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		// 写入硬盘
		HashCnt cnt;
		// src
		itStringCnt = hashSrcCnt.find(record.src);
		cnt = itStringCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		itStringCnt->second.curr++;
		// 判断是否有合并项要写入硬盘src
		itStringCnt = hashSrcCnt.find(src_mid);
		if (itStringCnt != hashSrcCnt.end()) // 说明有一个合并项存在
		{
			cnt = itStringCnt->second;
			fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
			fwrite(&record, sizeof(AdSearchRecord), 1, pData);
			itStringCnt->second.curr++;
		}
	}
	fclose(pData);

	// 打开文件
	DataFileName = string("log/") + date + names[6];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		// 写入硬盘
		HashCnt cnt;
		// src4
		itStringCnt = hashSrc4Cnt.find(record.src4);
		cnt = itStringCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		itStringCnt->second.curr++;
	}
	fclose(pData);

	// 打开文件
	DataFileName = string("log/") + date + names[7];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
			if (record.cate < 1000000) // 不可考虑五位的类别号
			{
				record.cate = 0;
			}
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		// 写入硬盘
		HashCnt cnt;
		// cate
		itIntCnt = hashCateCnt.find(record.cate);
		cnt = itIntCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		itIntCnt->second.curr++;
		// 判断cate
		if (record.cate != 0)
		{
			// 将最后-xxxx的尾去掉，生成一个新的hash项
			itIntCnt = hashCateCnt.find(record.cate/10000);
			cnt = itIntCnt->second;
			fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
			fwrite(&record, sizeof(AdSearchRecord), 1, pData);
			itIntCnt->second.curr++;
		}
	}
	fclose(pData);

	// 打开文件
	DataFileName = string("log/") + date + names[8];
	pData = fopen(DataFileName.c_str(), "w");
	if (pData == NULL)
	{
		std::cout<<"can't create data file\n";
		return -1;
	}
	//printf("data file opened.\n");
	ii = 0;
	// 文件指针回到开始
	fseek(InFile, 0, SEEK_SET);
	printf("2nd sub-scan, now %s.\n", DataFileName.c_str());
	while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, InFile) != NULL )
	{
		if (ii%5000 == 0)
		{
			printf("curr finished num: %d\n", ii);
		}
		ii++;
		
		// parse the whole line into different fields
		parse_string(rawRecord, "\t", tokens, &size);
		// 有可能有空域，这样tokens会出现错位，要保证读的域个数符合要求
		if (size != 3+FEATURE_DIM)
		{
			//printf("missing field\n");
			continue;
		}

		AdSearchRecord record;
		strcpy(record.query, tokens[3]);
		record.adid = atoi(tokens[4]);
		strcpy(record.region, tokens[5]);
		record.pos = atoi(tokens[6]);
		record.accid = atoi(tokens[7]);
		strcpy(record.src, tokens[8]);
		strcpy(record.src4, tokens[9]);
		if (string(tokens[10]) == "cate_not_found")
		{
			record.cate = 0;
		}
		else
		{
			record.cate = atoi(tokens[10]);
		}
		strcpy(record.adkey, tokens[11]);
		
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
		{
			record.isTest = 1;
		}
		else
		{
			record.isTest = 0;
		}
		record.nPv = atoi(tokens[0]);
		record.nClick = atoi(tokens[1]);
		//record.ctr = atof(tokens[2]);
		if (string("na") == tokens[FEATURE_DIM+3])
		{
			record.price = -1;
		}
		else
		{
			record.price = atoi(tokens[FEATURE_DIM+3]);
		}
		// 写入硬盘
		HashCnt cnt;
		// ad key
		itStringCnt = hashAdkeyCnt.find(record.adkey);
		cnt = itStringCnt->second;
		fseek(pData, (cnt.start+cnt.curr)*sizeof(AdSearchRecord), SEEK_SET);
		fwrite(&record, sizeof(AdSearchRecord), 1, pData);
		itStringCnt->second.curr++;
	}
	fclose(pData);

	fclose(InFile);
	std::cout<<"ad file load complete!\n";
	
	return 0;
}

int main(int argc, char * argv[])
{
	//assert(argc>2);
	if (argc < 4)
	{
		std::cout<<"Not enough input arguments!\n";
		std::cout<<argv[0]<<" adlist_file testAcc_file date\n";
		return -1;
	}//*/

	// calculating the loading time
	struct timeval tvStart,tvEnd;
	double linStart = 0,linEnd = 0,lTime = 0;
	gettimeofday (&tvStart,NULL);

	// read the test account list, and hash it
	if (load_testAcc(argv[2])!=0)
	{
		printf("Can't open test account file\n");
		return -1;
	}

	// read records from log file
	if (load_ad(argv[1], argv[3])!=0)
	{
		std::cout<<"Can't load ad file\n";
		return -1;
	}

	// hash finished, giving the time 
	gettimeofday (&tvEnd,NULL);  
	linStart = ((double)tvStart.tv_sec * 1000000 + (double)tvStart.tv_usec);  //unit uS
	linEnd = ((double)tvEnd.tv_sec * 1000000 + (double)tvEnd.tv_usec);        //unit uS
	lTime = linEnd-linStart; 
	printf("Hash time is %fs\n", lTime/1e6);

	return 0;
}

