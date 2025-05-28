#include "_string.h"

#pragma comment(lib, "ws2_32.lib")

namespace utils::string
{
	const char* va(const char* fmt, ...)
	{
		static thread_local va_provider<8, 256> provider;

		va_list ap;
		va_start(ap, fmt);

		const char* result = provider.get(fmt, ap);

		va_end(ap);
		return result;
	}

	void clean(const char* in, char* out, int max, bool removeColors)
	{
		if (!in || !out || !max) return;

		max--;
		auto current = 0;
		while (*in != 0 && current < max)
		{
			const auto color_index = (*(in + 1) - 48) >= 0xC ? 7 : (*(in + 1) - 48);

			if (removeColors && *in == '^' && (color_index != 7 || *(in + 1) == '7'))
			{
				++in;
			}
			else if (*in >= 0x20 && *in <= 0x7E)
			{
				*out = *in;
				++out;
				++current;
			}

			++in;
		}
		*out = '\0';
	}

	std::string clean(const std::string& string, bool removeColors)
	{
		std::string new_string;
		new_string.resize(string.size() + 1, 0);
		clean(string.data(), new_string.data(), static_cast<int>(new_string.size()), removeColors);
		return new_string.data();
	}

	std::string convert(const std::wstring& wstr)
	{
		std::string result;
		result.reserve(wstr.size());

		for (const auto& chr : wstr)
		{
			result.push_back(static_cast<char>(chr));
		}

		return result;
	}

	std::wstring convert(const std::string& str)
	{
		std::wstring result;
		result.reserve(str.size());

		for (const auto& chr : str)
		{
			result.push_back(static_cast<wchar_t>(chr));
		}

		return result;
	}
	
	static bool isValidIP(const std::string& ip)
	{
		struct sockaddr_in sa;
		return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1;
	}
	static bool isValidPort(const std::string& portStr)
	{
		try
		{
			return std::stoi(portStr) >= 0 && std::stoi(portStr) <= 65535;
		}
		catch (...)
		{
			return false;
		}
	}
	bool isValidIPPort(const std::string& ipPort)
	{
		std::stringstream ss(ipPort);
		std::string ip;
		std::string port;
    
		if (std::getline(ss, ip, ':') && std::getline(ss, port))
			return isValidIP(ip) && isValidPort(port);
		return false;
	}
}