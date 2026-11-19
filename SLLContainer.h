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

        explicit Node(const T &value) : data(value), next(nullptr) // Копируем значение в узел. Когда узел последний следующий пуст
        {
        }
    };

    // Каждая ячейка должна вмещать число и указатель на след
    // Поэтому получаем аллокатор для Node из переданного аллокатора для T
    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using NodeTraits = std::allocator_traits<NodeAllocator>;

    // Храним аллокатор, адреса первого и последнего узлов и размер
    NodeAllocator m_allocator;
    Node *m_head = nullptr;
    Node *m_tail = nullptr;
    std::size_t m_size = 0;

public:
    // Итератор запоминает текущий узел и умеет переходить к следующему
    class const_iterator
    {
    private:
        // nullptr обозначает позицию после последнего узла
        const Node *m_ptr = nullptr;

    public:
        // итератор идёт вперёд и возвращает значение T по константной ссылке       
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T *;
        using reference = const T &;

        // Без аргумента получаем итератор с m_ptr == nullptr
        const_iterator() = default;

        explicit const_iterator(const Node *ptr) : m_ptr(ptr)
        {
        }

        reference operator*() const         // *it возвращает значение текущего узла
        {
            return m_ptr->data;
        }

        pointer operator->() const  // it->field обращается к полю хранимого значения T
        {
            return std::addressof(m_ptr->data);
        }

        const_iterator &operator++()  // ++it переходит по next и возвращает уже сдвинутый итератор
        {
            m_ptr = m_ptr->next;
            return *this;
        }

        const_iterator operator++(int) // it++ тоже сдвигает итератор, но возвращает его старую позицию
        {
            const_iterator old = *this;
            ++(*this);
            return old;
        }

        bool operator==(const const_iterator &other) const // Итераторы равны, когда хранят один и тот же адрес
        {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const const_iterator &other) const // Цикл использует это сравнение, чтобы остановиться на end()
        {
            return !(*this == other);
        }
    };

    using iterator = const_iterator;

    explicit SLLContainer(const Allocator &allocator = Allocator()) // Создаём пустой список и сохраняем аллокатор для его узлов
        : m_allocator(allocator)
    {
    }

    // Запрещаем копирование. Простая копия указателей заставила бы
    // два списка уничтожать одни и те же узлы
    SLLContainer(const SLLContainer &) = delete;
    SLLContainer &operator=(const SLLContainer &) = delete;

    // Деструктор
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
        Node *new_node = NodeTraits::allocate(m_allocator, 1); // Просим у аллокатора место под один Node

        // Создаём Node по полученному адресу и записываем в него value
        try
        {
            NodeTraits::construct(m_allocator, new_node, value);
        }
        catch (...)
        {
            // Если создание узла не удалось, возвращаем выделенное место
            // и передаём исходную ошибку вызывающему коду
            NodeTraits::deallocate(m_allocator, new_node, 1);
            throw;
        }

        // Если список уже содержит узлы, старый хвост указывает на новый
        // В пустом списке новый узел становится головой
        if (m_tail)
        {
            m_tail->next = new_node;
        }
        else
        {
            m_head = new_node;
        }

        m_tail = new_node; // В обоих случаях новый узел становится хвостом
        ++m_size;
    }

    std::size_t size() const // Возвращаем сохранённое число элементов
    {
        return m_size;
    }

    bool empty() const  // Пустой список содержит ноль узлов
    {
        return m_size == 0;
    }

    const_iterator begin() const // Начальная позиция обхода
    {
        return const_iterator(m_head);
    }

    const_iterator end() const // Конечная позиция
    {
        return const_iterator();
    }
};
