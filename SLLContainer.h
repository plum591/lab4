#pragma once

#include <cstddef>
#include <iterator>
#include <memory>

template <typename T, typename Allocator = std::allocator<T>>
class SLLContainer
{
private:
    struct Node
    {
        T data;
        Node *next;

        explicit Node(const T &value) : data(value), next(nullptr)
        {
        }
    };

    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using NodeTraits = std::allocator_traits<NodeAllocator>;

    NodeAllocator m_allocator;
    Node *m_head = nullptr;
    Node *m_tail = nullptr;
    std::size_t m_size = 0;

public:
    class const_iterator
    {
    private:
        const Node *m_ptr = nullptr;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T *;
        using reference = const T &;

        const_iterator() = default;

        explicit const_iterator(const Node *ptr) : m_ptr(ptr)
        {
        }

        reference operator*() const
        {
            return m_ptr->data;
        }

        pointer operator->() const
        {
            return std::addressof(m_ptr->data);
        }

        const_iterator &operator++()
        {
            m_ptr = m_ptr->next;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator old = *this;
            ++(*this);
            return old;
        }

        bool operator==(const const_iterator &other) const
        {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const const_iterator &other) const
        {
            return !(*this == other);
        }
    };

    using iterator = const_iterator;

    explicit SLLContainer(const Allocator &allocator = Allocator())
        : m_allocator(allocator)
    {
    }

    SLLContainer(const SLLContainer &) = delete;
    SLLContainer &operator=(const SLLContainer &) = delete;

    ~SLLContainer()
    {
        while (m_head)
        {
            Node *current = m_head;
            m_head = current->next;
            NodeTraits::destroy(m_allocator, current);
            NodeTraits::deallocate(m_allocator, current, 1);
        }
    }

    void push_back(const T &value)
    {
        Node *new_node = NodeTraits::allocate(m_allocator, 1);
        try
        {
            NodeTraits::construct(m_allocator, new_node, value);
        }
        catch (...)
        {
            NodeTraits::deallocate(m_allocator, new_node, 1);
            throw;
        }

        if (m_tail)
        {
            m_tail->next = new_node;
        }
        else
        {
            m_head = new_node;
        }
        m_tail = new_node;
        ++m_size;
    }

    std::size_t size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return m_size == 0;
    }

    const_iterator begin() const
    {
        return const_iterator(m_head);
    }

    const_iterator end() const
    {
        return const_iterator();
    }
};
