#pragma once


template <typename T>
string ValueToString(T t)
{
	ostringstream stream;
	stream << fixed <<  t;
	return stream.str();
}

template <typename T>
T ValueFromString(const char* text)
{
	T t;

	string str(text);
	istringstream stream(str);
	stream >> t;

	return t;
}

template <>
inline bool ValueFromString<bool>(const char* text)
{
	string t(text);
	return t == "true" || t == "1";
}


inline size_t DecodePackedInt(const char* data, size_t& count)
{
	size_t res = 0;
	count = 0;

	do
	{
		res |= (data[count] & 0x7F) << (count * 7);
		++count;
	} while (data[count - 1] & 0x80);

	return res;
}

inline size_t EncodePackedInt(char* outbuff, size_t num)
{
	size_t count = 0;

	do
	{
		outbuff[count] = num & 0x7F;

		if (num > 0x7F)
		{
			outbuff[count] |= 0x80;
		}

		count++;
		num = num >> 7;
	} while (num);

	return count;
}