#pragma once

#include"Common/Common.h"
#include <string>
#include <stdint.h>
#include <float.h>
#include <map>
#include <set>

class TextGraphics;

class AppGlobals
{
public: 
	virtual ~AppGlobals() {}
	virtual void Increment(void) {}   
	virtual void Decrement(void) {}

	virtual void Display(TextGraphics&) const {}

	virtual void Set(FILE* file, const std::string& setting) = 0;

	AppGlobals* NextItem(void);
	AppGlobals* PrevItem(void);

protected:
	AppGlobals(void);
	AppGlobals(const std::string& path);

private:
	friend class ItemGroup;
	ItemGroup* m_GroupPtr;

};

class BooleanItem : public AppGlobals
{
public:

	BooleanItem(const std::string& path, bool item);
	BooleanItem& operator=(bool item) { m_Binary = item; return *this; }
	operator bool() const { return m_Binary; }

	virtual void Increment(void) override { m_Binary = true; }
	virtual void Decrement(void) override { m_Binary = false; }

	

	virtual void Display(TextGraphics& Text) const override;
	virtual void Set(FILE* file, const std::string& setting) override;



private:
	bool m_Binary;

};

class NumberItem : public AppGlobals
{
public:

	NumberItem(const std::string& path, float item, float minValue = -FLT_MAX, float maxValue = FLT_MAX, float stepSize = 1.0f);
	NumberItem& operator=(float item) { m_Number = Clamp(item); return *this; }
	operator float() const { return m_Number; }

	virtual void Increment(void) override { m_Number = Clamp(m_Number + m_StepSize); }
	virtual void Decrement(void) override { m_Number = Clamp(m_Number - m_StepSize); }

	virtual void Display(TextGraphics& Text) const override;
	
	virtual void Set(FILE* file, const std::string& setting)  override;


protected:

	float Clamp(float item) { return item > m_MaxNumber ? m_MaxNumber : item < m_MinNumber ? m_MinNumber : item; }

	float m_Number;
	float m_MinNumber;
	float m_MaxNumber;
	float m_StepSize;
};



class ExpItem : public NumberItem
{
public:
	ExpItem(const std::string& path, float item, float minExp = -FLT_MAX, float maxExp = FLT_MAX, float expStepSize = 1.0f);
	ExpItem& operator=(float item);    // m_Value = log2(val)
	operator float() const;            // returns exp2(m_Value)

	virtual void Display(TextGraphics& Text) const override;

	virtual void Set(FILE* file, const std::string& setting) override;

};


class IntegerItem : public AppGlobals
{
public:
	IntegerItem(const std::string& path, int32_t val, int32_t minValue = 0, int32_t maxValue = (1 << 24) - 1, int32_t stepSize = 1);

	IntegerItem& operator=(int32_t val) { m_Number = Clamp(val); return *this; }
	operator int32_t() const { return m_Number; }

	virtual void Increment(void) override { m_Number = Clamp(m_Number + m_StepSize); }
	virtual void Decrement(void) override { m_Number = Clamp(m_Number - m_StepSize); }

	virtual void Display(TextGraphics& Text) const override;
	
	virtual void Set(FILE* file, const std::string& setting) override;

protected:

	float Clamp(float item) { return item > m_MaxNumber ? m_MaxNumber : item < m_MinNumber ? m_MinNumber : item; }

	float m_Number;
	float m_MinNumber;
	float m_MaxNumber;
	float m_StepSize;

};

class EnumItem : public AppGlobals
{
public:
	EnumItem(const std::string& path, int32_t initialItem, int32_t listLength, const char** listLabels);
	EnumItem& operator=(int32_t val) { m_Item = Clamp(val); return *this; }
	operator int32_t() const { return m_Item; }

	virtual void Increment(void) override { m_Item = (m_Item + 1) % m_EnumLength; }
	virtual void Decrement(void) override { m_Item = (m_Item + m_EnumLength - 1) % m_EnumLength; }

	virtual void Display(TextGraphics& Text) const override;
	
	virtual void Set(FILE* file, const std::string& setting) override;

	void SetListLength(int32_t listLength) { m_EnumLength = listLength; m_Item = Clamp(m_Item); }

private:
	int32_t Clamp(int32_t item) { return item < 0 ? 0 : item >= m_EnumLength ? m_EnumLength - 1 : item; }

	int32_t m_Item;
	int32_t m_EnumLength;
	const char** m_EnumLabels;
};

class TextGraphicsCmdBuffers;

namespace EngineTuning
{
	void Initialize(void);
	void Update(float frameTime);
	void Display(TextGraphicsCmdBuffers& CmdBuffer, float x, float y, float w, float h);
	bool IsFocused(void);

} 

