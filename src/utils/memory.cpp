#include "memory.h"

namespace utils
{
	memory::allocator memory::mem_allocator_;

	memory::allocator::~allocator()
	{
		this->clear();
	}

	void memory::allocator::clear()
	{
		std::lock_guard _(this->mutex_);

		for (const auto& data : this->pool_)
		{
			memory::free(data);
		}

		this->pool_.clear();
	}

	void memory::allocator::free(void* data)
	{
		std::lock_guard _(this->mutex_);

		const auto j = std::find(this->pool_.begin(), this->pool_.end(), data);
		if (j != this->pool_.end())
		{
			memory::free(data);
			this->pool_.erase(j);
		}
	}

	void memory::allocator::free(const void* data)
	{
		this->free(const_cast<void*>(data));
	}

	void* memory::allocator::allocate(const size_t length)
	{
		std::lock_guard _(this->mutex_);

		auto* data = memory::allocate(length);
		this->pool_.push_back(data);
		return data;
	}

	bool memory::allocator::empty() const
	{
		return this->pool_.empty();
	}

	void* memory::allocate(const size_t length)
	{
		return std::calloc(length, 1);
	}

	void memory::free(void* data)
	{
		std::free(data);
	}

	void memory::free(const void* data)
	{
		free(const_cast<void*>(data));
	}

	memory::allocator* memory::get_allocator()
	{
		return &memory::mem_allocator_;
	}
}