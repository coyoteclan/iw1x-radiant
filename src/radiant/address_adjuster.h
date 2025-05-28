#ifdef BUILDING_IW1X_DLL
#define WEAK __declspec(selectany)
#else
#define WEAK
#endif

constexpr auto BASE_CGAME_MP = 0x30000000;
constexpr auto BASE_UI_MP = 0x40000000;

//extern DWORD address_cgame_mp;
//extern DWORD address_ui_mp;
#include "iw1x_core.h"

#define ABSOLUTE_CGAME_MP(relative) (address_cgame_mp + (relative - BASE_CGAME_MP))
#define ABSOLUTE_UI_MP(relative) (address_ui_mp + (relative - BASE_UI_MP))

template <typename T>
class adjuster
{
public:
	adjuster(const size_t address, const ptrdiff_t offset = 0) :
		object(reinterpret_cast<T*>(address)),
		offset(offset)
	{
	}

	T* get() const
	{
		uintptr_t base_address = (offset == BASE_CGAME_MP) ? address_cgame_mp :
			(offset == BASE_UI_MP) ? address_ui_mp : 0;
		
		if (base_address == 0)
			return object;
		
		return reinterpret_cast<T*>(base_address + (reinterpret_cast<uintptr_t>(object) - offset));
	}

	operator T* () const
	{
		return this->get();
	}

	T* operator->() const
	{
		return this->get();
	}
private:
	T* object;
	ptrdiff_t offset;
};