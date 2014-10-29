#include "stdafx.h"

#include "mstring.h"

// ----------------------------------------------------------------------------------------------------------------------------

MCString::MCString()
{
	String = NULL;
	Empty();
}

MCString::MCString(const char *DefaultString)
{
	String = NULL;
	Empty();
	Set(DefaultString);
}

MCString::MCString(const MCString &DefaultString)
{
	String = NULL;
	Empty();
	Set(DefaultString.String);
}

MCString::~MCString()
{
	delete [] String;
}

MCString::operator char* ()
{
	return String;
}

MCString& MCString::operator = (const char *NewString)
{
	if(String != NewString) Set(NewString);
	return *this;
}

MCString& MCString::operator = (const MCString &NewString)
{
	if(this != &NewString) Set(NewString.String);
	return *this;
}

MCString& MCString::operator += (const char *NewString)
{
	Append(NewString);
	return *this;
}

MCString& MCString::operator += (const MCString &NewString)
{
	Append(NewString.String);
	return *this;
}

MCString operator + (const MCString &String1, const char *String2)
{
	MCString String = String1;
	String += String2;
	return String;
}

MCString operator + (const char *String1, const MCString &String2)
{
	MCString String = String1;
	String += String2;
	return String;
}

MCString operator + (const MCString &String1, const MCString &String2)
{
	MCString String = String1;
	String += String2;
	return String;
}

void MCString::Append(const char *Format, ...)
{
	va_list ArgList;

	va_start(ArgList, Format);

	int AppendixLength = _vscprintf(Format, ArgList);
	char *Appendix = new char[AppendixLength + 1];
	vsprintf_s(Appendix, AppendixLength + 1, Format, ArgList);

	char *OldString = String;
	int OldStringLength = (int)strlen(String);

	int StringLength = OldStringLength + AppendixLength;
	String = new char[StringLength + 1];
	
	strcpy_s(String, StringLength + 1, OldString);
	strcat_s(String, StringLength + 1, Appendix);

	delete [] OldString;
	delete [] Appendix;
}

void MCString::Set(const char *Format, ...)
{
	va_list ArgList;

	va_start(ArgList, Format);

	delete [] String;

	int StringLength = _vscprintf(Format, ArgList);
	String = new char[StringLength + 1];
	vsprintf_s(String, StringLength + 1, Format, ArgList);
}

void MCString::Empty()
{
	delete [] String;
	String = new char[1];
	String[0] = 0;
}
