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
        // Выделяет память под count объектов и отмечает все ячейки свободными
        explicit Block(std::size_t count)
            : used(count, 0), data(std::allocator<T>{}.allocate(count))
        {
        }
        // Возвращает всю память блока стандартному аллокатору
        ~Block()
        {
            std::allocator<T>{}.deallocate(data, used.size());
        }
        // Запрещает копировать блок, чтобы два владельца не освободили один адрес
        Block(const Block &) = delete;
        Block &operator=(const Block &) = delete;
    };
    // Хранит выделенные блоки в порядке их создания
    std::list<Block> m_blocks;

public:
    void reserve()// Создаёт первый блок из BlockSize ячеек, если пул ещё пуст
    {
        if (m_blocks.empty())
        {
            m_blocks.emplace_back(BlockSize);
        }
    }
    // Количество соседних ячеек и возвращает адрес первой
    T *allocate(std::size_t count)
    {
        for (auto &block : m_blocks)
        {
            std::size_t free_count = 0;
            for (std::size_t i = 0; i < block.used.size(); ++i)
            {
                if (block.used[i])// Занятая ячейка прерывает ряд свободных ячеек
                {
                    free_count = 0;
                    continue;
                }
                ++free_count;
                // Когда нашли count ячеек подряд, вычисляем начало диапазона
                // и ставим единицы в отметках от 1 до i
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
        // Подходящего диапазона нет. Выбираем размер нового блока
        std::size_t capacity = BlockSize;
        if (count > capacity)
        {
            capacity = count;
        }
        
        // Создаём блок, занимаем первые count ячеек и возвращаем его начало
        auto &block = m_blocks.emplace_back(capacity);
        for (std::size_t i = 0; i < count; ++i)
        {
            block.used[i] = 1;
        }
        return block.data;
    }

    // Освобождение count ячеек памяти
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

// По типу ячейки находим указатель на соответствующий MemoryPool
using PoolStorage = std::map<std::type_index, std::shared_ptr<void>>;

// Контейнер вызывает PoolAllocator. Тот выбирает пул по типу T
template <typename T, std::size_t BlockSize>
class PoolAllocator
{
    static_assert(BlockSize > 0);

    // Позволяем аллокаторам разных типов читать m_storage друг друга
    template <typename U, std::size_t Size>
    friend class PoolAllocator;

private:
    // Копии аллокатора и аллокаторы после rebind делят один словарь
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
    using value_type = T;     // Сообщаем контейнеру тип объектов, для которых выдаём память

    template <typename U>    // Позволяем списку заменить int на Node, сохранив размер блока
    struct rebind
    {
        using other = PoolAllocator<U, BlockSize>;
    };

    PoolAllocator() : m_storage(std::make_shared<PoolStorage>())    // Новый самостоятельный аллокатор получает свой пустой словарь
    {
    }

    PoolAllocator(const PoolAllocator &) noexcept = default;    // Обычная копия использует тот же словарь

     // При смене типа сохраняем указатель на общий словарь пулов
    template <typename U>
    PoolAllocator(const PoolAllocator<U, BlockSize> &other) noexcept
        : m_storage(other.m_storage)
    {
    }

    void reserve()     // Просим пул для T заранее создать первый блок
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

    // Сравниваем адреса словарей, которыми владеют аллокаторы
    template <typename U>
    bool operator==(const PoolAllocator<U, BlockSize> &other) const noexcept
    {
        return m_storage == other.m_storage;
    }

    // Неравенство даёт результат, противоположный operator==
    template <typename U>
    bool operator!=(const PoolAllocator<U, BlockSize> &other) const noexcept
    {
        return !(*this == other);
    }
};
