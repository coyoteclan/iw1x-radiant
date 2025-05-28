#include "pch.h"

#include "component_loader.h"

void component_loader::load_dll_components()
{
    printf("Loading components from plugins directory...\n");
    OutputDebugStringA("Loading components from plugins directory...\n");
    std::filesystem::path plugins_path = "iw1x_plugins";
    if (!std::filesystem::exists(plugins_path))
    {
        printf("Plugins directory does not exist.\n");
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(plugins_path))
    {
        if (entry.path().extension() == ".dll")
        {
            HMODULE hModule = LoadLibraryA(entry.path().string().c_str());
            if (hModule)
            {
                typedef component_interface* (*CreateComponentFunc)();
                CreateComponentFunc create_func = (CreateComponentFunc)GetProcAddress(hModule, "create_component");
                if (create_func)
                {
                    try
                    {
                        component_interface* component = create_func();
                        if (component)
                        {
                            register_component(std::unique_ptr<component_interface>(component));
                            printf("Loaded component from %s\n", entry.path().string().c_str());
                        }
                        else
                        {
                            printf("Failed to create component from %s\n", entry.path().string().c_str());
                        }
                    }
                    catch (const std::exception& e)
                    {
                        printf("Error loading component from %s: %s\n", entry.path().string().c_str(), e.what());
                        FreeLibrary(hModule);
                    }
                }
                else
                {
                    printf("DLL %s does not export 'create_component'\n", entry.path().string().c_str());
                    FreeLibrary(hModule);
                }
            }
            else
            {
                printf("Failed to load DLL %s: %d\n", entry.path().string().c_str(), GetLastError());
            }
        }
    }
}

void component_loader::register_component(std::unique_ptr<component_interface>&& component)
{
	get_components().push_back(std::move(component));
}

bool component_loader::post_start()
{
	static auto handled = false;
	if (handled) return true;
	handled = true;

	try
	{
		for (const auto& component : get_components())
			component->post_start();
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

bool component_loader::post_load()
{
	static auto handled = false;
	if (handled) return true;
	handled = true;

	try
	{
		for (const auto& component : get_components())
			component->post_load();
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

/*void component_loader::post_unpack()
{
	static auto handled = false;
	if (handled) return;
	handled = true;

	for (const auto& component : get_components())
		component->post_unpack();
}*/

void component_loader::pre_destroy()
{
	static auto handled = false;
	if (handled) return;
	handled = true;

	for (const auto& component : get_components())
		component->pre_destroy();
}

void* component_loader::load_import(const std::string& library, const std::string& function)
{
	void* function_ptr = nullptr;

	for (const auto& component : get_components())
	{
		auto* const component_function_ptr = component->load_import(library, function);
		if (component_function_ptr)
			function_ptr = component_function_ptr;
	}

	return function_ptr;
}

/*void* component_loader::opengl_call(const std::string& function)
{
	void* function_ptr = nullptr;

	for (const auto& component : get_components())
	{
		auto* const component_function_ptr = component->opengl_call(function);
		if (component_function_ptr)
			function_ptr = component_function_ptr;
	}

	return function_ptr;
}*/

void component_loader::trigger_premature_shutdown()
{
	throw premature_shutdown_trigger();
}

std::vector<std::unique_ptr<component_interface>>& component_loader::get_components()
{
	using component_vector = std::vector<std::unique_ptr<component_interface>>;
	using component_vector_container = std::unique_ptr<component_vector, std::function<void(component_vector*)>>;

	static component_vector_container components(new component_vector, [](component_vector* component_vector)
		{
			pre_destroy();
			delete component_vector;
		});

	return *components;
}
