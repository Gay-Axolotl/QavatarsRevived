#pragma once
#include "QavatarsRevived/Main.hpp"

namespace ArrayUtils {
    //borrowed from QuestUI
    template <class Out, class T, class Predicate>
    inline std::vector<Out> Select(std::vector<T> array, Predicate pred)
    {
        std::vector<Out> newArray(array.size()); 
        for (int i = 0; i < array.size(); i++) { 
            newArray[i] = pred(array[i]); 
        } 
        return newArray;
    }

    // Replacement for il2cpp_utils::vectorToArray, which no longer exists in
    // the current beatsaber-hook version this project targets (it existed in
    // the older 5.1.6 the original VRM-Qavatars mod was built against).
    // ArrayW<T>(size) + manual element copy is a pattern already proven to
    // compile elsewhere in this codebase (see gLTFImageReader.hpp), so this
    // uses the same approach rather than guessing at an unverified ArrayW
    // constructor overload.
    template <class T>
    inline ArrayW<T> ToArrayW(const std::vector<T>& vec)
    {
        ArrayW<T> arr(vec.size());
        for (size_t i = 0; i < vec.size(); i++) {
            arr[i] = vec[i];
        }
        return arr;
    }
}
