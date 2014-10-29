#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// ----------------------------------------------------------------------------------------------------------------------------

class MCString
{
protected:
	char *String;

public:
	MCString();
	MCString(const char *DefaultString);
	MCString(const MCString &DefaultString);

	~MCString();

	operator char* ();

	MCString& operator = (const char *NewString);
	MCString& operator = (const MCString &NewString);
	MCString& operator += (const char *NewString);
	MCString& operator += (const MCString &NewString);

	friend MCString operator + (const MCString &String1, const char *String2);
	friend MCString operator + (const char *String1, const MCString &String2);
	friend MCString operator + (const MCString &String1, const MCString &String2);

	void Append(const char *Format, ...);
	void Set(const char *Format, ...);
	void Empty();
};
