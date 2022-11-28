#include <functional>
#include "binarySearch.h"

BinarySearch::SearchResult::SearchResult(size_t index, ResultStatus status) : index(index), status(status) {};

std::ostream& operator<<(std::ostream& os, const BinarySearch::SearchResult& result) {
    if (result.status == BinarySearch::ResultStatus::Found) {
        return std::cout << "Found(" << result.index << ")" << std::endl;
    } else {
        return std::cout << "NotFound(" << result.index << ")" << std::endl;
    }
}
