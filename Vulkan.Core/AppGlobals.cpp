
#include "AppGlobals.h"

namespace EngineTuning
{


	enum { kMaxUnregisteredTweaks = 1024 };
	char s_UnregisteredPath[kMaxUnregisteredTweaks][128];
	AppGlobals* s_UnregisteredVariable[kMaxUnregisteredTweaks] = { nullptr };

	int32_t s_UnregisteredCount = 0;
	// Internal functions
	void AddToVariableGraph(const std::string& path, AppGlobals& var);
	void RegisterVariable(const std::string& path, AppGlobals& var);

	AppGlobals* sm_SelectedVariable = nullptr;
	bool sm_IsVisible = false;

}

class ItemGroup : public AppGlobals
{
public:
	ItemGroup() : m_IsExpanded(false) {}

	AppGlobals* FindChild(const std::string& name)
	{
		auto iter = m_Children.find(name);
		return iter == m_Children.end() ? nullptr : iter->second;
	}

	void AddChild(const std::string& name, AppGlobals& child)
	{
		m_Children[name] = &child;
		child.m_GroupPtr = this;
	}

	void Display(TextGraphics& Text, float leftMargin, AppGlobals* highlightedTweak);

	void SaveToFile(FILE* file, int fileMargin);
	void LoadSettingsFromFile(FILE* file);

	AppGlobals* NextVariable(AppGlobals* currentVariable);
	AppGlobals* PrevVariable(AppGlobals* currentVariable);
	AppGlobals* FirstVariable(void);
	AppGlobals* LastVariable(void);

	bool IsExpanded(void) const { return m_IsExpanded; }

	virtual void Increment(void) override { m_IsExpanded = true; }
	virtual void Decrement(void) override { m_IsExpanded = false; }


	virtual void Set(FILE*, const std::string&) override {}

	static ItemGroup sm_RootGroup;


private:
	bool m_IsExpanded;
	std::map<std::string, AppGlobals*> m_Children;

	

};

void ItemGroup::Display(TextGraphics & Text, float leftMargin, AppGlobals * highlightedTweak)
{
	//TDO create a buffer and commantds for the text display
}

void ItemGroup::SaveToFile(FILE * file, int fileMargin)
{
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
	{
		const char* buffer = (iter->first).c_str();

		ItemGroup* subGroup = dynamic_cast<ItemGroup*>(iter->second);
		if (subGroup != nullptr)
		{
			fprintf(file, "%*c + %s ...\r\n", fileMargin, ' ', buffer);
			subGroup->SaveToFile(file, fileMargin + 5);
		}
		/*else if (dynamic_cast<CallbackTrigger*>(iter->second) == nullptr)
		{
			fprintf(file, "%*c %s:  %s\r\n", fileMargin, ' ', buffer, iter->second->ToString().c_str());
		}*/
	}
}

void ItemGroup::LoadSettingsFromFile(FILE * file)
{
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
	{
		ItemGroup* subGroup = dynamic_cast<ItemGroup*>(iter->second);
		if (subGroup != nullptr)
		{
			char skippedLines[100];
			fscanf_s(file, "%*s %[^\n]", skippedLines, (int)_countof(skippedLines));
			subGroup->LoadSettingsFromFile(file);
		}
		else
		{
			iter->second->Set(file, iter->first);
		}
	}
}

AppGlobals * ItemGroup::NextVariable(AppGlobals * currentVariable)
{
	auto iter = m_Children.begin();
	for (; iter != m_Children.end(); ++iter)
	{
		if (currentVariable == iter->second)
			break;
	}

	assert(iter != m_Children.end(), "Did not find engine variable in its designated group");

	auto nextIter = iter;
	++nextIter;

	if (nextIter == m_Children.end())
		return m_GroupPtr ? m_GroupPtr->NextVariable(this) : nullptr;
	else
		return nextIter->second;
}

AppGlobals * ItemGroup::PrevVariable(AppGlobals * currentVariable)
{
	auto iter = m_Children.begin();
	for (; iter != m_Children.end(); ++iter)
	{
		if (currentVariable == iter->second)
			break;
	}

	assert(iter != m_Children.end(), "Did not find engine variable in its designated group");

	if (iter == m_Children.begin())
		return this;

	auto prevIter = iter;
	--prevIter;

	ItemGroup* isGroup = dynamic_cast<ItemGroup*>(prevIter->second);
	if (isGroup && isGroup->IsExpanded())
		return isGroup->LastVariable();

	return prevIter->second;
}

AppGlobals * ItemGroup::FirstVariable(void)
{
	return m_Children.size() == 0 ? nullptr : m_Children.begin()->second;
}

AppGlobals * ItemGroup::LastVariable(void)
{
	if (m_Children.size() == 0)
		return this;

	auto LastVariable = m_Children.end();
	--LastVariable;

	AppGlobals* isGroup = dynamic_cast<AppGlobals*>(LastVariable->second);
	/*if (isGroup && isGroup->IsExpanded())
		return isGroup->LastVariable();*/

	return LastVariable->second;
}


AppGlobals::AppGlobals(void) : m_GroupPtr(nullptr)
{
}

AppGlobals::AppGlobals(const std::string& path) : m_GroupPtr(nullptr)
{
	EngineTuning::RegisterVariable(path, *this);
}

AppGlobals* AppGlobals::NextItem(void)
{
	AppGlobals* next = nullptr;
	ItemGroup* isGroup = dynamic_cast<ItemGroup*>(this);
	if (isGroup != nullptr && isGroup->IsExpanded())
		next = isGroup->FirstVariable();

	if (next == nullptr)
		next = m_GroupPtr->NextVariable(this);

	return next != nullptr ? next : this;
}

