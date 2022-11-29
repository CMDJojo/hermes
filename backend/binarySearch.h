#pragma once
#include <stdint.h>
#include <iostream>
#include <vector>
#include <functional>

namespace BinarySearch {
    enum class ResultStatus : uint8_t {
        Found,
        NotFound,
    };

    class SearchResult {
    public:
        const size_t index;
        const ResultStatus status;
        SearchResult(size_t index, ResultStatus status);
    };

    std::ostream& operator<<(std::ostream& os, const SearchResult& result);

    enum class ComparatorResult : uint8_t {
        LT,
        EQ,
        GT,
    };

    template<typename T>
    SearchResult binarySearch(const std::vector<T>& vec, std::function<ComparatorResult(const T& item)>comparator) {
        return binarySearch(vec, comparator, 0, vec.size());
    }

    template<typename T>
    SearchResult binarySearch(const std::vector<T>& vec, std::function<ComparatorResult(const T& item)>comparator, size_t low_bound, size_t high_bound) {
        size_t middle = 0;
        high_bound--;
        while (low_bound < high_bound) {
            middle = (low_bound + high_bound) / 2;
            ComparatorResult result = comparator(vec[middle]);
            if (result == ComparatorResult::LT) {
                high_bound = middle;
            } else if (result == ComparatorResult::GT) {
                low_bound = middle + 1;
            } else {
                return SearchResult(middle, ResultStatus::Found);
            }
        }
        ComparatorResult result = comparator(vec[middle]);
        if (result == ComparatorResult::LT) return SearchResult(middle, ResultStatus::NotFound);
        return SearchResult(middle + 1, ResultStatus::NotFound);
    }

}

