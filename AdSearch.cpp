#include <sys/time.h>

#include <iostream>
#include <iomanip>
//#include <string>
#include <list>
#include <vector>
//#include <ext/hash_map>
//#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "AdSearchDef.h"
#include "Platform/md5/md5.h"

using namespace std; 
//using namespace __gnu_cxx;

/////////////////////////////////////////
// global hash maps
//HashStringIndexType hashQueryIndex;
//HashStringIndexListType hashQueryIndex;
HashMD5IndexListType hashQueryIndex;
HashIntIndexType hashAdidIndex;
HashStringIndexType hashRgIndex;
HashIntIndexType hashPosIndex;
HashIntIndexType hashAccidIndex;
HashStringIndexType hashSrcIndex;
//HashStringIndexType hashSrc4Index;
HashIntIndexType hashCateIndex;
HashStringIndexType hashAdkeyIndex;

// cheat pid
t_string_map cheatpid_map;

/*
int ad_search(const string SearchFeatures[FEATURE_DIM], const std::vector<int> &dispFeatures, bool isTestAcc, int sortCrt, std::vector<AdDisplay> &DispList);
void search_display(const std::vector<AdDisplay> &DispList);
int list_intersection(const std::list<int> * list1, const std::list<int> * list2, std::list<int> &interlist); // list intersection function
int parse_string(const char * str, const char * split, char dest[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH], int * size);
int vector_TopHeapSort(std::vector<struct AdDisplay> &arr, int top, int criteria); // find the top ad info using pv/clk/ctr/price
bool AdDisplayLess(const struct AdDisplay &a, const struct AdDisplay &b, int criteria); // compare the structure using pv/clk/ctr/price, a<b: true; else: false
void * socket_sendrecv(void * sock); // new thread function to process the client connection
*/

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
	while(p != NULL && i < MAX_KEYWORD_LENGTH - 1)  //������MAX_KEYWORD_LEN���򣬶��ඪ��  
	{
		if(strlen(p) > MAX_STRING_LENGTH)           //���һ�����������ض�    
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

unsigned int calc_diff090101(unsigned int year, unsigned int month, unsigned int day)
{
	unsigned int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	unsigned int mdays_ly[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
	unsigned int num = 0;
	if (year < 2009) // error
	{
		return 0;
	}
	else if (year == 2009)
	{
		num = day;
		for (int i=month-1; i>0; i--)
		{
			num += mdays[i-1];
		}
	}
	else
	{
		for (unsigned int i=2009; i<year; i++)
		{
			if ((i%400 ==0) || (i%100!=0 && i%4 ==0))
			{
				num += 366;
			}
			else
			{
				num += 365;
			}
		}
		num += day;
		for (unsigned int i=month-1; i>0; i--)
		{       
			if ((year%400 ==0) || (year%100!=0 && year%4 ==0))
			{    
				num += mdays_ly[i-1];
			}    
			else 
			{    
				num += mdays[i-1]; 
			}    
		}
	}
	return num;
}

// ��������֮����������
// ��һ���������ڵڶ���������
int calc_diffdate(const string &date1, const string &date2)
{
	unsigned int year, month, day, num1, num2; 
	string sy, sm, sd;

	// ��2009��1��1��Ϊ��׼
	sy = date1.substr(0,4);
	year = atoi(sy.c_str());
	sm = date1.substr(4,2);
	month = atoi(sm.c_str());
	sd = date1.substr(6,2);
	day = atoi(sd.c_str());

	num1 = calc_diff090101(year, month, day);

	sy = date2.substr(0,4);
	year = atoi(sy.c_str());
	sm = date2.substr(4,2);
	month = atoi(sm.c_str());
	sd = date2.substr(6,2);
	day = atoi(sd.c_str());

	num2 = calc_diff090101(year, month, day);

	return num1-num2;
}

// ��������ƫ�ƣ��ӵ�ǰ������ǰ��
int calc_yesterday(string& date)
{
	unsigned int year, month, day;
	int leap = 0; //���꣬0��1��
	unsigned int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	unsigned int mdays_ly[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
	string sy, sm, sd;
	sy = date.substr(0,4);
	year = atoi(sy.c_str());
	if ( (year%400 ==0) || (year%100!=0 && year%4 ==0))
	{
		leap = 1;
	}
	sm = date.substr(4,2);
	month = atoi(sm.c_str());
	sd = date.substr(6,2);
	day = atoi(sd.c_str());

	if (day>1)
	{
		day--;
	}
	else if (month>1)
	{
		month--;
		if (leap == 0)
			day = mdays[month-1];
		else
			day = mdays_ly[month-1];
	}
	else
	{
		month = 12;
		day = 31;
		year--;
	}
	char tmp[5];
	sprintf(tmp, "%04d", year);
	sy = tmp;
	sprintf(tmp, "%02d", month);
	sm = tmp;
	sprintf(tmp, "%02d", day);
	sd = tmp;
	date = sy+sm+sd;

	return 0;
}

int load_cheatpid(const char * filename)
{
	FILE * in;
	in = fopen(filename, "r");
	if (in == NULL)
	{
		return -1;
	}
	char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
	char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
	while (fgets(rawRecord, MAX_RAW_RECORD_LENGTH, in) != NULL)
	{
		int size;
		parse_string(rawRecord, "\t", tokens, &size);
		if (size >= 2)
			cheatpid_map[tokens[1]] = 1;
	}
	cerr << "cheatpid_map size: " << cheatpid_map.size() << endl;

	return 0;
}

int load_index(const char * d) // ��������ʱ�䣬�Դ�ǰ�ƣ���������ʮ�죬ʱ���ʽ�磺20090224
{
	string date(d);
	string filename;
	FILE * pIndex;
	//char hashKeyStr[MAX_STRING_LENGTH];
	unsigned int hashKeyInt;
	HashStringIndexType::iterator itStringIndex;
	//HashStringIndexListType::iterator itStringIndexList;
	HashMD5IndexListType::iterator itMD5IndexList;
	HashIntIndexType::iterator itIntIndex;
	struct HashIndex currIndex;

	int ii; // day loop 
	for (ii = 0; ii < MAX_DAY; ii++)
	{
		// query index
		filename = string("log/")+date+string("/query.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				//strcpy(hashKeyStr, tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				md5_long_32 md5_output = getSign32((const char*)tokens[0], -1);
				uint32_t query_md5 = md5_output.data.intData[0];
				itMD5IndexList = hashQueryIndex.find(query_md5);
				if (itMD5IndexList != hashQueryIndex.end())
				{
					//itStringIndex->second[ii] = currIndex;
					// ��Ϊһ�����������
					HashIndexNode * preNode, *currNode;
					HashIndexNode * newNode = new HashIndexNode;
					newNode->N = ii;
					newNode->start = currIndex.start;
					newNode->len = currIndex.len;
					newNode->next = NULL;
					currNode = itMD5IndexList->second->next;
					preNode = itMD5IndexList->second;
					while (currNode != NULL)
					{
						preNode = currNode;
						currNode = preNode->next;
					}
					preNode->next = newNode;

				}
				else // �²���
				{
					/*hashQueryIndex[tokens[0]] = new HashIndex[MAX_DAY];
					itStringIndex = hashQueryIndex.find(tokens[0]);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itStringIndex->second[jj] = tmp;
					}
					itStringIndex->second[ii] = currIndex;*/
					hashQueryIndex[query_md5] = new HashIndexNode;
					hashQueryIndex[query_md5]->N = ii;
					hashQueryIndex[query_md5]->start = currIndex.start;
					hashQueryIndex[query_md5]->len = currIndex.len;
					hashQueryIndex[query_md5]->next = NULL;
				}
			}
			fclose(pIndex);
		}

		// adid index
		filename = string("log/")+date+string("/adid.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				hashKeyInt = (unsigned int)atoi(tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				itIntIndex = hashAdidIndex.find(hashKeyInt);
				if (itIntIndex != hashAdidIndex.end())
				{
					itIntIndex->second[ii] = currIndex;
				}
				else // �²���
				{
					hashAdidIndex[hashKeyInt] = new HashIndex[MAX_DAY];
					itIntIndex = hashAdidIndex.find(hashKeyInt);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itIntIndex->second[jj] = tmp;
					}
					itIntIndex->second[ii] = currIndex;
				}
			}
			fclose(pIndex);
		}

		// region index
		filename = string("log/")+date+string("/region.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				//strcpy(hashKeyStr, tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				itStringIndex = hashRgIndex.find(tokens[0]);
				if (itStringIndex != hashRgIndex.end())
				{
					itStringIndex->second[ii] = currIndex;
				}
				else // �²���
				{
					hashRgIndex[tokens[0]] = new HashIndex[MAX_DAY];
					itStringIndex = hashRgIndex.find(tokens[0]);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itStringIndex->second[jj] = tmp;
					}
					itStringIndex->second[ii] = currIndex;
				}
			}
			fclose(pIndex);
		}

		// pos index
		filename = string("log/")+date+string("/pos.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				hashKeyInt = (unsigned int)atoi(tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				itIntIndex = hashPosIndex.find(hashKeyInt);
				if (itIntIndex != hashPosIndex.end())
				{
					itIntIndex->second[ii] = currIndex;
				}
				else // �²���
				{
					hashPosIndex[hashKeyInt] = new HashIndex[MAX_DAY];
					itIntIndex = hashPosIndex.find(hashKeyInt);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itIntIndex->second[jj] = tmp;
					}
					itIntIndex->second[ii] = currIndex;
				}
			}
			fclose(pIndex);
		}

		// acc id index
		filename = string("log/")+date+string("/accid.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				hashKeyInt = (unsigned int)atoi(tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				itIntIndex = hashAccidIndex.find(hashKeyInt);
				if (itIntIndex != hashAccidIndex.end())
				{
					itIntIndex->second[ii] = currIndex;
				}
				else // �²���
				{
					hashAccidIndex[hashKeyInt] = new HashIndex[MAX_DAY];
					itIntIndex = hashAccidIndex.find(hashKeyInt);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itIntIndex->second[jj] = tmp;
					}
					itIntIndex->second[ii] = currIndex;
				}
			}
			fclose(pIndex);
		}

		// src index
		filename = string("log/")+date+string("/src.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				//strcpy(hashKeyStr, tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				itStringIndex = hashSrcIndex.find(tokens[0]);
				if (itStringIndex != hashSrcIndex.end())
				{
					itStringIndex->second[ii] = currIndex;
				}
				else // �²���
				{
					hashSrcIndex[tokens[0]] = new HashIndex[MAX_DAY];
					itStringIndex = hashSrcIndex.find(tokens[0]);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itStringIndex->second[jj] = tmp;
					}
					itStringIndex->second[ii] = currIndex;
				}
			}
			fclose(pIndex);
		}

		/*/ src4 index
		filename = string("log/")+date+string("/src4.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				//strcpy(hashKeyStr, tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				itStringIndex = hashSrc4Index.find(tokens[0]);
				if (itStringIndex != hashSrc4Index.end())
				{
					itStringIndex->second[ii] = currIndex;
				}
				else // �²���
				{
					hashSrc4Index[tokens[0]] = new HashIndex[MAX_DAY];
					itStringIndex = hashSrc4Index.find(tokens[0]);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itStringIndex->second[jj] = tmp;
					}
					itStringIndex->second[ii] = currIndex;
				}
			}
			fclose(pIndex);
		}//*/

		// cate index
		filename = string("log/")+date+string("/cate.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				hashKeyInt = (unsigned int)atoi(tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				itIntIndex = hashCateIndex.find(hashKeyInt);
				if (itIntIndex != hashCateIndex.end())
				{
					itIntIndex->second[ii] = currIndex;
				}
				else // �²���
				{
					hashCateIndex[hashKeyInt] = new HashIndex[MAX_DAY];
					itIntIndex = hashCateIndex.find(hashKeyInt);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itIntIndex->second[jj] = tmp;
					}
					itIntIndex->second[ii] = currIndex;
				}
			}
			fclose(pIndex);
		}

		// adkey index
		filename = string("log/")+date+string("/adkey.index");
		pIndex = fopen(filename.c_str(),"r");
		// �п��ܵ�ǰ�ļ������ڣ�������û������
		if (pIndex != NULL)
		{
			char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
			char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
			while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
			{
				int size;
				parse_string(rawRecord, "\t", tokens, &size);
				//strcpy(hashKeyStr, tokens[0]);
				currIndex.start = atoi(tokens[1]);
				currIndex.len = atoi(tokens[2]);
				itStringIndex = hashAdkeyIndex.find(tokens[0]);
				if (itStringIndex != hashAdkeyIndex.end())
				{
					itStringIndex->second[ii] = currIndex;
				}
				else // �²���
				{
					hashAdkeyIndex[tokens[0]] = new HashIndex[MAX_DAY];
					itStringIndex = hashAdkeyIndex.find(tokens[0]);
					for (int jj = 0; jj<MAX_DAY; jj++) // ��ʼ��Ϊ0������Ϊ0����ʾû��
					{
						HashIndex tmp = {0, 0};
						itStringIndex->second[jj] = tmp;
					}
					itStringIndex->second[ii] = currIndex;
				}
			}
			fclose(pIndex);
		}

		// ��ǰһ��
		calc_yesterday(date);
	}

	cerr << "hashQueryIndex size: " << hashQueryIndex.size() << endl;
	cerr << "hashAdidIndex size: " << hashAdidIndex.size() << endl; 
	cerr << "hashRgIndex size: " << hashRgIndex.size() << endl; 
	cerr << "hashAccidIndex size: " << hashAccidIndex.size() << endl; 
	cerr << "hashPosIndex size: " << hashPosIndex.size() << endl; 
	cerr << "hashSrcIndex size: " << hashSrcIndex.size() << endl; 
	cerr << "hashCateIndex size: " << hashCateIndex.size() << endl; 
	cerr << "hashAdkeyIndex size: " << hashAdkeyIndex.size() << endl; 
	
	return 0;
}

