#include<sstream>
#include<string>
#include <vector>
#include<numeric>
#include<filesystem>
#include<map>
#include<utility>
#include<fstream>
#include <iostream>
#include "Localization.h"
#include "ImGui/ImGuiContextInstance.h"
namespace editor
{
	using namespace std;

	vector<string>vec_key;
	vector<string> vec_CN;
	vector<string> vec_EN;
	static std::map<std::string, std::pair<std::string, std::string>> texts;
	const char* keyconst;
	void Read_csv()
	{
		vector<string> scv;
		std::filesystem::path base_path = std::filesystem::current_path();
		
		std::filesystem::path csv_path = base_path / "text_csv" / "text.csv";
	    ifstream fp(csv_path);
		string line;
		int textnum = 0;
		while (getline(fp, line))
		{
			stringstream ss(line);
			string str;
			while (getline(ss, str, ','))
			{

				scv.push_back(str + ",");

				textnum++;
			}

		};



		string str;
		str = accumulate(scv.begin(), scv.end(), str);
		size_t keybegin = str.find("TEXT_TEST");
		size_t keyend = str.find("TEXT_END") - keybegin;
		size_t CNbegin = str.find("测试");
		size_t CNend = (str.find("CNEND") - CNbegin);
		size_t ENbegin = str.find("Test");
		size_t ENend = str.find("ENEND") - ENbegin;
		string keystr = str.substr(keybegin, keyend);
		string CNstr = str.substr(CNbegin, CNend);
		string ENstr = str.substr(ENbegin, ENend);
		stringstream ssCN(CNstr);
		stringstream sskey(keystr);
		stringstream ssEN(ENstr);
		string keybuf;
		string CNbuf;
		string ENbuf;
		int keynum = 0;
		int CNnum = 0;
		int ENnum = 0;

		while (getline(sskey, keybuf, ','))
		{
			vec_key.push_back(keybuf);
			keynum++;
		}
		while (getline(ssCN, CNbuf, ','))
		{
			vec_CN.push_back(CNbuf);
			CNnum++;
		}
		while (getline(ssEN, ENbuf, ','))
		{
			vec_EN.push_back(ENbuf);
			ENnum++;
		}

		for (std::size_t i = 0; i < vec_key.size(); ++i) {
			texts[vec_key[i]] = { vec_CN[i], vec_EN[i] };
		}


	}
	const char* SetText(const char* text)
	{
		const char* result;
		if (1) {
			result = texts[text].first.c_str();
		}
		else {
			result = texts[text].second.c_str();
		}
		return result;

	}
}
