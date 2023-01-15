#pragma once

#include "compressed_pair.h"
// #include <utility>

#include <cstddef>  // std::nullptr_t
#include "common/my_int.h"

template <typename T>
struct DefaultDeleter {
    DefaultDeleter() = default;
    template <typename U>
    DefaultDeleter(const DefaultDeleter<U>& other) {
    }
    void operator()(T* ptr) const {
        delete ptr;
    }
};
template <typename T>
struct DefaultDeleter<T[]> {
    DefaultDeleter() = default;
    template <typename U>
    DefaultDeleter(const DefaultDeleter<U>& other) {
    }
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr);

    UniquePtr(T* ptr, const Deleter& deleter);
    UniquePtr(T* ptr, Deleter&& deleter);

    UniquePtr(UniquePtr&& other) noexcept;

    template <typename U, typename E>
    friend class UniquePtr;
    //---
    template <typename U, typename E>
    UniquePtr(UniquePtr<U, E>&& other);

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept;

    template <typename U, typename E>
    friend class UniquePtr;
    //---
    template <typename U, typename E>
    UniquePtr& operator=(UniquePtr<U, E>&& other);

    UniquePtr& operator=(std::nullptr_t);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release();
    void Reset(T* ptr = nullptr);
    void Swap(UniquePtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const;
    Deleter& GetDeleter();
    const Deleter& GetDeleter() const;
    explicit operator bool() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const;
    T* operator->() const;

private:
    CompressedPair<T*, Deleter> data_;
    void Clear();
};
template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(T* ptr) {
    data_.First() = ptr;
    data_.Second() = Deleter();
}
template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(T* ptr, const Deleter& deleter) {
    data_.First() = ptr;
    data_.Second() = deleter;
}
template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(T* ptr, Deleter&& deleter) {
    data_.First() = ptr;
    data_.Second() = std::move(deleter);
}
template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(UniquePtr&& other) noexcept {
    data_.First() = nullptr;
    data_.Second() = Deleter();
    std::swap(data_, other.data_);
}
template <typename T, typename Deleter>
template <typename U, typename E>
UniquePtr<T, Deleter>::UniquePtr(UniquePtr<U, E>&& other) {
    data_.First() = other.data_.First();
    other.data_.First() = nullptr;
    data_.Second() = std::move(other.data_.Second());
}
template <typename T, typename Deleter>
void UniquePtr<T, Deleter>::Clear() {
    if (data_.First()) {
        data_.Second().operator()(data_.First());
    }
    data_.First() = nullptr;
}
template <typename T, typename Deleter>
UniquePtr<T, Deleter>& UniquePtr<T, Deleter>::operator=(UniquePtr&& other) noexcept {
    if (&other == this) {
        return *this;
    }
    Clear();
    std::swap(data_, other.data_);
    return *this;
}
template <typename T, typename Deleter>
template <typename U, typename E>
UniquePtr<T, Deleter>& UniquePtr<T, Deleter>::operator=(UniquePtr<U, E>&& other) {
    //    if (&other == this) {  // TODO: find out
    //        return *this;
    //    }
    Clear();
    data_.First() = other.data_.First();
    other.data_.First() = nullptr;
    data_.Second() = std::move(other.data_.Second());
    return *this;
}
template <typename T, typename Deleter>
UniquePtr<T, Deleter>& UniquePtr<T, Deleter>::operator=(std::nullptr_t) {
    Clear();
    return *this;
}
template <typename T, typename Deleter>
UniquePtr<T, Deleter>::~UniquePtr() {
    Clear();
}
template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::Release() {
    T* ptr = data_.First();
    data_.First() = nullptr;
    return ptr;
}
template <typename T, typename Deleter>
void UniquePtr<T, Deleter>::Reset(T* ptr) {
    T* old_ptr = data_.First();
    data_.First() = ptr;
    if (old_ptr) {
        data_.Second().operator()(old_ptr);
    }
}
template <typename T, typename Deleter>
void UniquePtr<T, Deleter>::Swap(UniquePtr& other) {
    UniquePtr<T, Deleter> tmp = std::move(*this);
    *this = std::move(other);
    other = std::move(tmp);
}
template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::Get() const {
    return data_.First();
}
template <typename T, typename Deleter>
Deleter& UniquePtr<T, Deleter>::GetDeleter() {
    return data_.Second();
}
template <typename T, typename Deleter>
const Deleter& UniquePtr<T, Deleter>::GetDeleter() const {
    return data_.Second();
}
template <typename T, typename Deleter>
UniquePtr<T, Deleter>::operator bool() const {
    return data_.First() != nullptr;
}
template <typename T, typename Deleter>
T& UniquePtr<T, Deleter>::operator*() const {
    return *data_.First();
}
template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::operator->() const {
    return data_.First();
}

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr);

    UniquePtr(T* ptr, const Deleter& deleter);
    UniquePtr(T* ptr, Deleter&& deleter);

    UniquePtr(UniquePtr&& other) noexcept;

    template <typename U, typename E>
    friend class UniquePtr;
    //---
    template <typename U, typename E>
    UniquePtr(UniquePtr<U, E>&& other);

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept;

    template <typename U, typename E>
    friend class UniquePtr;
    //---
    template <typename U, typename E>
    UniquePtr& operator=(UniquePtr<U, E>&& other);

    UniquePtr& operator=(std::nullptr_t);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release();
    void Reset(T* ptr = nullptr);
    void Swap(UniquePtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const;
    Deleter& GetDeleter();
    const Deleter& GetDeleter() const;
    explicit operator bool() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    //    T& operator*() const;
    //    T* operator->() const;
    T& operator[](size_t ind);
    const T& operator[](size_t ind) const;

private:
    CompressedPair<T*, Deleter> data_;
    void Clear();
};
template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(T* ptr) {
    data_.First() = ptr;
    data_.Second() = Deleter();
}
template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(T* ptr, const Deleter& deleter) {
    data_.First() = ptr;
    data_.Second() = deleter;
}
template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(T* ptr, Deleter&& deleter) {
    data_.First() = ptr;
    data_.Second() = std::move(deleter);
}
template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(UniquePtr&& other) noexcept {
    data_.First() = nullptr;
    data_.Second() = Deleter();
    std::swap(data_, other.data_);
}
template <typename T, typename Deleter>
template <typename U, typename E>
UniquePtr<T[], Deleter>::UniquePtr(UniquePtr<U, E>&& other) {
    data_.First() = other.data_.First();
    other.data_.First() = nullptr;
    data_.Second() = std::move(other.data_.Second());
}
template <typename T, typename Deleter>
void UniquePtr<T[], Deleter>::Clear() {
    if (data_.First()) {
        data_.Second().operator()(data_.First());
    }
    data_.First() = nullptr;
}
template <typename T, typename Deleter>
UniquePtr<T[], Deleter>& UniquePtr<T[], Deleter>::operator=(UniquePtr&& other) noexcept {
    if (&other == this) {
        return *this;
    }
    Clear();
    std::swap(data_, other.data_);
    return *this;
}
template <typename T, typename Deleter>
template <typename U, typename E>
UniquePtr<T[], Deleter>& UniquePtr<T[], Deleter>::operator=(UniquePtr<U, E>&& other) {
    //    if (&other == this) {  // TODO: find out
    //        return *this;
    //    }
    Clear();
    data_.First() = other.data_.First();
    other.data_.First() = nullptr;
    data_.Second() = std::move(other.data_.Second());
    return *this;
}
template <typename T, typename Deleter>
UniquePtr<T[], Deleter>& UniquePtr<T[], Deleter>::operator=(std::nullptr_t) {
    Clear();
    return *this;
}
template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::~UniquePtr() {
    Clear();
}
template <typename T, typename Deleter>
T* UniquePtr<T[], Deleter>::Release() {
    T* ptr = data_.First();
    data_.First() = nullptr;
    return ptr;
}
template <typename T, typename Deleter>
void UniquePtr<T[], Deleter>::Reset(T* ptr) {
    T* old_ptr = data_.First();
    data_.First() = ptr;
    if (old_ptr) {
        data_.Second().operator()(old_ptr);
    }
}
template <typename T, typename Deleter>
void UniquePtr<T[], Deleter>::Swap(UniquePtr& other) {
    UniquePtr<T, Deleter> tmp = std::move(*this);
    *this = std::move(other);
    other = std::move(tmp);
}
template <typename T, typename Deleter>
T* UniquePtr<T[], Deleter>::Get() const {
    return data_.First();
}
template <typename T, typename Deleter>
Deleter& UniquePtr<T[], Deleter>::GetDeleter() {
    return data_.Second();
}
template <typename T, typename Deleter>
const Deleter& UniquePtr<T[], Deleter>::GetDeleter() const {
    return data_.Second();
}
template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::operator bool() const {
    return data_.First() != nullptr;
}
// template <typename T, typename Deleter>
// T& UniquePtr<T[], Deleter>::operator*() const {
//     return *data_.First();
// }
// template <typename T, typename Deleter>
// T* UniquePtr<T[], Deleter>::operator->() const {
//     return data_.First();
// }
template <typename T, typename Deleter>
T& UniquePtr<T[], Deleter>::operator[](size_t ind) {
    return *(data_.First() + ind);
}
template <typename T, typename Deleter>
const T& UniquePtr<T[], Deleter>::operator[](size_t ind) const {
    return *(data_.First() + ind);
}