// ����
bool AdDisplayLess(const struct AdDisplay &a, const struct AdDisplay &b, int criteria) // compare the structure using pv/clk/ctr/price, a<b: true; else: false
{
	switch(criteria)
	{
	case 0: // pv
		return a.nPv < b.nPv;
	case 1: // clk
		return a.nClick < b.nClick;
	case 2: // ctr, float type, there's no equal ............
		return a.ctr < b.ctr;
	case 3: // price
		return a.cost < b.cost;
	default:
		return false; // exception
	}
}

int vector_TopHeapSort(std::vector<struct AdDisplay> &arr, int top, int criteria) // find the top ad info using pv/clk/ctr/price
{
	int i, n;
	// build max heap
	n = arr.size();
	for (i=n/2-1; i>=0; i--) // only consider non-leave nodes
	{
		int cur = i;
		int chd = cur*2+1;
		while (chd < n)
		{
			if (chd < n-1 && AdDisplayLess(arr[chd], arr[chd+1], criteria))
			{
				chd++;
			}
			if (!AdDisplayLess(arr[cur], arr[chd], criteria))
			{
				break; // no adjust needed
			}
			else
			{
				std::swap(arr[cur],arr[chd]);
				cur = chd;
				chd = cur*2+1;
			}
		}
	}
	// sort
	n = arr.size()-1;
	i = 0;
	while (n>0 && i<top) // if size < top, sort all, else only sort the top ones
	{
		// swap the first and last
		std::swap(arr[0], arr[n]);
		n--;
		i++;
		// adjust heap
		int cur = 0;
		int chd = cur*2+1;
		while (chd <= n)
		{
			if (chd <= n-1 && AdDisplayLess(arr[chd], arr[chd+1], criteria))
			{
				chd++;
			}
			if (!AdDisplayLess(arr[cur], arr[chd], criteria))
			{
				break;
			}
			else
			{
				std::swap(arr[cur], arr[chd]);
				cur = chd;
				chd = cur*2+1;
			}			
		}
	}
	return 0; // no meaning at this time
}

