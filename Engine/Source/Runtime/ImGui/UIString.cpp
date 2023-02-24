#include"UIString.h"
namespace engine
{
	UIText::~UIText()
	{
	}
		
	

	 UIText::UIText()
	{
     
		m_pUIText = englishstring;
	
	}

	const char** UIText::SetText(const char* text)
	{
		UIString uistring = UIString::File;
		int i = static_cast<int>(uistring);
		int n = static_cast<int>(UIString::Count);
		for (i; i < n; i++)
		{
			if (strcmp(text, englishstring[i]) == 0)
			{
				return (m_pUIText + i);
			}
		}
		return 0;
	}

	
}