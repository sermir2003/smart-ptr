#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    void IncRef();
    void DecRef();
    size_t RefCount() const;
    void Reset();

private:
    size_t count_ = 0;
};
void SimpleCounter::IncRef() {
    ++count_;
}
void SimpleCounter::DecRef() {
    --count_;
}
size_t SimpleCounter::RefCount() const {
    return count_;
}
void SimpleCounter::Reset() {
    count_ = 0;
}

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef();

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef();

    // Get current counter value (the number of strong references).
    size_t RefCount() const;

    //    ~RefCounted();

    auto operator=(const RefCounted& other) {
        if (&other == this) {
            return *this;
        }
        counter_.Reset();
        counter_.IncRef();
        return *this;
    }

private:
    Counter counter_;
};
template <typename Derived, typename Counter, typename Deleter>
void RefCounted<Derived, Counter, Deleter>::IncRef() {
    counter_.IncRef();
}
template <typename Derived, typename Counter, typename Deleter>
void RefCounted<Derived, Counter, Deleter>::DecRef() {
    counter_.DecRef();
    if (counter_.RefCount() == 0) {
        Deleter::Destroy(static_cast<Derived*>(this));
    }
}
template <typename Derived, typename Counter, typename Deleter>
size_t RefCounted<Derived, Counter, Deleter>::RefCount() const {
    return counter_.RefCount();
}
// template <typename Derived, typename Counter, typename Deleter>
// RefCounted<Derived, Counter, Deleter>::~RefCounted() {
//     Deleter::Destroy(static_cast<Derived*>(this));
// }

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr();
    IntrusivePtr(std::nullptr_t);
    IntrusivePtr(T* ptr);

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other);

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other);

    IntrusivePtr(const IntrusivePtr& other);
    IntrusivePtr(IntrusivePtr&& other);

    //    template <typename ...Args>
    //    IntrusivePtr(Args&& ...args) {
    //        *this = IntrusivePtr<T>(new T{std::forward<Args>(args)...});
    //    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other);
    IntrusivePtr& operator=(IntrusivePtr&& other);

    // Destructor
    ~IntrusivePtr();

    // Modifiers
    void Reset();
    void Reset(T* ptr);
    void Swap(IntrusivePtr& other);

    // Observers
    T* Get() const;
    T& operator*() const;
    T* operator->() const;
    size_t UseCount() const;
    explicit operator bool() const;

private:
    void Clear();
    void IncRef();
    size_t RefCount() const;

private:
    T* ptr_;
};
template <typename T>
IntrusivePtr<T>::IntrusivePtr() : ptr_(nullptr) {
}
template <typename T>
IntrusivePtr<T>::IntrusivePtr(std::nullptr_t) : ptr_(nullptr) {
}
template <typename T>
IntrusivePtr<T>::IntrusivePtr(T* ptr) : ptr_(ptr) {
    IncRef();
}
template <typename T>
template <typename Y>
IntrusivePtr<T>::IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.ptr_) {
    IncRef();
}
template <typename T>
template <typename Y>
IntrusivePtr<T>::IntrusivePtr(IntrusivePtr<Y>&& other) : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
}
template <typename T>
IntrusivePtr<T>::IntrusivePtr(const IntrusivePtr& other) : ptr_(other.ptr_) {
    IncRef();
}
template <typename T>
IntrusivePtr<T>::IntrusivePtr(IntrusivePtr&& other) : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
}
template <typename T>
void IntrusivePtr<T>::Clear() {
    T* to_delete = ptr_;
    ptr_ = nullptr;
    if (to_delete) {
        to_delete->DecRef();
    }
}
template <typename T>
IntrusivePtr<T>& IntrusivePtr<T>::operator=(const IntrusivePtr& other) {
    if (&other == this) {
        return *this;
    }
    Clear();
    ptr_ = other.ptr_;
    IncRef();
    return *this;
}
template <typename T>
IntrusivePtr<T>& IntrusivePtr<T>::operator=(IntrusivePtr&& other) {
    if (&other == this) {
        return *this;
    }
    Clear();
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
}
template <typename T>
IntrusivePtr<T>::~IntrusivePtr() {
    Clear();
}
template <typename T>
void IntrusivePtr<T>::Reset() {
    Clear();
}
template <typename T>
void IntrusivePtr<T>::Reset(T* ptr) {
    Clear();
    *this = std::move(IntrusivePtr<T>(ptr));
}
template <typename T>
void IntrusivePtr<T>::Swap(IntrusivePtr& other) {
    IntrusivePtr<T> tmp = std::move(*this);
    *this = std::move(other);
    other = std::move(tmp);
}
template <typename T>
T* IntrusivePtr<T>::Get() const {
    return ptr_;
}
template <typename T>
T& IntrusivePtr<T>::operator*() const {
    return *ptr_;
}
template <typename T>
T* IntrusivePtr<T>::operator->() const {
    return ptr_;
}
template <typename T>
size_t IntrusivePtr<T>::UseCount() const {
    return RefCount();
}
template <typename T>
IntrusivePtr<T>::operator bool() const {
    return ptr_ != nullptr;
}
template <typename T>
void IntrusivePtr<T>::IncRef() {
    if (ptr_) {
        ptr_->IncRef();
    }
}
template <typename T>
size_t IntrusivePtr<T>::RefCount() const {
    return (ptr_ ? ptr_->RefCount() : 0);
}

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T{std::forward<Args>(args)...});
}
