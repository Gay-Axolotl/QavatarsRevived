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
    //
    // Same-type overload: vector element type IS the ArrayW element type
    // (e.g. std::vector<UnityEngine::BoneWeight> -> ArrayW<UnityEngine::BoneWeight>,
    // std::vector<UnityEngine::Matrix4x4> -> ArrayW<UnityEngine::Matrix4x4>).
    template <class T>
    inline ArrayW<T> ToArrayW(const std::vector<T>& vec)
    {
        ArrayW<T> arr(vec.size());
        for (size_t i = 0; i < vec.size(); i++) {
            arr[i] = vec[i];
        }
        return arr;
    }

    // Converting overload: vector element type differs from the ArrayW
    // element type and needs an explicit per-element conversion -- this is
    // required for e.g. std::vector<Sombrero::FastVector3> ->
    // ArrayW<UnityEngine::Vector3>, since ArrayW<T> has no cross-type
    // conversion even when T and U are otherwise compatible in expressions.
    // Pass a lambda/function converting a single U to a single T.
    template <class T, class U, class Converter>
    inline ArrayW<T> ToArrayW(const std::vector<U>& vec, Converter convert)
    {
        ArrayW<T> arr(vec.size());
        for (size_t i = 0; i < vec.size(); i++) {
            arr[i] = convert(vec[i]);
        }
        return arr;
    }
}
