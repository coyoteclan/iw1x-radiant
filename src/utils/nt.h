#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <functional>
#include <filesystem>

namespace utils::nt
{
	class library final
	{
	public:
		static library load(const std::string& name);
		static library get_by_address(void* address);

		library();
		explicit library(const std::string& name);
		explicit library(HMODULE handle);

		library(const library& a) : module(a.module)
		{
		}

		bool operator!=(const library& obj) const { return !(*this == obj); };
		bool operator==(const library& obj) const;

		operator bool() const;
		operator HMODULE() const;
		size_t get_relative_entry_point() const;

		bool is_valid() const;
		std::uint8_t* get_ptr() const;

		HMODULE get_handle() const;

		template <typename T>
		T get_proc(const std::string& process) const
		{
			if (!this->is_valid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->module, process.data()));
		}

		template <typename T>
		T get_proc(const char* name) const
		{
			if (!this->is_valid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->module, name));
		}

		template <typename T>
		std::function<T> get(const std::string& process) const
		{
			if (!this->is_valid()) return std::function<T>();
			return static_cast<T*>(this->get_proc<void*>(process));
		}

		template <typename T, typename... Args>
		T invoke(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__cdecl)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_pascal(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__stdcall)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_this(const std::string& process, void* this_ptr, Args ... args) const
		{
			auto method = this->get<T(__thiscall)(void*, Args ...)>(this_ptr, process);
			if (method) return method(args...);
			return T();
		}

		std::vector<PIMAGE_SECTION_HEADER> get_section_headers() const;

		PIMAGE_NT_HEADERS get_nt_headers() const;
		PIMAGE_DOS_HEADER get_dos_header() const;
		PIMAGE_OPTIONAL_HEADER get_optional_header() const;

	private:
		HMODULE module;
	};
}