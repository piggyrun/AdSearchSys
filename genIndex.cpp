#define _FILE_OFFSET_BITS 64

#include <iostream>
#include <fstream>
#include <sys/time.h>

#include "AdSearchDef.h"
#include "Platform/md5/md5.h"

using namespace std; 

// global varibles
HashTestAccType hashTestAcc;
char * ptext = new char[TEXT_MEM_SIZE]; // uniq词的内存空间
char * ppid = new char[PID_MEM_SIZE]; // uniq pid的内存空间

vector<string> parse_string(const string& src, string tok, bool trim=false, string null_subst="");
int load_testAcc(const char *file); // load test account list, and write to global variable hashTestAcc
int genIndex(const char *file, const char * date);
int write_inv_data(const string &filename, uint32_t * pInvlist, AdSearchRecord_index * padlist, uint32_t adlist_num);
int convert_struct(const AdSearchRecord_index &in, AdSearchRecord &out);
bool is_valid_pid(string bd_pid);

int main(int argc, char * argv[])
{
	if (argc < 4)
	{
		cerr << "Usage: " << argv[0] << " adlist_file testAcc_file date\n";
		return -1;
	}

	// calculating the loading time
	struct timeval tvStart,tvEnd;
	double linStart = 0,linEnd = 0,lTime = 0;
	gettimeofday (&tvStart,NULL);

	// read the test account list, and hash it
	if (load_testAcc(argv[2]) != 0)
	{
		cerr << "Can't open test account file\n";
		return -1;
	}

	// read records from log file
	if (genIndex(argv[1], argv[3]) != 0)
	{
		cerr << "genIndex failed\n";
		return -1;
	}

	// hash finished, giving the time 
	gettimeofday (&tvEnd,NULL);  
	linStart = ((double)tvStart.tv_sec * 1000000 + (double)tvStart.tv_usec);  //unit uS
	linEnd = ((double)tvEnd.tv_sec * 1000000 + (double)tvEnd.tv_usec);        //unit uS
	lTime = linEnd-linStart; 
	printf("Indexing time is %fs\n", lTime/1e6);

	delete[] ptext;
	delete[] ppid;

	return 0;
}

//trim指示是否保留空串，默认为保留
//解析字符串
vector<string> parse_string(const string& src, string tok, bool trim, string null_subst)
{
	vector<string> v;
	v.clear();
	if (src.empty())
	{
		//throw "parse_string: empty string\0";
		return v;
	}
	else if (tok.empty())
	{
		v.push_back(src);
		return v;
	}

	string::size_type pre_index = 0, index = 0, len = 0;
	while ((index = src.find_first_of(tok, pre_index)) != string::npos)
	{
		if ((len = index-pre_index)!=0)
		{
			v.push_back(src.substr(pre_index, len));
		}
		else if(trim==false)
		{
			v.push_back(null_subst);
		}
		pre_index = index+1;
	}
	string endstr = src.substr(pre_index);
	if (trim==false)
	{
		v.push_back(endstr.empty() ? null_subst : endstr);
	}
	else if (!endstr.empty())
	{
		v.push_back(endstr);
	}

	return v;
}

int load_testAcc(const char *file) // load test account list, and write to global variable hashTestAcc
{
	// read the test account list, and hash it
	ifstream in(file);
	// open test account file
	if (!in)
	{
		cerr << "Can't open test account file\n";
		return -1;
	}

	string line;
	while (getline(in, line))
	{
		uint32_t AccID = atoi(line.c_str());
		if (hashTestAcc.find(AccID) == hashTestAcc.end())
		{
			hashTestAcc[AccID] = 1;
		}
	}
	in.close();

	cerr << "hashTestAcc map size: " << hashTestAcc.size() << endl;

	return 0;
}