// tool: �ɱ�ŷ����ַ���������ֵ
string getFeature(const AdSearchRecord SearchFeatures, int dim)
{
	string str;
	char tmp[32];
	switch(dim)
	{
		case 0:
			str = SearchFeatures.query;
			break;
		case 1:
		        sprintf(tmp, "%d", SearchFeatures.adid);
			str = tmp;
			break;
		case 2:
			str = SearchFeatures.region;
			break;
		case 3:
			sprintf(tmp, "%d", SearchFeatures.pos);
			str = tmp;
			break;
		case 4:
			sprintf(tmp, "%d", SearchFeatures.accid);
			str = tmp;
			break;
		case 5:
			str = SearchFeatures.src;
			break;
		case 6:
			str = SearchFeatures.src4;
			break;
		case 7:
			if (SearchFeatures.cate == 0)
			{
				str = "cate_not_found";
			}
			else
			{
				sprintf(tmp, "%d", SearchFeatures.cate);
				str = tmp;
			}
			break;
		case 8:
			str = SearchFeatures.adkey;
			break;
		default:
			break;
	}
	return str; 
}

// ad search func: 
// In: SearchFeatures[], dispFeatures, isTestAcc, date(������������ڼ�ǰ�Ƶ�������, NΪ�������ڶ�Ӧ�����������±꣨��С�±꣩
// Out: DispList
// return: size of the result
//int ad_search(const string SearchFeatures[FEATURE_DIM], const std::vector<int> &dispFeatures, bool isTestAcc, int sortCrt, std::vector<AdDisplay> &DispList)
int ad_search(const string SearchFeatures[FEATURE_DIM], const std::vector<int> &dispFeatures, 
		const string &date0, const int N, const int num_day, bool isTestAcc, bool isPidFree, bool isCheat, int sortCrt, int top, std::vector<AdDisplay> &DispList)
{
	FILE * pData;
	string date = date0;
	string names[FEATURE_DIM] = {
		"/query.dat",
		"/adid.dat",
		"/region.dat",
		"/pos.dat",
		"/accid.dat",
		"/src.dat",
		"/src.dat", // "/src4.dat"
		"/cate.dat",
		"/adkey.dat"
	};   
	string filename;
	//std::vector<int> usedFeatures; // the used search features' number
	//HashFeatureType::iterator itHashs[FEATURE_DIM]; // hashmap iterators
	HashStringIndexType::iterator itStringIndex;
	//HashStringIndexListType::iterator itStringIndexList;
	HashMD5IndexListType::iterator itMD5IndexList;
	HashIntIndexType::iterator itIntIndex;

	bool usedFeatures[FEATURE_DIM]; // the used search features' number
	for (int i=0; i<FEATURE_DIM; i++)
	{   
		if (!SearchFeatures[i].empty()) // this search criteria is used
		{   
			usedFeatures[i] = true; // this feature used
		}   
		else
		{
			usedFeatures[i] = false;
		}
	}   

	bool isSearchCondition = false; // �Ƿ��в�ѯ��������Ϊ�У���Ϊû��
	for ( int ii = 0; ii<FEATURE_DIM; ii++)
	{
		if (usedFeatures[ii])
		{
			isSearchCondition = true;
			break;
		}
	}
	// �����ж�չʾ����
	if (dispFeatures.empty())
	{
		std::cerr << "Please specify display criteria.\n";
		//return 0;
	}
	else if (!isSearchCondition) // ��չʾ��������û�в�ѯ����
	{
		// if no search condition specified and there's only 1 display condition, merge the hashmap in the name of the display condition
		if (dispFeatures.size() == 1)
		{
			string Indexnames[FEATURE_DIM] = {
				"/query.index",
				"/adid.index",
				"/region.index",
				"/pos.index",
				"/accid.index",
				"/src.index",
				"/src.index", // "/src4.index"
				"/cate.index",
				"/adkey.index"
			};   
			HashDisplayType hashDisp;
			HashDisplayType::iterator itHashDisp;
			// ����չʾ�������ļ�
			for (int nn = N; nn< N+num_day && nn<MAX_DAY; nn++)
			{
				filename = string("log/")+date+names[dispFeatures[0]];
				string indexfile = string("log/")+date+Indexnames[dispFeatures[0]];
				pData = fopen(filename.c_str(), "r");
				FILE *pIndex = fopen(indexfile.c_str(), "r");
				if (pData != NULL && pIndex != NULL)
				{
					unsigned int len;
					char rawRecord[MAX_RAW_RECORD_LENGTH]; // the whole line from log file
					char tokens[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH]; // attributes with their values of the whole line
					AdDisplay adElem;
					AdSearchRecord ad;
					while( fgets(rawRecord, MAX_RAW_RECORD_LENGTH, pIndex) != NULL )
					{    
						int size;
						parse_string(rawRecord, "\t", tokens, &size);
						adElem.key = tokens[0];
						adElem.nPv = 0;
						adElem.nClick = 0;
						adElem.cost = 0;
						len = atoi(tokens[2]);
						for (unsigned int ii=0; ii<len; ii++)
						{
							fread(&ad, sizeof(AdSearchRecord), 1, pData);
							if (isTestAcc && ad.isTest==1)
							{
								continue; // �������ʺ�
							}
							adElem.nPv += ad.nPv;
							adElem.nClick += ad.nClick;
							//if (ad.nClick != 0)
							//{
							adElem.cost += ad.nClick * ad.price;
							//}
						}
						//adElem.cost *= adElem.nClick;
						//adElem.ctr = (float)adElem.nClick/adElem.nPv;
						//����hash
						// hash the key to a hashmap
						itHashDisp = hashDisp.find(adElem.key);
						if (itHashDisp != hashDisp.end())
						{
							itHashDisp->second.nPv += adElem.nPv;
							itHashDisp->second.nClick += adElem.nClick;
							// there may be several record corresponding to this key
							// some of their prices are 0, some are NOT, so the value need to be updated
							//if (adElem.nClick != 0) // click happen
							//{
							itHashDisp->second.cost += adElem.cost;
							//}
						}
						else // new data
						{
							hashDisp[adElem.key].key = adElem.key;
							hashDisp[adElem.key].nPv = adElem.nPv;
							hashDisp[adElem.key].nClick = adElem.nClick;
							hashDisp[adElem.key].cost = adElem.cost;
						}
					}
					fclose(pData);
					fclose(pIndex);
				}
				calc_yesterday(date);
			}
			// ��hash����
			for (itHashDisp = hashDisp.begin(); itHashDisp != hashDisp.end(); ++itHashDisp)
			{
				itHashDisp->second.ctr = (float)itHashDisp->second.nClick/itHashDisp->second.nPv;
				//itHashDisp->second.cost *= itHashDisp->second.nClick;
				DispList.push_back(itHashDisp->second);
				//std::cout<<itHashDisp->second.nPv<<std::endl;
			}
			// sort, the criteria needed: pv, click, ctr, cost
			vector_TopHeapSort(DispList, top, sortCrt);

			//return DispList.size();
		}
		else
		{
			std::cerr << "for no search criteria conidtion, please specifiy only 1 display criteria.\n";
			//return 0;
		}
	}
	else // �в�ѯ������Ҳ��չʾ����
	{
		HashDisplayType hashDisp;
		HashDisplayType::iterator itHashDisp;
		int nn; // NΪ��һ�����ڶ�Ӧ�����������±꣬ѭ�������ڵݼ��������
		for (nn = N; nn < N+num_day && nn<MAX_DAY; nn++) // һ��һ��飬����hash
		{
			// �ҵ���С�����ݣ���Ӳ�̶���
			filename = string("log/")+date+names[0];
			unsigned int minLen = INFINTE;
			unsigned int start = 0;
			if (usedFeatures[0]) // ��ѯ��ά�����ڲ�ѯ
			{
				md5_long_32 md5_output = getSign32((const char*)SearchFeatures[0].c_str(), -1);
				uint32_t query_md5 = md5_output.data.intData[0];
				itMD5IndexList = hashQueryIndex.find(query_md5);
				if (itMD5IndexList == hashQueryIndex.end())
				{
					// ĳһ��ѯ�����޽�������±��β�ѯʧ��
					minLen = 0;
				}
				else //if (itStringIndex->second[nn].len < minLen)
				{
					//minLen = itStringIndex->second[nn].len;
					//start = itStringIndex->second[nn].start;
					// ��������Ѱ��N==nn������û�У���ʧ��
					HashIndexNode * pNode = itMD5IndexList->second;
					while (pNode != NULL && pNode->N != nn)
					{
						pNode = pNode->next;
					}
					if (pNode != NULL)
					{
						minLen = pNode->len;
						start = pNode->start;
					}
					else
					{
						minLen = 0;
					}
				}
			}
			if (usedFeatures[1]) // ��ά�����ڲ�ѯ
			{
				itIntIndex = hashAdidIndex.find(atoi(SearchFeatures[1].c_str()));
				if (itIntIndex == hashAdidIndex.end())
				{
					// ĳһ��ѯ�����޽�������±��β�ѯʧ��
					minLen = 0;
				}
				else if (itIntIndex->second[nn].len < minLen)
				{
					filename = string("log/")+date+names[1];
					minLen = itIntIndex->second[nn].len;
					start = itIntIndex->second[nn].start;
				}
			}
			if (usedFeatures[2]) // ��ά�����ڲ�ѯ
			{
				itStringIndex = hashRgIndex.find(SearchFeatures[2]);
				if (itStringIndex == hashRgIndex.end())
				{
					// ĳһ��ѯ�����޽�������±��β�ѯʧ��
					minLen = 0;
				}
				else if (itStringIndex->second[nn].len < minLen)
				{
					filename = string("log/")+date+names[2];
					minLen = itStringIndex->second[nn].len;
					start = itStringIndex->second[nn].start;
				}
			}
			if (usedFeatures[3]) // ��ά�����ڲ�ѯ
			{
				itIntIndex = hashPosIndex.find(atoi(SearchFeatures[3].c_str()));
				if (itIntIndex == hashPosIndex.end())
				{
					// ĳһ��ѯ�����޽�������±��β�ѯʧ��
					minLen = 0;
				}
				else if (itIntIndex->second[nn].len < minLen)
				{
					filename = string("log/")+date+names[3];
					minLen = itIntIndex->second[nn].len;
					start = itIntIndex->second[nn].start;
				}
			}
			if (usedFeatures[4]) // ��ά�����ڲ�ѯ
			{
				itIntIndex = hashAccidIndex.find(atoi(SearchFeatures[4].c_str()));
				if (itIntIndex == hashAccidIndex.end())
				{
					// ĳһ��ѯ�����޽�������±��β�ѯʧ��
					minLen = 0;
				}
				else if (itIntIndex->second[nn].len < minLen)
				{
					filename = string("log/")+date+names[4];
					minLen = itIntIndex->second[nn].len;
					start = itIntIndex->second[nn].start;
				}
			}
			if (usedFeatures[5]) // pid ��ά�����ڲ�ѯ
			{
				string src = SearchFeatures[5];
				size_t sz = src.size()-1;
				if (src[sz] == '*')
					src.erase(sz);
				itStringIndex = hashSrcIndex.find(src);
				if (itStringIndex == hashSrcIndex.end())
				{
					/* ȫ����ΪVy��key��������Ҫ���ж�
					if (src.size() == 32)
					{
						// �����-xxxx��βȥ��������һ���µ�hash��
						string src_mid = src.substr(0,27);
						itStringIndex = hashSrcIndex.find(src_mid);
						if (itStringIndex == hashSrcIndex.end())
						{
							minLen = 0;
						}
						else if (itStringIndex->second[nn].len < minLen)
						{
							filename = string("log/")+date+names[5];
							minLen = itStringIndex->second[nn].len;
							start = itStringIndex->second[nn].start;
						}
					}
					else // ����Ҫ����hash��
					{
						minLen = 0;
					}//*/
					minLen = 0;
				}
				else if (itStringIndex->second[nn].len < minLen)
				{
					filename = string("log/")+date+names[5];
					minLen = itStringIndex->second[nn].len;
					start = itStringIndex->second[nn].start;
				}
			}
			if (usedFeatures[6]) // ��ά�����ڲ�ѯ
			{
				string src = SearchFeatures[6];
				size_t sz = src.size()-1;
				if (src[sz] == '*')
					src.erase(sz);
				itStringIndex = hashSrcIndex.find(src);
				if (itStringIndex == hashSrcIndex.end())
				{
					// ĳһ��ѯ�����޽�������±��β�ѯʧ��
					minLen = 0;
				}
				else if (itStringIndex->second[nn].len < minLen)
				{
					filename = string("log/")+date+names[6];
					minLen = itStringIndex->second[nn].len;
					start = itStringIndex->second[nn].start;
				}
			}
			if (usedFeatures[8]) // ��ά�����ڲ�ѯ
			{
				itStringIndex = hashAdkeyIndex.find(SearchFeatures[8]);
				if (itStringIndex == hashAdkeyIndex.end())
				{
					// ĳһ��ѯ�����޽�������±��β�ѯʧ��
					minLen = 0;
				}
				else if (itStringIndex->second[nn].len < minLen)
				{
					filename = string("log/")+date+names[8];
					minLen = itStringIndex->second[nn].len;
					start = itStringIndex->second[nn].start;
				}
			}
			// cate: 
			// �����������ֻƥ��ǰ��λ
			if (usedFeatures[7]) // ��ά�����ڲ�ѯ
			{
				unsigned int cate = atoi(SearchFeatures[7].c_str());
				itIntIndex = hashCateIndex.find(cate);
				if (itIntIndex == hashCateIndex.end())
				{
					/* ȫ����ΪVy��key��������Ҫ���ж�
					// ĳһ��ѯ�����޽�������±��β�ѯʧ��
					if (minLen == INFINTE) // ����֮��û�в�ѯ������ʹ�ã�Ҫ����ȫƥ��ſ���
					{
						minLen = 0;
					}//
					// ���Ǵ������Ƿ���ƥ��
					itIntIndex = hashCateIndex.find(cate/10000);
					if (itIntIndex == hashCateIndex.end())
					{
						minLen = 0;
					}
					else if (itIntIndex->second[nn].len < minLen)
					{
						filename = string("log/")+date+names[7];
						minLen = itIntIndex->second[nn].len;
						start = itIntIndex->second[nn].start;
					}*/
					minLen = 0;
				}
				else if (itIntIndex->second[nn].len < minLen)
				{
					filename = string("log/")+date+names[7];
					minLen = itIntIndex->second[nn].len;
					start = itIntIndex->second[nn].start;
				}
			}
			
			if (minLen*sizeof(AdSearchRecord) > 3000000000)
			{
				return -2;
			}

			// ���ļ��������λ��
			AdSearchRecord * rawlist;
			//int * flg_list;
			// �����ѯû�н�������������죬תǰһ�죨����Ƕ����ѯ��
			if (minLen == 0)
			{
				calc_yesterday(date);
				continue;
			}
			else // ���ļ�������
			{
				//read
				pData = fopen(filename.c_str(), "r");
				if (pData != NULL)
				{
					fseek(pData, start*sizeof(AdSearchRecord), SEEK_SET);
					try{ 
						rawlist = new AdSearchRecord[minLen];
					}catch (const bad_alloc& e){
						return -1;
					} 

					fread(rawlist, sizeof(AdSearchRecord), minLen, pData);
					fclose(pData);
					// ת��һ��
					calc_yesterday(date);
				}
				else // ���ļ�ʧ�ܣ�Ҳת��һ��
				{
					calc_yesterday(date);
					continue;
				}
			}
			// ���ж����ѯ����������˶��������
			// ���˷����������������Ĳ�����hash
			/*std::list<AdSearchRecord> resultlist;
			std::list<AdSearchRecord>::iterator it;
			for ( int ii=0; ii<minLen; ii++)
			{
				resultlist.push_back(rawlist[ii]);
			}
			delete[] rawlist;
			// ����
			for (it=resultlist.begin(); it!=resultlist.end(); ++it)
			{
				// query
				if (usedFeatures[0])
				{
					if (strcmp(it->query, SearchFeatures.query)!=0)
					{
						resultlist.erase(it);
						continue;
					}
				}
				// adid
				if (usedFeatures[1])
				{
					if (it->adid!=SearchFeatures.adid)
					{
						resultlist.erase(it);
						continue;
					}
				}
				// region
				if (usedFeatures[2])
				{
					if (strcmp(it->region, SearchFeatures.region)!=0)
					{
						resultlist.erase(it);
						continue;
					}
				}
				// pos
				if (usedFeatures[3])
				{
					if (it->pos!=SearchFeatures.pos)
					{
						resultlist.erase(it);
						continue;
					}
				}
				// accid
				if (usedFeatures[4])
				{
					if (it->accid!=SearchFeatures.accid)
					{
						resultlist.erase(it);
						continue;
					}
				}
				// src
				if (usedFeatures[5])
				{
					if (strcmp(it->src, SearchFeatures.src)!=0)
					{
						resultlist.erase(it);
						continue;
					}
				}
				// src4
				if (usedFeatures[6])
				{
					if (strcmp(it->src4, SearchFeatures.src4)!=0)
					{
						resultlist.erase(it);
						continue;
					}
				}
				// cate
				if (usedFeatures[7])
				{
					if (it->cate!=SearchFeatures.cate)
					{
						resultlist.erase(it);
						continue;
					}
				}
				// adkey
				if (usedFeatures[8])
				{
					if (strcmp(it->adkey, SearchFeatures.adkey)!=0)
					{
						resultlist.erase(it);
						continue;
					}
				}
			}*/

			// ���µ�����д����ʾhash
			// using display criteria to display the search results
			//for (it = resultlist.begin(); it!=resultlist.end(); ++it)
			unsigned int ii;
			for (ii = 0; ii < minLen; ii++)
			{
				// query
				if (usedFeatures[0])
				{
					if (strcmp(rawlist[ii].query, SearchFeatures[0].c_str())!=0)
					{
						continue;
					}
				}
				// adid
				if (usedFeatures[1])
				{
					if (rawlist[ii].adid!=(unsigned int)atoi(SearchFeatures[1].c_str()))
					{
						continue;
					}
				}
				// region
				if (usedFeatures[2])
				{
					if (strcmp(rawlist[ii].region, SearchFeatures[2].c_str())!=0)
					{
						continue;
					}
				}
				// pos
				if (usedFeatures[3])
				{
					if (rawlist[ii].pos!=atoi(SearchFeatures[3].c_str()))
					{
						continue;
					}
				}
				// accid
				if (usedFeatures[4])
				{
					if (rawlist[ii].accid!=(unsigned int)atoi(SearchFeatures[4].c_str()))
					{
						continue;
					}
				}
				// src
				// ����һ��PID��Ҫ�������ж�
				if (usedFeatures[5])
				{
					string search_pid = SearchFeatures[5];
					size_t sz = search_pid.size()-1;
					if (search_pid[sz] == '*')
						search_pid.erase(sz);
					string pid = rawlist[ii].src;
					if (search_pid.substr(0,6)=="sogou-" && pid.substr(0,6) == "sogou-")
					{    
						if (pid.substr(0, search_pid.size()) != search_pid)
						{    
							continue;
						}    
					}    
					else 
					{    
						if (pid != search_pid)
						{
							continue;
						}
					}
				}
				// src4
				if (usedFeatures[6])
				{
					if (strcmp(rawlist[ii].src4, SearchFeatures[6].c_str())!=0)
					{
						continue;
					}
				}
				// cate
				// ֧�ִ��࣬�������ǰ��λ�ܶ���Ҳ��������
				if (usedFeatures[7])
				{
					char tmp[16];
					sprintf(tmp, "%u", rawlist[ii].cate);
					string cate = tmp;
					if (cate.substr(0, SearchFeatures[7].size()) != SearchFeatures[7])
					{
						continue;
					}
				}
				// adkey
				if (usedFeatures[8])
				{
					if (strcmp(rawlist[ii].adkey, SearchFeatures[8].c_str())!=0)
					{
						continue;
					}
				}
				//AdSearchRecord currRd = *it;
				if (isTestAcc && rawlist[ii].isTest==1)
				{
					continue; // don't display test acc and this one IS a test acc
				}
				if (isPidFree) // ȥ��sogou__free sohu__free
				{
					string src = rawlist[ii].src;
					if (src == string("sogou__free") || src == string("sohu__free") || src == string("bd__free"))
					{
						continue;
					}
				}
				if (isCheat) // ȥ������pid
				{
					if (cheatpid_map.find(rawlist[ii].src) != cheatpid_map.end())
						continue;
				}

				// generating the hash key
				string key;
				int i;
				for (i=0; i<(int)dispFeatures.size(); i++)
				{
					if (i == 0)
					{
						key += getFeature(rawlist[ii], dispFeatures[i]);
					}
					else
					{
						//key += "+";
						key += "</td><td>";
						key += getFeature(rawlist[ii], dispFeatures[i]);
					}
				}
				// hash the key to a hashmap
				itHashDisp = hashDisp.find(key);
				if (itHashDisp != hashDisp.end())
				{
					itHashDisp->second.nPv += rawlist[ii].nPv;
					itHashDisp->second.nClick += rawlist[ii].nClick;
					// there may be several record corresponding to this key
					// some of their prices are 0, some are NOT, so the value need to be updated
					//if (rawlist[ii].nClick != 0) // click happen
					//{
					itHashDisp->second.cost += rawlist[ii].nClick * rawlist[ii].price;
					//}//
				}
				else // new data
				{
					hashDisp[key].key = key;
					hashDisp[key].nPv = rawlist[ii].nPv;
					hashDisp[key].nClick = rawlist[ii].nClick;
					hashDisp[key].cost = rawlist[ii].nClick * rawlist[ii].price;
				}
			}
			delete[] rawlist;
		}

		// sort the display hashmap
		for (itHashDisp = hashDisp.begin(); itHashDisp != hashDisp.end(); ++itHashDisp)
		{
			itHashDisp->second.ctr = (float)itHashDisp->second.nClick/itHashDisp->second.nPv;
			//itHashDisp->second.cost *= itHashDisp->second.nClick;
			DispList.push_back(itHashDisp->second);
			//std::cout<<itHashDisp->second.nPv<<std::endl;
		}
		// sort, the criteria needed: pv, click, ctr, cost
		//sort(DispList.begin(), DispList.end(), PvGreater);
		vector_TopHeapSort(DispList, top, sortCrt);

		//return DispList.size();
	}

	return DispList.size();
}

