#include <string>
#include <vector>
#include <ext/hash_map>

#ifdef __GNUC__ 
        #if __GNUC__ < 3
                #include <hash_map.h>
                namespace Sgi { using ::hash_map; }; // inherit globals 
        #else
                #include <ext/hash_map>
                #if __GNUC_MINOR__ == 0 
                        namespace Sgi = std;               // GCC 3.0 
		#else
			namespace Sgi = ::__gnu_cxx;       // GCC 3.1 and later 
		#endif
	#endif
#else      // ...  there are other compilers, right?
	namespace Sgi = std;
#endif

#define FEATURE_DIM 9
// 0 -- query
// 1 -- adid
// 2 -- region
// 3 -- pos
// 4 -- acc id
// 5 -- src small
// 6 -- src big (4 PIDs)
// 7 -- category: NOTE: there's a special: cate_not_found
// 8 -- ad key
#define MAX_RAW_RECORD_LENGTH 1024
#define MAX_KEYWORD_LENGTH 30
#define MAX_STRING_LENGTH 255
#define TOP_DISPLAY 100
#define PORT 1983 // socket listen port
#define MAX_CONN 10 // max connection number
#define BUF_LEN 8192 // buf length
#define MAX_DAY 30 // 最长存三十天的数据
const size_t MAX_WORD_LENGTH = 64; // 单个查询词，adkey等文本最大长度
const size_t MAX_PID_LENGTH = 40; // 单个pid最大长度，合法最大长度为32
const unsigned int INFINTE = 1000000000;
const size_t LEVEL_NUM = 5;
// for index
const size_t TEXT_MEM_SIZE = 200000000; // 设一天uniq 1000w词，平均每个词十个字
const size_t PID_MEM_SIZE = 400000; // 存pid的内存，一共10000个pid，每个最长32

// ad structure
struct AdSearchRecord
{
	char query[MAX_WORD_LENGTH];
	unsigned int adid;
	char region[8];
	char pos;
	unsigned int accid;
	char src[MAX_PID_LENGTH];
	char src4[8];
	unsigned int cate; // NOTE: 0 means: cate_not_found
	char adkey[MAX_WORD_LENGTH];

	char isTest; // 0: 不是测试，1: 是测试帐号

	int nPv;    // number of pvs
	int nClick; // number of clicks
	//float ctr;  // CTR

	int price; // price
};

// 上面结构用于索引的版本
struct AdSearchRecord_index
{
	// 所有字符串存在一块内存，记录记点和终点
	uint32_t query_start;
	uint32_t query_len;
	uint32_t adkey_start;
	uint32_t adkey_len;
	uint32_t pid_start;
	uint32_t pid_len;

	uint32_t adid;
	uint32_t accid;
	uint32_t cate; // NOTE: 0 means: cate_not_found
	uint8_t region;
	uint8_t pos;
	uint8_t src4;

	uint8_t isTest; // 0: 不是测试，1: 是测试帐号

	uint32_t nPv;    // number of pvs
	uint32_t nClick; // number of clicks

	int16_t price; // price
};

// 记录索引数据的结构，过第一遍文件时生成索引
struct HashCnt
{
	uint32_t len; // size of this term
	uint32_t start; // start position
	uint32_t curr; // current insert location
};

// 记录索引结构，含起点及大小
struct HashIndex
{
	unsigned int start;//该项在文件起始位置
	unsigned int len;//访项含有记录条数
};

struct HashIndexNode // 用于链式结构的索引，减少类似查询词这样的key产生的巨大内存浪费
{
	char N; // 取值0~MAX_DAY，表示一个日期
	unsigned int start;
	unsigned int len;
	HashIndexNode * next; // 下一个
};

// display structure
struct AdDisplay
{
	std::string key; // display key
	int nPv;    // number of pvs
	int nClick; // number of clicks
	float ctr;  // CTR = nclick/nPv
	int cost; // NOTE: display is cost = price*click
};

struct str_hash
{   
	size_t operator()(const std::string& str) const
	{   
		return Sgi::__stl_hash_string(str.c_str());
	}   
};  

// indexing
typedef Sgi::hash_map<uint32_t, char> HashTestAccType;
//typedef Sgi::hash_map<uint32_t, AdSearchRecord> t_id_record_map;
//typedef Sgi::hash_map<uint32_t, AdSearchRecord_index> t_id_record_map;
//typedef Sgi::hash_map<std::string, std::vector<uint32_t>, str_hash> t_string_list_map;
//typedef Sgi::hash_map<uint32_t, std::vector<uint32_t> > t_uint_list_map;
typedef Sgi::hash_map<std::string, HashCnt, str_hash> t_string_cnt_map;
typedef Sgi::hash_map<uint32_t, HashCnt> t_uint_cnt_map;
typedef Sgi::hash_map<std::string, HashIndex, str_hash> t_string_index_map;
typedef Sgi::hash_map<uint32_t, HashIndex> t_uint_index_map;

typedef Sgi::hash_map<std::string, struct HashCnt, str_hash> HashStringCntType;
typedef Sgi::hash_map<unsigned int, struct HashCnt> HashIntCntType;
typedef Sgi::hash_map<std::string, struct HashIndex *, str_hash> HashStringIndexType;
typedef Sgi::hash_map<std::string, struct HashIndexNode *, str_hash> HashStringIndexListType;
typedef Sgi::hash_map<uint32_t, struct HashIndexNode *> HashMD5IndexListType;
typedef Sgi::hash_map<unsigned int, struct HashIndex *> HashIntIndexType;
typedef Sgi::hash_map<std::string, struct AdDisplay, str_hash> HashDisplayType;
typedef Sgi::hash_map<std::string, uint8_t, str_hash> t_string_map;
