#include "thread.h"

namespace utils::thread
{
	bool set_name(const HANDLE t, const std::string& name)
	{
		const nt::library kernel32("kernel32.dll");
		if (!kernel32)
		{
			return false;
		}

		const auto set_description = kernel32.get_proc<HRESULT(WINAPI *)(HANDLE, PCWSTR)>("SetThreadDescription");
		if (!set_description)
		{
			return false;
		}

		return SUCCEEDED(set_description(t, string::convert(name).data()));
	}

	bool set_name(const DWORD id, const std::string& name)
	{
		auto* const t = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, id);
		if (!t) return false;

		const auto _ = gsl::finally([t]()
		{
			CloseHandle(t);
		});

		return set_name(t, name);
	}

	bool set_name(std::thread& t, const std::string& name)
	{
		return set_name(t.native_handle(), name);
	}

	bool set_name(const std::string& name)
	{
		return set_name(GetCurrentThread(), name);
	}
}