int genIndex(const char *file, const char * date)
{
	// 记录文本（词、pid）在相应内存位置的表
	t_uint_index_map text_index_map;
	t_uint_index_map pid_index_map;
	// 八个索引表
	t_uint_cnt_map hashQueryCnt(3000000);
	t_uint_cnt_map hashAdkeyCnt(2000000);
	t_uint_cnt_map hashAdidCnt(2000000);
	t_uint_cnt_map hashCateCnt;
	t_uint_cnt_map hashAccidCnt;
	t_string_cnt_map hashSrcCnt;
	t_string_cnt_map hashRgCnt;
	t_uint_cnt_map hashPosCnt; 

	// 1. 做正排，同时理清倒排的结构
	uint32_t adlist_num = 0; // 正排元素个数
	uint32_t validpid_adlist_num = 0; // 会去掉一些不合法pid后，这个数量会比adlist_num小
	// open log file
	ifstream in(file);
	if (!in)
	{
		return -1;
	}
	string line;
	while (getline(in, line))
	{
		adlist_num++;
	}
	cerr << "data num: " << adlist_num << endl;
	in.clear();
	in.seekg(0, ios::beg); // 读文件指标回文件头

	// 生成正排
	struct AdSearchRecord_index * padlist = new AdSearchRecord_index[adlist_num]; // 正排内存块
	size_t curr_text_pos = 0, curr_pid_pos = 0; // 上述两空间的下一个插入位置
	size_t cnt = 0;
	while (getline(in, line))
	{
		if (cnt >= adlist_num) // 防越界
		{
			cerr << "file line num mismatch ...\n";
			return -1;
		}
		if (cnt % 1000000 == 1)
			cerr << "adlist num: " << cnt << endl;

		vector<string> tokens = parse_string(line, "\t");

		md5_long_32 md5_output;
		uint32_t md5_val;

		// 插入正排
		//struct AdSearchRecord_index &record = padlist[cnt]; // 内存单元的引用
		struct AdSearchRecord_index record;
		// query
		string query(tokens[3]);
		if (query.size() >= MAX_WORD_LENGTH) // 定长截断
		{
			query = query.substr(0, MAX_WORD_LENGTH-1);
		}
		md5_output = getSign32((const char*)query.c_str(), -1);
		md5_val = md5_output.data.intData[0];
		if (text_index_map.find(md5_val) == text_index_map.end()) // 新插入
		{
			size_t len = query.size();
			if (curr_text_pos+len >= TEXT_MEM_SIZE) // 防越界
			{
				cerr << "TEXT_MEM_SIZE not enough ...\n";
				return -2;
			}
			record.query_start = curr_text_pos;
			record.query_len = len;
			strncpy(ptext+curr_text_pos, query.c_str(), len); // 插入
			HashIndex idx;
			idx.start = curr_text_pos;
			idx.len = len;
			text_index_map[md5_val] = idx;
			curr_text_pos += len; // 移到下一个可插入位置
		}
		else
		{
			record.query_start = text_index_map[md5_val].start;
			record.query_len = text_index_map[md5_val].len;
		}
		hashQueryCnt[md5_val].len++; // 倒排项数+1
		// adkey
		string adkey(tokens[11]);
		if (adkey.size() >= MAX_WORD_LENGTH) // 定长截断
		{
			adkey = adkey.substr(0, MAX_WORD_LENGTH-1);
		}
		md5_output = getSign32((const char*)adkey.c_str(), -1);
		md5_val = md5_output.data.intData[0];
		if (text_index_map.find(md5_val) == text_index_map.end()) // 新插入
		{
			size_t len = adkey.size();
			if (curr_text_pos+len >= TEXT_MEM_SIZE) // 防越界
			{
				cerr << "TEXT_MEM_SIZE not enough ...\n";
				return -2;
			}
			record.adkey_start = curr_text_pos;
			record.adkey_len = len;
			strncpy(ptext+curr_text_pos, adkey.c_str(), len); // 插入
			HashIndex idx;
			idx.start = curr_text_pos;
			idx.len = len;
			text_index_map[md5_val] = idx;
			curr_text_pos += len; // 移到下一个可插入位置
		}
		else
		{
			record.adkey_start = text_index_map[md5_val].start;
			record.adkey_len = text_index_map[md5_val].len;
		}
		hashAdkeyCnt[md5_val].len++; // 倒排项数+1
		// pid
		string pid(tokens[8]);
		if (pid.size() >= MAX_PID_LENGTH) // 定长截断
		{
			pid = pid.substr(0, MAX_PID_LENGTH-1);
		}
		md5_output = getSign32((const char*)pid.c_str(), -1);
		md5_val = md5_output.data.intData[0];
		if (pid_index_map.find(md5_val) == pid_index_map.end()) // 新插入
		{
			size_t len = pid.size();
			if (curr_pid_pos+len >= PID_MEM_SIZE) // 防越界
			{
				cerr << "PID_MEM_SIZE not enough ...\n";
				return -3;
			}
			record.pid_start = curr_pid_pos;
			record.pid_len = len;
			strncpy(ppid+curr_pid_pos, pid.c_str(), len); // 插入
			HashIndex idx;
			idx.start = curr_pid_pos;
			idx.len = len;
			pid_index_map[md5_val] = idx;
			curr_pid_pos += len; // 移到下一个可插入位置
		}
		else
		{
			record.pid_start = pid_index_map[md5_val].start;
			record.pid_len = pid_index_map[md5_val].len;
		}
		if (is_valid_pid(pid)) // 不合法的pid没有倒排
		{
			hashSrcCnt[pid].len++; // 倒排项数+1
			validpid_adlist_num++;
		}
		// adid
		record.adid = atoi(tokens[4].c_str());
		hashAdidCnt[record.adid].len++; // 倒排项数+1
		// accid
		record.accid = atoi(tokens[7].c_str());
		hashAccidCnt[record.accid].len++; // 倒排项数+1
		// cate
		record.cate = atoi(tokens[10].c_str());
		hashCateCnt[record.cate].len++; // 倒排项数+1
		// pos
		record.pos = atoi(tokens[6].c_str());
		hashPosCnt[record.pos].len++; // 倒排项数+1
		// region
		if (tokens[5] == "左侧")
			record.region = 0;
		else if (tokens[5] == "小兰条")
			record.region = 1;
		else
			record.region = 2;
		hashRgCnt[tokens[5]].len++; // 倒排项数+1

		if (tokens[9] == "sogou")
			record.src4 = 0;
		else if (tokens[9] == "sohu")
			record.src4 = 1;
		else if (tokens[9] == "sogou-*")
			record.src4 = 2;
		else
			record.src4 = 3;
		if (hashTestAcc.find(record.accid) != hashTestAcc.end())
			record.isTest = 1;
		else
			record.isTest = 0;

		record.nPv = atoi(tokens[0].c_str());
		record.nClick = atoi(tokens[1].c_str());
		if (string("na") == tokens[FEATURE_DIM+3])
			record.price = -1;
		else
			record.price = atoi(tokens[FEATURE_DIM+3].c_str());

		padlist[cnt] = record;
		cnt++;
	}
	in.close();
	const size_t ptext_size = curr_text_pos;
	const size_t ppid_size = curr_pid_pos;
	cerr << "validpid_adlist_num: " << validpid_adlist_num << endl;
	cerr << "text mem actually size: " << ptext_size << endl;
	cerr << "pid mem actually size: " << ppid_size << endl;

	// 2. 整理倒排结构，输出倒排的索引*.index文件
	string IndexFileName;
	ofstream pIndex;
	t_uint_cnt_map::iterator it_uint;
	t_string_cnt_map::iterator it_str;
	size_t curr_pos;
	uint32_t level_start[LEVEL_NUM];
	uint32_t level_len[LEVEL_NUM];

	// query
	IndexFileName = string("log/") + date + string("/query.index");
	pIndex.open(IndexFileName.c_str());
	if (!pIndex)
	{
		std::cerr << "can't create " << IndexFileName << endl;
		return -1;
	}
	cerr << "writing " << IndexFileName << " ..." << endl;
	curr_pos = 0;
	for (it_uint = hashQueryCnt.begin(); it_uint != hashQueryCnt.end(); ++it_uint)
	{
		it_uint->second.start = curr_pos;
		it_uint->second.curr = 0;
		curr_pos += it_uint->second.len;
		// output
		if (text_index_map.find(it_uint->first) == text_index_map.end())
		{
			cerr << "text not in memory ...\n";
			return -5;
		}
		HashIndex idx = text_index_map[it_uint->first];
		if (idx.start+idx.len > ptext_size)
		{
			cerr << "text mem overflow ...\n";
			return -5;
		}
		if (idx.len > MAX_WORD_LENGTH-1)
		{
			cerr << "single text too long ...\n";
			return -5;
		}
		char text[MAX_WORD_LENGTH];
		memset(text, 0, MAX_WORD_LENGTH);
		strncpy(text, ptext+idx.start, idx.len);
		pIndex << text << "\t" << it_uint->second.start << "\t" << it_uint->second.len << endl;
	}
	pIndex.close();
	if (curr_pos != adlist_num) // 所有项的和为数据文件行数
	{
		cerr << "[query] adlist num mismatch ...\n";
		return -4;
	}

	// adid
	IndexFileName = string("log/") + date + string("/adid.index");
	pIndex.open(IndexFileName.c_str());
	if (!pIndex)
	{
		std::cerr << "can't create " << IndexFileName << endl;
		return -1;
	}
	cerr << "writing " << IndexFileName << " ..." << endl;
	curr_pos = 0;
	for (it_uint = hashAdidCnt.begin(); it_uint != hashAdidCnt.end(); ++it_uint)
	{
		it_uint->second.start = curr_pos;
		it_uint->second.curr = 0;
		curr_pos += it_uint->second.len;
		pIndex << it_uint->first << "\t" << it_uint->second.start << "\t" << it_uint->second.len << endl;
	}
	pIndex.close();
	if (curr_pos != adlist_num) // 所有项的和为数据文件行数
	{
		cerr << "[adid] adlist num mismatch ...\n";
		return -4;
	}

	// pos
	IndexFileName = string("log/") + date + string("/pos.index");
	pIndex.open(IndexFileName.c_str());
	if (!pIndex)
	{
		std::cerr << "can't create " << IndexFileName << endl;
		return -1;
	}
	cerr << "writing " << IndexFileName << " ..." << endl;
	curr_pos = 0;
	for (it_uint = hashPosCnt.begin(); it_uint != hashPosCnt.end(); ++it_uint)
	{
		it_uint->second.start = curr_pos;
		it_uint->second.curr = 0;
		curr_pos += it_uint->second.len;
		pIndex << it_uint->first << "\t" << it_uint->second.start << "\t" << it_uint->second.len << endl;
	}
	pIndex.close();
	if (curr_pos != adlist_num) // 所有项的和为数据文件行数
	{
		cerr << "[pos] adlist num mismatch ...\n";
		return -4;
	}

	// accid
	IndexFileName = string("log/") + date + string("/accid.index");
	pIndex.open(IndexFileName.c_str());
	if (!pIndex)
	{
		std::cerr << "can't create " << IndexFileName << endl;
		return -1;
	}
	cerr << "writing " << IndexFileName << " ..." << endl;
	curr_pos = 0;
	for (it_uint = hashAccidCnt.begin(); it_uint != hashAccidCnt.end(); ++it_uint)
	{
		it_uint->second.start = curr_pos;
		it_uint->second.curr = 0;
		curr_pos += it_uint->second.len;
		pIndex << it_uint->first << "\t" << it_uint->second.start << "\t" << it_uint->second.len << endl;
	}
	pIndex.close();
	if (curr_pos != adlist_num) // 所有项的和为数据文件行数
	{
		cerr << "[accid] adlist num mismatch ...\n";
		return -4;
	}

	// cate: 对各级分类号作索引，但不新增冗余数据，故对hash key排序
	IndexFileName = string("log/") + date + string("/cate.index");
	pIndex.open(IndexFileName.c_str());
	if (!pIndex)
	{
		std::cerr << "can't create " << IndexFileName << endl;
		return -1;
	}
	cerr << "writing " << IndexFileName << " ..." << endl;
	curr_pos = 0;
	vector<uint32_t> cate_list; // 用于排序
	for (it_uint = hashCateCnt.begin(); it_uint != hashCateCnt.end(); ++it_uint)
	{
		uint32_t cate = it_uint->first;
		cate_list.push_back(cate);
	}
	sort(cate_list.begin(), cate_list.end());
	// start building index ...
	uint32_t curr_cate;
	uint32_t pre_cate = cate_list[0];
	for (size_t ii=0; ii<LEVEL_NUM; ii++)
	{
		level_start[ii] = curr_pos;
		level_len[ii] = 0;
	}
	size_t denum[2] = {100,10000};
	for (size_t ii=  0; ii<cate_list.size(); ii++)
	{
		uint32_t cate = cate_list[ii];
		it_uint = hashCateCnt.find(cate);
		if (it_uint == hashCateCnt.end())
			continue; // 理论上不会执行
		it_uint->second.start = curr_pos;
		it_uint->second.curr = 0;
		curr_cate = cate_list[ii];
		for (size_t jj=0; jj<2; jj++)
		{
			if (curr_cate/denum[jj] == pre_cate/denum[jj])
			{
				level_len[jj] += it_uint->second.len;
			}
			else
			{
				pIndex << pre_cate/denum[jj] << "\t" << level_start[jj] << "\t" << level_len[jj] << endl;
				level_start[jj] = curr_pos;
				level_len[jj] = it_uint->second.len;
			}
		}
		// 当前分类号
		pIndex << it_uint->first << "\t" << curr_pos << "\t" << it_uint->second.len << endl;
		curr_pos += it_uint->second.len;
		pre_cate = curr_cate;
	}
	for (size_t jj=0; jj<2; jj++)
	{
		pIndex << pre_cate/denum[jj] << "\t" << level_start[jj] << "\t" << level_len[jj] << endl;
	}
	pIndex.close();
	if (curr_pos != adlist_num) // 所有项的和为数据文件行数
	{
		cerr << "[cate] adlist num mismatch ...\n";
		return -4;
	}

	// adkey
	IndexFileName = string("log/") + date + string("/adkey.index");
	pIndex.open(IndexFileName.c_str());
	if (!pIndex)
	{
		std::cerr << "can't create " << IndexFileName << endl;
		return -1;
	}
	cerr << "writing " << IndexFileName << " ..." << endl;
	curr_pos = 0;
	for (it_uint = hashAdkeyCnt.begin(); it_uint != hashAdkeyCnt.end(); ++it_uint)
	{
		it_uint->second.start = curr_pos;
		it_uint->second.curr = 0;
		curr_pos += it_uint->second.len;
		// output
		if (text_index_map.find(it_uint->first) == text_index_map.end())
		{
			cerr << "text not in memory ...\n";
			return -5;
		}
		HashIndex idx = text_index_map[it_uint->first];
		if (idx.start+idx.len > ptext_size)
		{
			cerr << "text mem overflow ...\n";
			return -5;
		}
		if (idx.len > MAX_WORD_LENGTH-1)
		{
			cerr << "single text too long ...\n";
			return -5;
		}
		char text[MAX_WORD_LENGTH];
		memset(text, 0, MAX_WORD_LENGTH);
		strncpy(text, ptext+idx.start, idx.len);
		pIndex << text << "\t" << it_uint->second.start << "\t" << it_uint->second.len << endl;
	}
	pIndex.close();
	if (curr_pos != adlist_num) // 所有项的和为数据文件行数
	{
		cerr << "[adkey] adlist num mismatch ...\n";
		return -4;
	}

	// region
	IndexFileName = string("log/") + date + string("/region.index");
	pIndex.open(IndexFileName.c_str());
	if (!pIndex)
	{
		std::cerr << "can't create " << IndexFileName << endl;
		return -1;
	}
	cerr << "writing " << IndexFileName << " ..." << endl;
	curr_pos = 0;
	for (it_str = hashRgCnt.begin(); it_str != hashRgCnt.end(); ++it_str)
	{
		it_str->second.start = curr_pos;
		it_str->second.curr = 0;
		curr_pos += it_str->second.len;
		pIndex << it_str->first << "\t" << it_str->second.start << "\t" << it_str->second.len << endl;
	}
	pIndex.close();
	if (curr_pos != adlist_num) // 所有项的和为数据文件行数
	{
		cerr << "[region] adlist num mismatch ...\n";
		return -4;
	}

	// src(pid): 对各级pid作索引，但不新增冗余数据，故对hash key排序
	IndexFileName = string("log/") + date + string("/src.index");
	pIndex.open(IndexFileName.c_str());
	if (!pIndex)
	{
		std::cerr << "can't create " << IndexFileName << endl;
		return -1;
	}
	cerr << "writing " << IndexFileName << " ..." << endl;
	curr_pos = 0;
	// 先给非sogou-的部分建索引，即sohu, sogou, sogou__free, sohu__free, bd__free，同时记录合法的sogou-xxx，并进行排序
	vector<string> bd_pid_list;
	for (it_str = hashSrcCnt.begin(); it_str != hashSrcCnt.end(); ++it_str) // 倒排hash中没有不合法的Pid
	{
		//if (it_str->first.size() < 6 || it_str->first.substr(0,6) != "sogou-")
		if (strstr(it_str->first.c_str(), "sogou-") == NULL) // 非sogou-xxx
		{
			pIndex << it_str->first << "\t" << curr_pos << "\t" << it_str->second.len << endl;
			it_str->second.start = curr_pos;
			it_str->second.curr = 0;
			curr_pos += it_str->second.len;
		}
		else //if (is_valid_bd_pid(it_str->first)) //去掉打错的bd pid
		{
			string pid = it_str->first;
			bd_pid_list.push_back(pid);
		}
	}
	sort(bd_pid_list.begin(), bd_pid_list.end());
	// 再给sogou-xxx建索引
	string pre_pid = bd_pid_list[0];
	string curr_pid;
	for (size_t ii=0; ii<LEVEL_NUM; ii++)
	{
		level_start[ii] = curr_pos;
		level_len[ii] = 0;
	}
	size_t pid_level_pos[LEVEL_NUM] = {6,11,28};
	for (size_t ii = 0; ii<bd_pid_list.size(); ii++)
	{
		it_str = hashSrcCnt.find(bd_pid_list[ii]);
		if (it_str == hashSrcCnt.end())
			continue; // 理论上不会执行
		it_str->second.start = curr_pos;
		it_str->second.curr = 0;
		// mulit-level
		curr_pid = it_str->first;
		size_t lvl = (curr_pid.size()==27 && pre_pid.size()==27) ? 2 : 3;
		for (size_t jj=0; jj<lvl; jj++)
		{
			if (curr_pid.substr(0,pid_level_pos[jj]) == pre_pid.substr(0,pid_level_pos[jj]))
			{
				level_len[jj] += it_str->second.len;
			}
			else
			{
				if (jj!=2 || pre_pid.size() == 32) // 对于第三级，前一个有三级时才需要写
				{
					pIndex << pre_pid.substr(0,pid_level_pos[jj]) << "\t" << level_start[jj] << "\t" << level_len[jj] << endl;
				}
				if (jj!=2 || curr_pid.size() == 32) // 对于第三级，当前一个有三级时才需要设置初始值
				{
					level_start[jj] = curr_pos;
					level_len[jj] = it_str->second.len;
				}
			}
		}
		// 当前小pid
		pIndex << it_str->first << "\t" << curr_pos << "\t" << it_str->second.len << endl;
		curr_pos += it_str->second.len;
		pre_pid = curr_pid;
	}
	size_t lvl = curr_pid.size()==27 ? 2 : 3;
	for (size_t jj=0; jj<lvl; jj++)
	{
		pIndex << pre_pid.substr(0,pid_level_pos[jj]) << "\t" << level_start[jj] << "\t" << level_len[jj] << endl;
	}
	pIndex.close();
	if (curr_pos != validpid_adlist_num) // 因为有pid乱码会被过滤掉, 这个数要和过滤后的量一致
	{
		cerr << "[src] adlist num mismatch ...\n";
		return -4;
	}

	// 3. 再过一遍正排，填充倒排数据
	// 申请倒排空间
	uint32_t * pQueryInvlist = new uint32_t[adlist_num];
	uint32_t * pAdidInvlist = new uint32_t[adlist_num];
	uint32_t * pAccidInvlist = new uint32_t[adlist_num];
	uint32_t * pPosInvlist = new uint32_t[adlist_num];
	uint32_t * pRgInvlist = new uint32_t[adlist_num];
	uint32_t * pAdkeyInvlist = new uint32_t[adlist_num];
	uint32_t * pCatelist = new uint32_t[adlist_num];
	uint32_t * pSrcInvlist = new uint32_t[validpid_adlist_num]; // 这个数量可能会小，因为去掉一些不合法pid

	// 过一遍正排，将正排编号填入倒排中
	cerr << "scan adlist to assign each ad to a inv-list ...\n";
	for (size_t ii=0; ii < adlist_num; ii++)
	{
		AdSearchRecord_index record = padlist[ii]; // 结构赋值，都是数值，可以直接赋
		char text[MAX_WORD_LENGTH]; 
		md5_long_32 md5_output;
		uint32_t md5_val;
		string key;

		// query
		if (record.query_len > MAX_WORD_LENGTH-1) // 防越界
		{
			cerr << "adlist: single word too long ...\n";
			return -6;
		}
		memset(text, 0, MAX_WORD_LENGTH);
		strncpy(text, ptext+record.query_start, record.query_len);
		md5_output = getSign32((const char*)text, -1);
		md5_val = md5_output.data.intData[0];
		it_uint = hashQueryCnt.find(md5_val);
		if (it_uint == hashQueryCnt.end())
		{
			cerr << "hash term not found ...\n";
			return -6;
		}
		pQueryInvlist[it_uint->second.start+it_uint->second.curr] = ii;
		it_uint->second.curr++;
		// adkey
		if (record.adkey_len > MAX_WORD_LENGTH-1) // 防越界
		{
			cerr << "adlist: single word too long ...\n";
			return -6;
		}
		memset(text, 0, MAX_WORD_LENGTH);
		strncpy(text, ptext+record.adkey_start, record.adkey_len);
		md5_output = getSign32((const char*)text, -1);
		md5_val = md5_output.data.intData[0];
		it_uint = hashAdkeyCnt.find(md5_val);
		if (it_uint == hashAdkeyCnt.end())
		{
			cerr << "hash term not found ...\n";
			return -6;
		}
		pAdkeyInvlist[it_uint->second.start+it_uint->second.curr] = ii;
		it_uint->second.curr++;
		// src
		if (record.pid_len > MAX_PID_LENGTH-1) // 防越界
		{
			cerr << "adlist: single pid too long ...\n";
			return -6;
		}
		memset(text, 0, MAX_WORD_LENGTH);
		strncpy(text, ppid+record.pid_start, record.pid_len);
		if (is_valid_pid(text))
		{
			it_str = hashSrcCnt.find(text);
			if (it_str == hashSrcCnt.end())
			{
				cerr << "hash term not found ...\n";
				return -6;
			}
			pSrcInvlist[it_str->second.start+it_str->second.curr] = ii;
			it_str->second.curr++;
		}
		// adid
		it_uint = hashAdidCnt.find(record.adid);
		if (it_uint == hashAdidCnt.end())
		{
			cerr << "hash term not found ...\n";
			return -6;
		}
		pAdidInvlist[it_uint->second.start+it_uint->second.curr] = ii;
		it_uint->second.curr++;
		// accid
		it_uint = hashAccidCnt.find(record.accid);
		if (it_uint == hashAccidCnt.end())
		{
			cerr << "hash term not found ...\n";
			return -6;
		}
		pAccidInvlist[it_uint->second.start+it_uint->second.curr] = ii;
		it_uint->second.curr++;
		// pos
		it_uint = hashPosCnt.find(record.pos);
		if (it_uint == hashPosCnt.end())
		{
			cerr << "hash term not found ...\n";
			return -6;
		}
		pPosInvlist[it_uint->second.start+it_uint->second.curr] = ii;
		it_uint->second.curr++;
		// cate
		it_uint = hashCateCnt.find(record.cate);
		if (it_uint == hashCateCnt.end())
		{
			cerr << "hash term not found ...\n";
			return -6;
		}
		pCatelist[it_uint->second.start+it_uint->second.curr] = ii;
		it_uint->second.curr++;
		// region
		switch (record.region)
		{
			case 0: key = "左侧"; break;
			case 1: key = "小兰条"; break;
			default: key = "右侧";
		}
		it_str = hashRgCnt.find(key);
		if (it_str == hashRgCnt.end())
		{
			cerr << "hash term not found ...\n";
			return -6;
		}
		pRgInvlist[it_str->second.start+it_str->second.curr] = ii;
		it_str->second.curr++;
	}

	// 4. 输出倒排数据文件, *.dat
	cerr << "output *.dat ...\n";
	string DatafilePrefix = string("log/") + date + string("/");
	if (write_inv_data(DatafilePrefix+"query.dat", pQueryInvlist, padlist, adlist_num) != 0)
	{
		cerr << "write query.dat failed\n";
		return -1;
	}
	if (write_inv_data(DatafilePrefix+"adid.dat", pAdidInvlist, padlist, adlist_num) != 0)
	{
		cerr << "write query.dat failed\n";
		return -1;
	}
	if (write_inv_data(DatafilePrefix+"pos.dat", pPosInvlist, padlist, adlist_num) != 0)
	{
		cerr << "write query.dat failed\n";
		return -1;
	}
	if (write_inv_data(DatafilePrefix+"region.dat", pRgInvlist, padlist, adlist_num) != 0)
	{
		cerr << "write query.dat failed\n";
		return -1;
	}
	if (write_inv_data(DatafilePrefix+"accid.dat", pAccidInvlist, padlist, adlist_num) != 0)
	{
		cerr << "write query.dat failed\n";
		return -1;
	}
	if (write_inv_data(DatafilePrefix+"src.dat", pSrcInvlist, padlist, validpid_adlist_num) != 0)
	{
		cerr << "write query.dat failed\n";
		return -1;
	}
	if (write_inv_data(DatafilePrefix+"cate.dat", pCatelist, padlist, adlist_num) != 0)
	{
		cerr << "write query.dat failed\n";
		return -1;
	}
	if (write_inv_data(DatafilePrefix+"adkey.dat", pAdkeyInvlist, padlist, adlist_num) != 0)
	{
		cerr << "write query.dat failed\n";
		return -1;
	}

	// 释放正排空间
	delete[] padlist;
	// 释放倒排空间
	delete[] pQueryInvlist;
	delete[] pAdidInvlist;
	delete[] pAccidInvlist;
	delete[] pPosInvlist;
	delete[] pSrcInvlist;
	delete[] pRgInvlist;
	delete[] pCatelist;
	delete[] pAdkeyInvlist;

	cerr << "building index complete!\n";
	cerr << "hashQueryCnt size: " << hashQueryCnt.size() << endl;
	cerr << "hashAdidCnt size: " << hashAdidCnt.size() << endl;
	cerr << "hashRgCnt size: " << hashRgCnt.size() << endl;
	cerr << "hashAccidCnt size: " << hashAccidCnt.size() << endl;
	cerr << "hashPosCnt size: " << hashPosCnt.size() << endl;
	cerr << "hashSrcCnt size: " << hashSrcCnt.size() << endl;
	cerr << "hashCateCnt size: " << hashCateCnt.size() << endl;
	cerr << "hashAdkeyCnt size: " << hashAdkeyCnt.size() << endl;
	
	return 0;
}

