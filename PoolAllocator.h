#pragma once

#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <vector>

template <typename T, std::size_t BlockSize>
class MemoryPool
{
private:
    struct Block
    {
        std::vector<unsigned char> used;
        T *data;

        explicit Block(std::size_t count)
            : used(count, 0), data(std::allocator<T>{}.allocate(count))
        {
        }

        ~Block()
        {
            std::allocator<T>{}.deallocate(data, used.size());
        }

        Block(const Block &) = delete;
        Block &operator=(const Block &) = delete;
    };

    std::list<Block> m_blocks;

public:
    void reserve()
    {
        if (m_blocks.empty())
        {
            m_blocks.emplace_back(BlockSize);
        }
    }

    T *allocate(std::size_t count)
    {
        for (auto &block : m_blocks)
        {
            std::size_t free_count = 0;
            for (std::size_t i = 0; i < block.used.size(); ++i)
            {
                if (block.used[i])
                {
                    free_count = 0;
                    continue;
                }

                ++free_count;
                if (free_count == count)
                {
                    const std::size_t first = i + 1 - count;
                    for (std::size_t j = first; j <= i; ++j)
                    {
                        block.used[j] = 1;
                    }
                    return block.data + first;
                }
            }
        }

        std::size_t capacity = BlockSize;
        if (count > capacity)
        {
            capacity = count;
        }
        auto &block = m_blocks.emplace_back(capacity);
        for (std::size_t i = 0; i < count; ++i)
        {
            block.used[i] = 1;
        }
        return block.data;
    }

    void deallocate(T *ptr, std::size_t count) noexcept
    {
        for (auto &block : m_blocks)
        {
            for (std::size_t i = 0; i < block.used.size(); ++i)
            {
                if (block.data + i == ptr)
                {
                    for (std::size_t j = 0; j < count; ++j)
                    {
                        block.used[i + j] = 0;
                    }
                    return;
                }
            }
        }
    }
};

using PoolStorage = std::map<std::type_index, std::shared_ptr<void>>;

template <typename T, std::size_t BlockSize>
class PoolAllocator
{
    static_assert(BlockSize > 0);

    template <typename U, std::size_t Size>
    friend class PoolAllocator;

private:
    std::shared_ptr<PoolStorage> m_storage;

    MemoryPool<T, BlockSize> &get_pool()
    {
        auto &pool = (*m_storage)[std::type_index(typeid(T))];
        if (!pool)
        {
            pool = std::make_shared<MemoryPool<T, BlockSize>>();
        }
        return *static_cast<MemoryPool<T, BlockSize> *>(pool.get());
    }

public:
    using value_type = T;

    template <typename U>
    struct rebind
    {
        using other = PoolAllocator<U, BlockSize>;
    };

    PoolAllocator() : m_storage(std::make_shared<PoolStorage>())
    {
    }

    PoolAllocator(const PoolAllocator &) noexcept = default;

    template <typename U>
    PoolAllocator(const PoolAllocator<U, BlockSize> &other) noexcept
        : m_storage(other.m_storage)
    {
    }

    void reserve()
    {
        get_pool().reserve();
    }

    T *allocate(std::size_t count)
    {
        if (count == 0)
        {
            return nullptr;
        }
        return get_pool().allocate(count);
    }

    void deallocate(T *ptr, std::size_t count) noexcept
    {
        if (count == 0)
        {
            return;
        }
        auto pool = m_storage->find(std::type_index(typeid(T)));
        static_cast<MemoryPool<T, BlockSize> *>(pool->second.get())->deallocate(ptr, count);
    }

    template <typename U>
    bool operator==(const PoolAllocator<U, BlockSize> &other) const noexcept
    {
        return m_storage == other.m_storage;
    }

    template <typename U>
    bool operator!=(const PoolAllocator<U, BlockSize> &other) const noexcept
    {
        return !(*this == other);
    }
};
