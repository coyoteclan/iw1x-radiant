#include "nt.h"

namespace utils::nt
{
	library library::load(const std::string& name)
	{
		return library(LoadLibraryA(name.data()));
	}

	library library::get_by_address(void* address)
	{
		HMODULE handle = nullptr;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCSTR>(address), &handle);
		return library(handle);
	}

	library::library()
	{
		this->module = GetModuleHandleA(nullptr);
	}

	library::library(const std::string& name)
	{
		this->module = GetModuleHandleA(name.data());
	}

	library::library(const HMODULE handle)
	{
		this->module = handle;
	}

	bool library::operator==(const library& obj) const
	{
		return this->module == obj.module;
	}

	library::operator bool() const
	{
		return this->is_valid();
	}

	library::operator HMODULE() const
	{
		return this->get_handle();
	}

	PIMAGE_NT_HEADERS library::get_nt_headers() const
	{
		if (!this->is_valid()) return nullptr;
		return reinterpret_cast<PIMAGE_NT_HEADERS>(this->get_ptr() + this->get_dos_header()->e_lfanew);
	}

	PIMAGE_DOS_HEADER library::get_dos_header() const
	{
		return reinterpret_cast<PIMAGE_DOS_HEADER>(this->get_ptr());
	}

	PIMAGE_OPTIONAL_HEADER library::get_optional_header() const
	{
		if (!this->is_valid()) return nullptr;
		return &this->get_nt_headers()->OptionalHeader;
	}

	std::vector<PIMAGE_SECTION_HEADER> library::get_section_headers() const
	{
		std::vector<PIMAGE_SECTION_HEADER> headers;

		auto nt_headers = this->get_nt_headers();
		auto section = IMAGE_FIRST_SECTION(nt_headers);

		for (uint16_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i, ++section)
		{
			if (section) headers.push_back(section);
		}

		return headers;
	}

	std::uint8_t* library::get_ptr() const
	{
		return reinterpret_cast<std::uint8_t*>(this->module);
	}

	size_t library::get_relative_entry_point() const
	{
		if (!this->is_valid()) return 0;
		return this->get_nt_headers()->OptionalHeader.AddressOfEntryPoint;
	}

	bool library::is_valid() const
	{
		return this->module != nullptr && this->get_dos_header()->e_magic == IMAGE_DOS_SIGNATURE;
	}

	HMODULE library::get_handle() const
	{
		return this->module;
	}
}