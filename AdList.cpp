#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ext/hash_map>

using namespace std;

struct str_hash
{   
	size_t operator()(const std::string& str) const
	{   
		return __gnu_cxx::__stl_hash_string(str.c_str());
	}   
};

typedef __gnu_cxx::hash_map<uint32_t, uint32_t> t_uint_uint_map;
typedef __gnu_cxx::hash_map<string, uint32_t, str_hash> t_string_uint_map;

vector<string> parse_string(const string& src, string tok, bool trim=false, string null_subst="");

int main(int argc, char * argv[])
{
	if (argc < 2)
	{
		cerr << "Usage: " << argv[0] << " date\n";
		return -1;
	}
	string date(argv[1]);
	string keyad("log/"+date+"/valid_keyad");
	string cd_log("log/"+date+"/click_log");
	string ie_log("log/"+date+"/pv_log");
	string outfile("log/"+date+"/adlist");

	// 不统计的pid
	const string pid_black("sogou-tjsank");

	ifstream in;
	string line;
	size_t cnt;

	// Adid to catalog Hashmap
	t_uint_uint_map adid_cate_map;
	cerr <<  "load ad category from valid_keyad ...\n";
	in.open(keyad.c_str());
	if (!in)
	{
		cerr << "can't open keyad\n";
		return -1;
	}
	while (getline(in, line))
	{
		vector<string> tmp = parse_string(line, "");
		uint32_t adid = atoi(tmp[0].c_str());
		uint32_t cate = atoi(tmp[2].c_str());
		adid_cate_map[adid] = cate;
	}
	in.close();
	cerr << "adid_cate_map size: " << adid_cate_map.size() << endl;

	// pv record
	t_string_uint_map sample_pv_map;
	cerr << "loading pv records ...\n";
	in.open(ie_log.c_str());
	if (!in)
	{
		cerr << "can't open ie_log\n";
		return -1;
	}
	cnt = 0;
	while (getline(in, line))
	{
		if (cnt%100000 == 0)
		{
			cerr << "current processing: " << cnt << endl;
		}
		cnt++;
		vector<string> tmp = parse_string(line, "\t");
		if (tmp.size() < 13)
			continue;
		// 过滤pid
		if (tmp[2] == pid_black)
			continue;
		string query = tmp[12];
		// 小的PID
		string PID = tmp[2];
		string PID4;
		// 大的PID，分为sohu, sogou, sogou-*, others
		if (PID == "sohu")
		{
			PID4 = "sohu";
		}
		else if (PID == "sogou")
		{
			PID4 = "sogou";
		}
		else if (PID.substr(0,6) == "sogou-")
		{
			PID4 = "sogou-*";
		}
		else
		{
			PID4 = "others";
		}
		vector<string> adid_list = parse_string(tmp[5], ",");
		vector<string> accountid_list = parse_string(tmp[6], ",");
		vector<string> flag_list = parse_string(tmp[7], ",");
		vector<string> reserved_list = parse_string(tmp[8], ",");
		vector<string> adkey_list = parse_string(tmp[9], ",");
		for (size_t i=0; i < adid_list.size(); i++)
		{
			int pv_pos = 0;
			int page_num = 0;
			string Region;
			int flag = atoi(flag_list[i].c_str());
			int resv = atoi(reserved_list[i].c_str());
			char tmp_str[32];
			// 广告位置
			int flg = (flag>>4) & 7;
			if (flg == 6) // 小兰条
			{
				Region = "小兰条";
				pv_pos = ((resv>>1)&15)+1; // 广告位置 
			}
			/*else if (flg == 5 && resv%2 == 0 ) // right
			{
				Region = "右侧";
				page_num = (resv>>5)&3;
				pv_pos = page_num * 8 + ((resv>>1)&15)+1; // 右侧广告位置 
			}*/// 暂时不计算右侧
			else if (flg == 5 && resv%2 == 1 ) // left 
			{
				Region = "左侧";
				page_num = (resv>>5)&3;
				pv_pos = page_num * 8 + ((resv>>1)&15)+1; // 左侧广告位置 
			}
			else // 忽略除了以上三类的广告
			{
				continue;
			}

			// 获得广告分类                   
			string cate;
			uint32_t tmp_adid = atoi(adid_list[i].c_str());
			if (adid_cate_map.find(tmp_adid) != adid_cate_map.end())
			{
				uint32_t c = adid_cate_map[tmp_adid];
				sprintf(tmp_str, "%u", c);
				cate = tmp_str;
			}
			else
			{
				cate = "cate_not_found";
			}

			// hash the record
			string pv_sample = query + "\t" + adid_list[i] + "\t" + Region + "\t";
			sprintf(tmp_str, "%d", pv_pos);
			pv_sample += tmp_str;
			pv_sample += "\t" + accountid_list[i] + "\t" + PID + "\t" + PID4 + "\t" + cate + "\t" + adkey_list[i];
			sample_pv_map[pv_sample]++;
		}
	}
	in.close();

	// click record
	t_string_uint_map sample_click_map;
	t_string_uint_map sample_price_map;
	cerr << "loading click records ...\n";
	in.open(cd_log.c_str());
	if (!in)
	{
		cerr << "can't open cd_log\n";
		return -1;
	}
	cnt = 0;
	while (getline(in, line))
	{
		if (cnt%10000 == 0)
		{
			cerr << "current processing: " << cnt << endl;
		}
		cnt++;
		vector<string> tmp = parse_string(line, "\t");
		if (tmp.size() < 15)
			continue;

		if (tmp[12] != "0")
			continue;

		// 过滤某些pid
		if (tmp[2] == pid_black)
			continue;

		// 小的PID
		string PID = tmp[2];
		string PID4;
		// 大的PID，分为sohu, sogou, sogou-*, others
		if (PID == "sohu")
		{
			PID4 = "sohu";
		}
		else if (PID == "sogou")
		{
			PID4 = "sogou";
		}
		else if (PID.substr(0,6) == "sogou-")
		{
			PID4 = "sogou-*";
		}
		else
		{
			PID4 = "others";
		}

		int click_pos = 0;
		int page_num = 0;
		string Region;
		int flag = atoi(tmp[7].c_str());
		int resv = atoi(tmp[8].c_str());
		char tmp_str[32];
		// 广告位置
		int flg = (flag>>4) & 7;
		if (flg == 6) // 小兰条
		{
			Region = "小兰条";
			click_pos = ((resv>>1)&15)+1; // 广告位置 
		}
		/*else if (flg == 5 && resv%2 == 0 ) // right
		{
			Region = "右侧";
			page_num = (resv>>5)&3;
			click_pos = page_num * 8 + ((resv>>1)&15)+1; // 右侧广告位置 
		}*///
		else if (flg == 5 && resv%2 == 1 ) // left 
		{
			Region = "左侧";
			page_num = (resv>>5)&3;
			click_pos = page_num * 8 + ((resv>>1)&15)+1; // 左侧广告位置 
		}
		else // 忽略除了以上三类的广告
		{
			continue;
		}

		// 获得广告分类                   
		string cate;
		uint32_t tmp_adid = atoi(tmp[5].c_str());
		if (adid_cate_map.find(tmp_adid) != adid_cate_map.end())
		{
			uint32_t c = adid_cate_map[tmp_adid];
			sprintf(tmp_str, "%u", c);
			cate = tmp_str;
		}
		else
		{
			cate = "cate_not_found";
		}

		// hash the record
		string query = tmp[13];
		// 将query的大写转小写
		for (size_t i=0; i<query.size(); ++i)
			query[i] = tolower(query[i]);

		string click_sample = query + "\t" + tmp[5] + "\t" + Region + "\t";
		sprintf(tmp_str, "%d", click_pos);
		click_sample += tmp_str;
		click_sample += "\t" + tmp[6] + "\t" + PID + "\t" + PID4 + "\t" + cate + "\t" + tmp[9];
		if (sample_pv_map.find(click_sample) != sample_pv_map.end())
		{
			sample_click_map[click_sample]++;
		}
		else 
		{
			sample_pv_map[click_sample]++;
			sample_click_map[click_sample]++;
		}
		// click price
		sample_price_map[click_sample] = atoi(tmp[14].c_str());
	}
	in.close();

	cerr << "generating ad date list ...\n";
	ofstream out(outfile.c_str());
	if (!out)
	{
		cerr << "can't open outfile.\n";
		return -1;
	}
	cnt=0;
	t_string_uint_map::iterator it;
	for (it = sample_pv_map.begin(); it != sample_pv_map.end(); ++it)
	{
		if (cnt%100000 == 0)
		{
			cerr << "current processing: " << cnt << endl;
		}
		cnt++;

		uint32_t click;
		string price;
		float ctr;
		uint32_t pv = it->second;
		if (sample_click_map.find(it->first) != sample_click_map.end())
		{
			click = sample_click_map[it->first]; 
			uint32_t p = sample_price_map[it->first];
			char tmp_str[32];
			sprintf(tmp_str, "%u", p);
			price = tmp_str;
		}
		else
		{
			click = 0; 
			price = "na";
		}
		ctr = (float)click/pv;
		out <<  pv << "\t" << click << "\t" << ctr << "\t" << it->first << "\t" << price << endl;
	}
	out.close();

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
