// The MIT License (MIT)

// Copyright (c) 2017 Daniel Feist

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <random>
#include <vector>
#include <thread>
#include <iostream>
#include <algorithm>

namespace detail
{

template<typename It, typename Cmp>
void insertion_sort(It first, It last, Cmp cmp)
{
    for (auto begin = first; begin != last; ++begin)
    {
        const auto insertion = std::upper_bound(first, begin, *begin, cmp);
        std::rotate(insertion, begin, std::next(begin));
    }
}

template<typename It>
void insertion_sort(It first, It last)
{
    insertion_sort(first, last, std::less<>());
}

template<typename It>
void sort_small_instances(It first, It last)
{
    if (std::distance(first, last) < 40)
    {
        insertion_sort(first, last);
        return;
    }
}

} // namespace detail

namespace pivot
{

template<typename It, typename RandomGenerator>
It random(It first, It last, RandomGenerator& g)
{
    std::uniform_int_distribution<> dis(0, std::distance(first, last) - 1);
    std::advance(first, dis(g));
    return first;
}

template<typename It>
It random(It first, It last)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return random(first, last, gen);
}

template<typename It>
It median(It first, It last)
{
    return std::next(first, std::distance(first,last)/2);
}

} // namespace selector

template<typename BiIt,
         typename Pivot_func = decltype(pivot::random<BiIt>),
         typename Cmp = std::less<>>
void sequential_quicksort(BiIt first, BiIt last, Pivot_func pivot_func = pivot::random, Cmp cmp = Cmp{})
{
    if (std::distance(first, last) < 2)
        return;

    //detail::sort_small_instances(first, last); // optimization

    auto pivot = pivot_func(first, last);
    auto pivot_value = *pivot;
    std::iter_swap(first, pivot);

    auto greater_than_pivot = std::partition(std::next(first), last, [pivot_value, cmp](const auto& val) {
        return cmp(val, pivot_value);
    });

    std::iter_swap(std::prev(greater_than_pivot), first);

    sequential_quicksort(first, std::prev(greater_than_pivot), pivot_func, cmp);
    sequential_quicksort(greater_than_pivot, last, pivot_func, cmp);
}

template<typename BiIt,
         typename Pivot_func = decltype(pivot::random<BiIt>),
         typename Cmp = std::less<>>
void naive_parallel_quicksort(BiIt first, BiIt last, int depth = 0, Pivot_func pivot_func = pivot::random, Cmp cmp = Cmp{})
{
    if (std::distance(first, last) < 2)
        return;

    //detail::sort_small_instances(first, last); // optimization

    auto pivot = pivot_func(first, last);
    auto pivot_value = *pivot;
    std::iter_swap(first, pivot);

    auto greater_than_pivot = std::partition(std::next(first), last, [pivot_value, cmp](const auto& val) {
        return cmp(val, pivot_value);
    });

    std::iter_swap(std::prev(greater_than_pivot), first);

    if (depth < 5)
    {
        std::thread t1([=]() {
            naive_parallel_quicksort(first, std::prev(greater_than_pivot), depth+1, pivot_func, cmp);
        });

        naive_parallel_quicksort(greater_than_pivot, last, depth+1, pivot_func, cmp);
        t1.join();
    }
    else
    {
        naive_parallel_quicksort(first, std::prev(greater_than_pivot), depth+1, pivot_func, cmp);
        naive_parallel_quicksort(greater_than_pivot, last, depth+1, pivot_func, cmp);
    }
}

namespace helpers
{
    template <class C>
    void insert_random_ints(C& c)
    {
        std::random_device rd;
        std::mt19937 g(rd());

        std::iota(c.begin(), c.end(), 0);
        std::shuffle(c.begin(), c.end(), g);
    }
} // namespace helpers

#define TEST_ALGORITHM(NAME)                                                    \
template<class I>                                                               \
void test_ ## NAME (I first, I last)                                            \
{                                                                               \
    std::for_each(first, last, [](auto t) {                                     \
        NAME ## _quicksort(begin(t), end(t));                                   \
        std::cout << std::boolalpha << std::is_sorted(begin(t), end(t)) << ","; \
    });                                                                         \
    std::cout << "\n";                                                          \
}

TEST_ALGORITHM(sequential)
TEST_ALGORITHM(naive_parallel)

int main()
{
    using std::vector;

    auto empty = vector<int> {};
    auto singleton = vector<int> {1};
    auto doubleton = vector<int> {9,4};
    auto random = vector<int> {8,1,4,2,6,0,9,5,3,7};
    auto sorted = vector<int> {0,1,2,3,4,5,6,7,8,9};
    auto reversed = vector<int> {9,8,7,6,5,4,3,2,1};
    auto almost_sorted = vector<int> {0,1,2,3,5,4,6,9,8};
    auto many_unique = vector<int> {1,2,0,1,0,0,2,2,1};
    auto wave = vector<int> {1,2,3,2,1,2,3,4,5,6,7,6,5,4,3,2,1};

    auto random100 = vector<int> (100);
    helpers::insert_random_ints(random100);

    auto random1000 = vector<int> (1000);
    helpers::insert_random_ints(random1000);

    auto inputs = vector<vector<int>> {empty, singleton, doubleton,
                                       random, sorted, reversed,
                                       almost_sorted, many_unique,
                                       wave, random100, random1000};

    test_sequential(std::begin(inputs), std::end(inputs));
    test_naive_parallel(std::begin(inputs), std::end(inputs));
}