AppGlobals* AppGlobals::PrevItem(void)
{
	AppGlobals* prev = m_GroupPtr->PrevVariable(this);
	if (prev != nullptr && prev != m_GroupPtr)
	{
		ItemGroup* isGroup = dynamic_cast<ItemGroup*>(prev);
		if (isGroup != nullptr && isGroup->IsExpanded())
			prev = isGroup->LastVariable();
	}
	return prev != nullptr ? prev : this;
}

BooleanItem::BooleanItem(const std::string& path, bool val)
	: AppGlobals(path)
{
	m_Binary = val;
}

void BooleanItem::Display(TextGraphics& Text) const
{
	//Text.DrawFormattedString("[%c]", m_Flag ? 'X' : '-');
}

void BooleanItem::Set(FILE* file, const std::string& setting)
{
	std::string pattern = "\n " + setting + ": %s";
	char valstr[6];

	// Search through the file for an entry that matches this setting's name
	fscanf_s(file, pattern.c_str(), valstr, _countof(valstr));

	// Look for one of the many affirmations
	m_Binary = (
		0 == _stricmp(valstr, "1") ||
		0 == _stricmp(valstr, "on") ||
		0 == _stricmp(valstr, "yes") ||
		0 == _stricmp(valstr, "true"));
}

NumberItem::NumberItem(const std::string& path, float val, float minVal, float maxVal, float stepSize)
	: AppGlobals(path)
{
	assert(minVal <= maxVal);
	m_MinNumber = minVal;
	m_MaxNumber = maxVal;
	m_Number = Clamp(val);
	m_StepSize = stepSize;
}

void NumberItem::Display(TextGraphics& Text) const
{
	//Text.DrawFormattedString("%-11f", m_Value);
}

void NumberItem::Set(FILE* file, const std::string& setting)
{
	std::string scanString = "\n" + setting + ": %f";
	float valueRead;

	//If we haven't read correctly, just keep m_Value at default value
	if (fscanf_s(file, scanString.c_str(), &valueRead))
		*this = valueRead;
}

#if _MSC_VER < 1800
__forceinline float log2(float x) { return log(x) / log(2.0f); }
__forceinline float exp2(float x) { return pow(2.0f, x); }
#endif

ExpItem::ExpItem(const std::string& path, float val, float minExp, float maxExp, float expStepSize)
	: NumberItem(path, log2(val), minExp, maxExp, expStepSize)
{
}

ExpItem& ExpItem::operator=(float val)
{
	m_Number = Clamp(log2(val));
	return *this;
}

ExpItem::operator float() const
{
	return exp2(m_Number);
}

void ExpItem::Display(TextGraphics& Text) const
{
	//Text.DrawFormattedString("%-11f", (float)*this);
}



void ExpItem::Set(FILE* file, const std::string& setting)
{
	std::string scanString = "\n" + setting + ": %f";
	float valueRead;

	//If we haven't read correctly, just keep m_Value at default value
	if (fscanf_s(file, scanString.c_str(), &valueRead))
		*this = valueRead;
}

IntegerItem::IntegerItem(const std::string& path, int32_t val, int32_t minVal, int32_t maxVal, int32_t stepSize)
	: AppGlobals(path)
{
	assert(minVal <= maxVal);
	m_MinNumber = minVal;
	m_MaxNumber = maxVal;
	m_Number = Clamp(val);
	m_StepSize = stepSize;
}

void IntegerItem::Display(TextGraphics& Text) const
{
	//Text.DrawFormattedString("%-11d", m_Value);
}



void IntegerItem::Set(FILE* file, const std::string& setting)
{
	std::string scanString = "\n" + setting + ": %d";
	int32_t valueRead;

	if (fscanf_s(file, scanString.c_str(), &valueRead))
		*this = valueRead;
}

EnumItem::EnumItem(const std::string& path, int32_t initialVal, int32_t listLength, const char** listLabels)
	: AppGlobals(path)
{
	assert(listLength > 0);
	m_EnumLength = listLength;
	m_EnumLabels = listLabels;
	m_Item = Clamp(initialVal);
}

void EnumItem::Display(TextGraphics& Text) const
{
	//Text.DrawString(m_EnumLabels[m_Value]);
}


void EnumItem::Set(FILE* file, const std::string& setting)
{
	std::string scanString = "\n" + setting + ": %[^\n]";
	char valueRead[14];

	if (fscanf_s(file, scanString.c_str(), valueRead, _countof(valueRead)) == 1)
	{
		std::string valueReadStr = valueRead;
		valueReadStr = valueReadStr.substr(0, valueReadStr.length() - 1);

		//if we don't find the string, then leave m_EnumLabes[m_Value] as default
		for (int32_t i = 0; i < m_EnumLength; ++i)
		{
			if (m_EnumLabels[i] == valueReadStr)
			{
				m_Item = i;
				break;
			}
		}
	}

}

void EngineTuning::Initialize(void)
{

	for (int32_t i = 0; i < s_UnregisteredCount; ++i)
	{
		assert(strlen(s_UnregisteredPath[i]) > 0, "Register = %d\n", i);
		assert(s_UnregisteredVariable[i] != nullptr);
		AddToVariableGraph(s_UnregisteredPath[i], *s_UnregisteredVariable[i]);
	}
	s_UnregisteredCount = -1;

}

void EngineTuning::Update(float frameTime)
{
	//Some update
}

void EngineTuning::Display(TextGraphicsCmdBuffers& Context, float x, float y, float w, float h)
{
	//rernder the overlay here
}