template <typename Deleter>
class UniquePtr<void, Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(void* ptr = nullptr);

    UniquePtr(void* ptr, const Deleter& deleter);
    UniquePtr(void* ptr, Deleter&& deleter);

    UniquePtr(UniquePtr&& other) noexcept;

    template <typename U, typename E>
    friend class UniquePtr;
    //---
    template <typename U, typename E>
    UniquePtr(UniquePtr<U, E>&& other);

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept;

    template <typename U, typename E>
    friend class UniquePtr;
    //---
    template <typename U, typename E>
    UniquePtr& operator=(UniquePtr<U, E>&& other);

    UniquePtr& operator=(std::nullptr_t);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void* Release();
    void Reset(void* ptr = nullptr);
    void Swap(UniquePtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    void* Get() const;
    Deleter& GetDeleter();
    const Deleter& GetDeleter() const;
    explicit operator bool() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

private:
    CompressedPair<void*, Deleter> data_;
    void Clear();
};
template <typename Deleter>
UniquePtr<void, Deleter>::UniquePtr(void* ptr) {
    data_.First() = ptr;
    data_.Second() = Deleter();
}
template <typename Deleter>
UniquePtr<void, Deleter>::UniquePtr(void* ptr, const Deleter& deleter) {
    data_.First() = ptr;
    data_.Second() = deleter;
}
template <typename Deleter>
UniquePtr<void, Deleter>::UniquePtr(void* ptr, Deleter&& deleter) {
    data_.First() = ptr;
    data_.Second() = std::move(deleter);
}
template <typename Deleter>
UniquePtr<void, Deleter>::UniquePtr(UniquePtr&& other) noexcept {
    data_.First() = nullptr;
    data_.Second() = Deleter();
    std::swap(data_, other.data_);
}
template <typename Deleter>
template <typename U, typename E>
UniquePtr<void, Deleter>::UniquePtr(UniquePtr<U, E>&& other) {
    data_.First() = other.data_.First();
    other.data_.First() = nullptr;
    data_.Second() = std::move(other.data_.Second());
}
template <typename Deleter>
void UniquePtr<void, Deleter>::Clear() {
    if (data_.First()) {
        data_.Second().operator()(data_.First());
    }
    data_.First() = nullptr;
}
template <typename Deleter>
UniquePtr<void, Deleter>& UniquePtr<void, Deleter>::operator=(UniquePtr&& other) noexcept {
    if (&other == this) {
        return *this;
    }
    Clear();
    std::swap(data_, other.data_);
    return *this;
}
template <typename Deleter>
template <typename U, typename E>
UniquePtr<void, Deleter>& UniquePtr<void, Deleter>::operator=(UniquePtr<U, E>&& other) {
    //    if (&other == this) {  // TODO: find out
    //        return *this;
    //    }
    Clear();
    data_.First() = other.data_.First();
    other.data_.First() = nullptr;
    data_.Second() = std::move(other.data_.Second());
    return *this;
}
template <typename Deleter>
UniquePtr<void, Deleter>& UniquePtr<void, Deleter>::operator=(std::nullptr_t) {
    Clear();
    return *this;
}
template <typename Deleter>
UniquePtr<void, Deleter>::~UniquePtr() {
    Clear();
}
template <typename Deleter>
void* UniquePtr<void, Deleter>::Release() {
    void* ptr = data_.First();
    data_.First() = nullptr;
    return ptr;
}
template <typename Deleter>
void UniquePtr<void, Deleter>::Reset(void* ptr) {
    void* old_ptr = data_.First();
    data_.First() = ptr;
    if (old_ptr) {
        data_.Second().operator()(old_ptr);
    }
}
template <typename Deleter>
void UniquePtr<void, Deleter>::Swap(UniquePtr& other) {
    UniquePtr<void, Deleter> tmp = std::move(*this);
    *this = std::move(other);
    other = std::move(tmp);
}
template <typename Deleter>
void* UniquePtr<void, Deleter>::Get() const {
    return data_.First();
}
template <typename Deleter>
Deleter& UniquePtr<void, Deleter>::GetDeleter() {
    return data_.Second();
}
template <typename Deleter>
const Deleter& UniquePtr<void, Deleter>::GetDeleter() const {
    return data_.Second();
}
template <typename Deleter>
UniquePtr<void, Deleter>::operator bool() const {
    return data_.First() != nullptr;
}