// �ɲ�Ѱ����б����ɷ��ظ�ҳ��������ַ���
int outStr(const std::vector<AdDisplay> &Reslt, string &adbuf, int top)
{
	int dispLen, curLen, i;
	curLen = (int)Reslt.size();
	char tmp[255];
	if ((int)Reslt.size() > top)
	{
		dispLen = top;
	}
	else
	{
		dispLen = curLen;
	}	
	// ͬʱ�����������ܺ�
	int npv = 0;
	int nclick = 0;
	int cost = 0;
	float price = 0;
	for (i=curLen-1; i>=curLen-dispLen; i--)
	{
		adbuf += Reslt[i].key;
		// ������\tȷ��key�����벻�Ե��ָ���\t
		if (Reslt[i].nClick == 0)
		{
			price = 0;
		}
		else
		{
			price = (float)Reslt[i].cost/Reslt[i].nClick;
		}
		sprintf(tmp, "\t\t%d\t%d\t%.2f\t%d\t%.2f\t", Reslt[i].nPv, Reslt[i].nClick, 100*Reslt[i].ctr, Reslt[i].cost, price);
		adbuf += string(tmp);
	}
	for (i=0; i<(int)Reslt.size(); i++)
	{
		npv += Reslt[i].nPv;
		nclick += Reslt[i].nClick;
		cost += Reslt[i].cost;
	}
	if (nclick == 0)
	{
		price = 0;
	}
	else
	{
		price = (float)cost/nclick;
	}
	adbuf += string("�ܼ�");
	sprintf(tmp, "\t%d\t%d\t%.2f\t%d\t%.2f\t", npv, nclick, (float)nclick/npv*100, cost, price);
	adbuf += string(tmp);
	if (dispLen != curLen)
	{
		sprintf(tmp, "Top %d displayed, total is %d.\n", dispLen, curLen);
		adbuf += string(tmp);
	}
	
	return 0;
}

