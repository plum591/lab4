#include <functional>
#include <iostream>
#include <map>
#include <utility>

#include "PoolAllocator.h"
#include "SLLContainer.h"

int main()
{
    using MapAllocator = PoolAllocator<std::pair<const int, int>, 10>;

    std::map<int, int> factorials;
    std::map<int, int, std::less<int>, MapAllocator> pool_factorials;

    int factorial = 1;
    for (int i = 0; i < 10; ++i)
    {
        if (i > 0)
        {
            factorial *= i;
        }
        factorials.emplace(i, factorial);
        pool_factorials.emplace(i, factorial);
    }

    for (const auto &entry : factorials)
    {
        std::cout << entry.first << ' ' << entry.second << '\n';
    }

    for (const auto &entry : pool_factorials)
    {
        std::cout << entry.first << ' ' << entry.second << '\n';
    }

    SLLContainer<int> numbers;
    SLLContainer<int, PoolAllocator<int, 10>> pool_numbers;

    for (int i = 0; i < 10; ++i)
    {
        numbers.push_back(i);
        pool_numbers.push_back(i);
    }

    for (int number : numbers)
    {
        std::cout << number << '\n';
    }

    for (int number : pool_numbers)
    {
        std::cout << number << '\n';
    }

    return 0;
}
