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
         typename pivot_func = decltype(pivot::random<BiIt>),
         typename Cmp = std::less<>>
void quicksort_1(BiIt first, BiIt last, pivot_func pf = pivot::random, Cmp cmp = Cmp{})
{
    if (first < last)
    {
        auto pivot = pf(first, last);
        auto pivot_value = *pivot;
        std::iter_swap(first, pivot);

        auto greater_than_pivot = std::partition(std::next(first), last, [pivot_value, cmp](const auto& val) {
            return cmp(val, pivot_value);
        });

        std::iter_swap(std::prev(greater_than_pivot), first);

        quicksort_1(first, std::prev(greater_than_pivot), pf, cmp);
        quicksort_1(greater_than_pivot, last, pf, cmp);
    }
}

//#lastif // include guard

int main()
{
    std::vector<int> input {5,3,1,2,5,6,7,8,12,4,2,3,5,1,3,5,0};
    quicksort_1(input.begin(), input.end());

    for (const auto& i : input)
        std::cout << i << " ";
    std::cout << '\n';
}
