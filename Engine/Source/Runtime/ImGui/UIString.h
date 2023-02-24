#include"Application/Localization.h"
#include<string.h>

namespace engine
{

	
	enum class UIString
	{
		File = 0,
		New,
		Open,
		Count
	};

	class UIText {
	public:
		 ~UIText();
		 UIText();
	    const char** SetText(const char* text);
	    const char** m_pUIText = nullptr;
	private:
		 const char* englishstring[static_cast<int>(UIString::Count)]=
		 {
			 "File",
	         "New",
	         "Open"
		 };
		 const char* chinesestring[static_cast<int>(UIString::Count)]=
		 {
		 "文件",
		 "创建",
		 "打开"
		 };

	};

	
	
		

	
	
	
	
}