bool is_valid_pid(string pid)
{
	if (strstr(pid.c_str(), "sogou-") == NULL) // 不是bd
		return true;
	else // bd要符合一定格式
	{
		if (pid.size() == 32 && pid[5] == '-' && pid[10] == '-' && pid[27] == '-')
			return true;
		if (pid.size() == 27 && pid[5] == '-' && pid[10] == '-')
			return true;
		return false;
	}
}

int write_inv_data(const string &filename, uint32_t * pInvlist, AdSearchRecord_index * padlist, uint32_t adlist_num)
{
	ofstream pData(filename.c_str(), ios::binary);
	if (!pData)
	{
		std::cerr << "can't create " << pData << endl;
		return -1;
	}

	for (uint32_t ii=0; ii<adlist_num; ii++)
	{
		AdSearchRecord tmp;
		AdSearchRecord_index in = padlist[pInvlist[ii]];
		convert_struct(in, tmp);
		pData.write((char *)(&tmp), sizeof(AdSearchRecord));
	}
	pData.close();
	return 0;
}

int convert_struct(const AdSearchRecord_index &in, AdSearchRecord &out)
{
	// 此处有数组越界隐患，设计时在函数外保证
	strncpy(out.query, ptext+in.query_start, in.query_len);
	out.query[in.query_len] = '\0';
	out.adid = in.adid;
	string tmp;
	switch (in.region)
	{
		case 0: tmp = "左侧"; break;
		case 1: tmp = "小兰条"; break;
		case 2: tmp = "右侧"; break;
		default: tmp = "右侧";
	}
	strcpy(out.region, tmp.c_str());
	out.pos = in.pos;
	out.accid = in.accid;
	strncpy(out.src, ppid+in.pid_start, in.pid_len);
	out.src[in.pid_len] = '\0';
	switch (in.src4)
	{
		case 0: tmp = "sogou"; break;
		case 1: tmp = "sohu"; break;
		case 2: tmp = "sogou-*"; break;
		default: tmp = "others";
	}
	strcpy(out.src4, tmp.c_str());
	out.cate = in.cate;
	strncpy(out.adkey, ptext+in.adkey_start, in.adkey_len);
	out.adkey[in.adkey_len] = '\0';
	out.isTest = in.isTest;
	out.nPv = in.nPv;
	out.nClick = in.nClick;
	out.price = in.price;

	return 0;
}