string getOfflineFilename(const string SearchWords[FEATURE_DIM], const std::vector<int> &dispWords)
{
	string filename;
	int i;
	// �ļ����Ĳ�ѯ���������ǲ�ѯ����
	for (i=0; i<FEATURE_DIM; i++)
	{
		filename += SearchWords[i];
	}
	// �ļ�����չʾ����������һ��01������չʾ�����Ƿ�ʹ��
	int dispfn[FEATURE_DIM] = {0};
	for (i=0; i<(int)dispWords.size(); i++)
	{
		dispfn[dispWords[i]] = 1;
	}
	for (i=0; i<FEATURE_DIM; i++)
	{
		if (dispfn[i] == 0)
			filename += "0";
		else
			filename += "1";
	}
	// ��׺
	filename += ".cache";

	return filename;
}

void * socket_sendrecv(void * sock)
{
	char buf[BUF_LEN];
	string out_string;
	int sockclient;
	int* p = (int *)sock;
	sockclient = *p;

	recv(sockclient, buf, BUF_LEN, 0);
	cout << "search request received:\n" << buf << endl;

	// search code here
	char in[MAX_KEYWORD_LENGTH][MAX_STRING_LENGTH];
	int size;
	parse_string(buf, "\t", in, &size);

	std::vector<AdDisplay> Reslt;

	string SearchWords[FEATURE_DIM];
	std::vector<int> dispWords; // the display features' number
	bool isTestAcc = false;
	bool isPidFree = false;
	bool isCheat = false;
	int sortCtr;
	int top; // top�������
	// ���ڲ�����num_dayͳ����������N��ʼ���ڣ��������ڣ���Ӧ�������±�
	string sdate, edate, currdate;
	int num_day, N;
	int i;

	// search criteria
	for (i = 0; i<FEATURE_DIM; i++)
	{
		if (string(in[i]) != "!") // this criteria is used
		{
			SearchWords[i] = in[i];
		}
	}
	// display criteria
	for (i = FEATURE_DIM; i<2*FEATURE_DIM; i++)
	{
		if (string(in[i]) == "1")
		{
			dispWords.push_back(i-FEATURE_DIM);
		}
	}
	// sort criteria
	sortCtr = atoi(in[2*FEATURE_DIM]);
	// filtering test account?
	if (string(in[2*FEATURE_DIM+1]) == "1")
	{
		isTestAcc = true;
	}
	if (string(in[2*FEATURE_DIM+2]) == "1")
	{
		isPidFree = true;
	}
	if (string(in[2*FEATURE_DIM+3]) == "1")
	{
		isCheat = true;
	}
	top = atoi(in[2*FEATURE_DIM+4]);
	if (top < 1)
		top = TOP_DISPLAY;

	// starting date & ending date, date format: "20090101"
	sdate = in[2*FEATURE_DIM+5];
	edate = in[2*FEATURE_DIM+6];
	currdate = in[2*FEATURE_DIM+7];
	// ��֤���ڼ�����ȷ
	if (sdate.size() != 8 || edate.size() != 8 || currdate.size() != 8)
	{
		out_string = "date error!";
	}
	else
	{
		// web�������ǵ�������ڣ�����ʼ����������ʵ�����죬�ʼ�һ��
		calc_yesterday(currdate);

		// ����ֹ����ǰ�Ƶ�����������ֹ���ڣ�
		num_day = calc_diffdate(edate, sdate)+1;
		// ��ֹ���ڶ�Ӧ�����±�
		N = calc_diffdate(currdate, edate);

		if (num_day<1 || N<0 || N>MAX_DAY) //��������
		{
			out_string = "date out of range";
		}
		else
		{
			/* ���޶�cache����
			string cachename = getOfflineFilename(SearchWords, dispWords);
			string cachepath = string("log/")+edate+string("/") + cachename;
			FILE * pCache;
			if (num_day==1 && (pCache=fopen(cachepath.c_str(),"r")) != NULL) // ��cache
			{
				fgets(buf, BUF_LEN, pCache);
				fclose(pCache);
			}
			else*/
			{
				int ret = ad_search(SearchWords, dispWords, edate, N, num_day, isTestAcc, isPidFree, isCheat, sortCtr, top, Reslt);
				// write search result to buf
				// display the search result
				if (ret < 0)
				{
					out_string = "�ڴ治������ѯ����:( ��ѡ��ϸ���Ĳ�ѯ����";
				}
				else if (!Reslt.empty())
				{
					outStr(Reslt, out_string, top);
				}
				else // return null string
				{
					out_string = "no result";
				}// end of display 
			}
		}
	}
	send(sockclient, out_string.c_str(), out_string.size(), 0);
	close(sockclient);

	return NULL;
}

