#include "hook.h"

namespace utils::hook
{
    namespace
    {
        [[maybe_unused]] class _
        {
        public:
            _()
            {
                if (MH_Initialize() != MH_OK)
                    throw std::runtime_error("Failed to initialize MinHook");
            }

            ~_()
            {
                MH_Uninitialize();
            }
        } __;
    }

    detour::detour(const size_t place, void* target) : detour(reinterpret_cast<void*>(place), target)
    {
    }

    detour::detour(void* place, void* target)
    {
        this->create(place, target);
    }

    detour::~detour()
    {
        this->clear();
    }

    void detour::enable() const
    {
        MH_EnableHook(this->place);
    }

    void detour::disable() const
    {
        MH_DisableHook(this->place);
    }

    void detour::create(void* _place, void* target)
    {
        this->clear();
        this->place = _place;

        MH_STATUS status = MH_CreateHook(this->place, target, &this->original);
        if (status != MH_OK)
            if (status != MH_ERROR_ALREADY_CREATED) // MH_STATUS can be MH_ERROR_ALREADY_CREATED for dll like ui_mp_x86
                throw std::runtime_error(string::va("Unable to create hook at location: %p, MH_STATUS: %s", this->place, MH_StatusToString(status)));

        this->enable();
    }

    void detour::create(const size_t _place, void* target)
    {
        this->create(reinterpret_cast<void*>(_place), target);
    }

    void detour::clear()
    {
        if (this->place)
            MH_RemoveHook(this->place);

        this->place = nullptr;
        this->original = nullptr;
    }

    void* detour::get_original() const
    {
        return this->original;
    }

    void nop(void* place, const size_t length)
    {
        DWORD old_protect{};
        VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

        std::memset(place, 0x90, length);

        VirtualProtect(place, length, old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), place, length);
    }

    void nop(const size_t place, const size_t length)
    {
        //int len = (length < place) ? length : (length - place);

        nop(reinterpret_cast<void*>(place), length);
    }
    
    void call(void* pointer, void* data)
    {
        auto* patch_pointer = PBYTE(pointer);
        set<uint8_t>(patch_pointer, 0xE8);
        set<int32_t>(patch_pointer + 1, int32_t(size_t(data) - (size_t(pointer) + 5)));
    }

    void call(const size_t pointer, void* data)
    {
        return call(reinterpret_cast<void*>(pointer), data);
    }

    void call(const size_t pointer, const size_t data)
    {
        return call(pointer, reinterpret_cast<void*>(data));
    }
    
    void jump(void* pointer, void* data)
    {
        auto* patch_pointer = PBYTE(pointer);
        set<uint8_t>(patch_pointer, 0xE9);
        set<int32_t>(patch_pointer + 1, int32_t(size_t(data) - (size_t(pointer) + 5)));
    }

    void jump(const size_t pointer, void* data)
    {
        return jump(reinterpret_cast<void*>(pointer), data);
    }

    void jump(const size_t pointer, const size_t data)
    {
        return jump(pointer, reinterpret_cast<void*>(data));
    }
    
    void inject(void* pointer, const void* data)
    {
        set<int32_t>(pointer, int32_t(size_t(data) - (size_t(pointer) + 4)));
    }

    void inject(const size_t pointer, const void* data)
    {
        return inject(reinterpret_cast<void*>(pointer), data);
    }
}