int main(int argc, char * argv[])
{
	//assert(argc>2);
	if (argc < 2)
	{
		std::cerr << "Not enough input arguments!\n";
		std::cerr << argv[0]<<" date\n";
		return -1;
	}//*/

	// socket
	int sockfd;
	struct sockaddr_in serverAddr, clientAddr;
	//char buf[BUF_LEN];
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		// build socket failed !	
	}
	memset(&serverAddr, 0, sizeof(struct sockaddr_in)); // zero-padding
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_pton(AF_INET, "10.10.67.22", &serverAddr.sin_addr);
	int ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
	while (ret<0)
	{
		cerr << "binding failed. try again.\n";
		sleep(15);
		ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
	}
	cerr << "binding succeed, PORT: " << PORT << endl;

	// calculating the hash time
	struct timeval tvStart,tvEnd;
	double linStart = 0,linEnd = 0,lTime = 0;
	gettimeofday (&tvStart,NULL);

	cerr << "loading cheatpid ....\n";
	if (load_cheatpid("cheatpid") != 0)
	{
		cerr << "load cheatpid failed\n";
		return -1;
	}
	// load last a few day's index
	cerr << "loading index.....\n";
	load_index(argv[1]);

	// hash finished, giving the time 
	gettimeofday (&tvEnd,NULL);  
	linStart = ((double)tvStart.tv_sec * 1000000 + (double)tvStart.tv_usec);  //unit uS
	linEnd = ((double)tvEnd.tv_sec * 1000000 + (double)tvEnd.tv_usec);        //unit uS
	lTime = linEnd-linStart; 
	cerr << "load index time is " << lTime/1e6 << "s\n";
	
	/*/////////////////////////////////////////////////////////////////////
	// calculating the off-line search results
	// ֻ�ܼ��㵱������ݣ���ˣ����¼��㲻֧�ֶ��죬Ҳ��֧�ֶ��չʾ����
	// 4 dimensions:
	// ad region: ground, right
	// ad pos: none, 1~8
	// ad src4: sohu, sogou, sogou-*, others (sohu��sogou��С���е�Ҳ����)
	// display condiiton: query, adid, accid, pid, catalog, adkey
	// NOTE: none means this search condition not specified
	const int AdRg = 2;
	const int AdPos = 6;
	const int AdSrc = 4;
	const int AdDisp = 6;
	string RgWord[AdRg] = {"С����", "�Ҳ�"};
	string PosWord[AdPos] = {"", "1", "2", "3", "4", "5"};
	string SrcWord[AdSrc] = {"sohu", "sogou", "sogou-*", "others"};
	int DispWord[AdDisp] = {0,1,4,5,7,8};
	std::vector<AdDisplay> offlineReslt;
	char buf[BUF_LEN];
	
	std::vector<int> dispWords; // the display features' number
	int ii, jj, ll;

	// ���������λ�õ����
	for (ii=0; ii<AdRg; ii++)
	{
		string SearchWords[FEATURE_DIM];
		SearchWords[2] = RgWord[ii];
		for (jj=0; jj<AdPos; jj++)
		{
			SearchWords[3] = PosWord[jj];
			for (ll=0; ll<AdDisp; ll++)
			{
				dispWords.clear();
				dispWords.push_back(DispWord[ll]);
				// ֻ���㵱���ȥ�����ʺŰ��������Ľ��
				ad_search(SearchWords, dispWords, string(argv[1]), 0, 1, true, 1, offlineReslt);
				if (!offlineReslt.empty())
				{
					string adbuf;
					outStr(offlineReslt, adbuf);
					adbuf += "\nCache used.\n";
					strcpy(buf, adbuf.c_str());
				}
				else // return null string
				{
					strcpy(buf, "no result");
				} 
				// ��offlineResltд���ļ������ļ����������Ǻ��ֲ�ѯ
				// �����ļ���: �ļ������в�ѯ������չʾ������Ϣ��ʹ�����½��ʱ����������ΪĬ��
				string filename = getOfflineFilename(SearchWords, dispWords);
				string filepath = string("log/")+string(argv[1])+string("/") + filename;
				FILE * pCache = fopen(filepath.c_str(), "w");
				if (pCache != NULL)
				{
					fputs(buf, pCache);
					fclose(pCache);
				}
			} // end of disp loop
		} // end of pos loop
	} // end of region loop
	
	// �����Դ
	for (ii=0; ii<AdSrc; ii++)
	{
		string SearchWords[FEATURE_DIM];
		SearchWords[6] = SrcWord[ii];
		for (ll=0; ll<AdDisp; ll++)
		{
			dispWords.clear();
			dispWords.push_back(DispWord[ll]);
			// ֻ���㵱���ȥ�����ʺŰ��������Ľ��
			ad_search(SearchWords, dispWords, string(argv[1]), 0, 1, true, 1, offlineReslt);
			if (!offlineReslt.empty())
			{
				string adbuf;
				outStr(offlineReslt, adbuf);
				adbuf += "\nCache used.\n";
				strcpy(buf, adbuf.c_str());
			}
			else // return null string
			{
				strcpy(buf, "no result");
			} 
			// ��offlineResltд���ļ������ļ����������Ǻ��ֲ�ѯ
			// �����ļ���: �ļ������в�ѯ������չʾ������Ϣ��ʹ�����½��ʱ����������ΪĬ��
			string filename = getOfflineFilename(SearchWords, dispWords);
			string filepath = string("log/")+string(argv[1])+string("/") + filename;
			FILE * pCache = fopen(filepath.c_str(), "w");
			if (pCache != NULL)
			{
				fputs(buf, pCache);
				fclose(pCache);
			}
		}
	}
	// cache finished */

	/////////////////////////////////////////////////////////////////////////////////
	// search Ad starts here
	
	listen(sockfd, MAX_CONN);

	std::cerr << "listening started... server running\n";
	while (1)
	{
		int sockaddrSize = sizeof(struct sockaddr_in);
		int sockclient = accept(sockfd, (struct sockaddr *)&clientAddr, (socklen_t *)&sockaddrSize);
		
		pthread_t id;
		int ret;
		ret = pthread_create(&id, NULL, socket_sendrecv, (void *)&sockclient);

		//break;
	}
	close(sockfd);//*/

	// free memory
	HashStringIndexType::iterator itString;
	//HashStringIndexListType::iterator itStringList;
	HashMD5IndexListType::iterator itMD5List;
	HashIntIndexType::iterator itInt;
	for (itMD5List = hashQueryIndex.begin(); itMD5List != hashQueryIndex.end(); ++itMD5List)
	{
		HashIndexNode * curr = itMD5List->second;
		HashIndexNode * nxt = curr->next;
		while (nxt != NULL)
		{
			delete curr;
			curr = nxt;
			nxt = nxt->next;
		}
		delete curr;
		//delete[] itString->second;
	}
	for (itInt = hashAdidIndex.begin(); itInt != hashAdidIndex.end(); ++itInt)
	{
		delete[] itInt->second;
	}
	for (itString = hashRgIndex.begin(); itString != hashRgIndex.end(); ++itString)
	{
		delete[] itString->second;
	}
	for (itInt = hashPosIndex.begin(); itInt != hashPosIndex.end(); ++itInt)
	{
		delete[] itInt->second;
	}
	for (itInt = hashAccidIndex.begin(); itInt != hashAccidIndex.end(); ++itInt)
	{
		delete[] itInt->second;
	}
	for (itString = hashSrcIndex.begin(); itString != hashSrcIndex.end(); ++itString)
	{
		delete[] itString->second;
	}
	/*for (itString = hashSrc4Index.begin(); itString != hashSrc4Index.end(); ++itString)
	{
		delete[] itString->second;
	}//*/
	for (itInt = hashCateIndex.begin(); itInt != hashCateIndex.end(); ++itInt)
	{
		delete[] itInt->second;
	}
	for (itString = hashAdkeyIndex.begin(); itString != hashAdkeyIndex.end(); ++itString)
	{
		delete[] itString->second;
	}

	return 0;
}

/*
void search_display(const std::vector<AdDisplay> &DispList)
{
	// display the search result
	if (!DispList.empty())
	{
		std::cout<<"Search result:\n";
		std::cout<<setw(30)<<"key"<<setw(10)<<"Pv"<<setw(10)<<"Click"<<setw(10)<<"CTR (%)"<<setw(10)<<"cost"<<std::endl;
		int dispLen, curLen, i;
		curLen = (int)DispList.size();
		if (DispList.size() > TOP_DISPLAY)
		{
			dispLen = TOP_DISPLAY;
		}
		else
		{
			dispLen = curLen;
		}	
		for (i=curLen-1; i>=curLen-dispLen; i--)
		{
			std::cout<<setw(30)<<DispList[i].key
				<<setw(10)<<DispList[i].nPv
				<<setw(10)<<DispList[i].nClick
				<<setw(10)<<setprecision(2)<<100*DispList[i].ctr
				<<setw(10)<<DispList[i].cost<<"\n";
		}
		if (dispLen != curLen)
		{
			std::cout<<"Top 100 displayed\nTotal is "<<curLen<<std::endl;
		}
	}// end of display
}

int list_intersection(const std::list<int> * list1, const std::list<int> * list2, std::list<int> &interlist) // list intersection function
{
	// using merge sort (list1 and list2 are ordered lists) to do the intersection, return the size of the intersection set
	if (interlist.size() != 0)// make sure the return list is empty right now
	{
		interlist.clear();
	}

	typedef std::list<int>::const_iterator ListItType;
	ListItType it1, it2;
	it1 = list1->begin();
	it2 = list2->begin();

	while (it1 != list1->end() && it2 != list2->end())
	{
		if (*it1 > *it2)
		{
			++it2;
		}
		else if (*it1 < *it2)
		{
			++it1;
		}
		else
		{
			interlist.push_back(*it1);
			++it1;
			++it2;
		}
	}

	return interlist.size();
}
*/
