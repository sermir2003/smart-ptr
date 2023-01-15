#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <type_traits>
#include <utility>

template <typename T>
class SharedPtr {
public:
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr();
    SharedPtr(std::nullptr_t);

    template <typename Y>
    explicit SharedPtr(Y* ptr);

    SharedPtr(const SharedPtr& other);
    SharedPtr(SharedPtr&& other);

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr);

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other);

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other);

    SharedPtr(T* ptr, ControlBlockBase* block);

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    //    template <typename Y>
    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other);
    SharedPtr& operator=(SharedPtr&& other);

    //    template <typename Y>
    //    SharedPtr<T>& operator=(const SharedPtr<Y>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset();

    template <typename Y>
    void Reset(Y* ptr);

    void Swap(SharedPtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const;
    T& operator*() const;
    T* operator->() const;
    size_t UseCount() const;
    explicit operator bool() const;

private:
    void ControlIncreaseStrong();
    void ControlDecreaseStrong();
    int ControlGetCntStrong() const;
    void Clear();
    ControlBlockBase* control_;
    T* ptr_;
};
template <typename T>
SharedPtr<T>::SharedPtr() : control_(nullptr), ptr_(nullptr) {
}
template <typename T>
SharedPtr<T>::SharedPtr(std::nullptr_t) : control_(nullptr), ptr_(nullptr) {
}
template <typename T>
template <typename Y>
SharedPtr<T>::SharedPtr(Y* ptr) : control_(new ControlBlockPointer(ptr)), ptr_(ptr) {
}
template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other) : control_(other.control_), ptr_(other.ptr_) {
    ControlIncreaseStrong();
}
template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other) : control_(other.control_), ptr_(other.ptr_) {
    other.control_ = nullptr;
    other.ptr_ = nullptr;
}
template <typename T>
template <typename Y>
SharedPtr<T>::SharedPtr(const SharedPtr<Y>& other) : control_(other.control_), ptr_(other.ptr_) {
    ControlIncreaseStrong();
}
template <typename T>
template <typename Y>
SharedPtr<T>::SharedPtr(SharedPtr<Y>&& other) : control_(other.control_), ptr_(other.ptr_) {
    other.control_ = nullptr;
    other.ptr_ = nullptr;
}
template <typename T>
template <typename Y>
SharedPtr<T>::SharedPtr(const SharedPtr<Y>& other, T* ptr) : control_(other.control_), ptr_(ptr) {
    ControlIncreaseStrong();
}
template <typename T>
SharedPtr<T>::SharedPtr(T* ptr, ControlBlockBase* block) : control_(block), ptr_(ptr) {
}
template <typename T>
void SharedPtr<T>::Clear() {
    ControlDecreaseStrong();
    control_ = nullptr;
    ptr_ = nullptr;
}
template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
    if (&other == this) {
        return *this;
    }
    Clear();
    control_ = other.control_;
    ptr_ = other.ptr_;
    ControlIncreaseStrong();
    return *this;
}
template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) {
    if (&other == this) {
        return *this;
    }
    Clear();
    control_ = other.control_;
    ptr_ = other.ptr_;
    other.control_ = nullptr;
    other.ptr_ = nullptr;
    return *this;
}
template <typename T>
SharedPtr<T>::~SharedPtr() {
    Clear();
}
template <typename T>
void SharedPtr<T>::Reset() {
    Clear();
}
template <typename T>
template <typename Y>
void SharedPtr<T>::Reset(Y* ptr) {
    Clear();
    *this = std::move(SharedPtr<Y>(ptr));
}
template <typename T>
void SharedPtr<T>::Swap(SharedPtr& other) {
    SharedPtr tmp = std::move(*this);
    *this = std::move(other);
    other = std::move(tmp);
}
template <typename T>
T* SharedPtr<T>::Get() const {
    return ptr_;
}
template <typename T>
T& SharedPtr<T>::operator*() const {
    return *ptr_;
}
template <typename T>
T* SharedPtr<T>::operator->() const {
    return ptr_;
}
template <typename T>
size_t SharedPtr<T>::UseCount() const {
    return ControlGetCntStrong();
}
template <typename T>
SharedPtr<T>::operator bool() const {
    return control_ != nullptr;
}
template <typename T>
void SharedPtr<T>::ControlIncreaseStrong() {
    if (control_) {
        control_->IncreaseStrong();
    }
}
template <typename T>
void SharedPtr<T>::ControlDecreaseStrong() {
    if (control_) {
        control_->DecreaseStrong();
    }
}
template <typename T>
int SharedPtr<T>::ControlGetCntStrong() const {
    return (control_ ? control_->GetCntStrong() : 0);
}
template <typename T>
// template <typename Y>
SharedPtr<T>::SharedPtr(const WeakPtr<T>& other) {
    if (!other.control_ || !other.control_->IsResourceAlive()) {
        throw BadWeakPtr();
    }
    control_ = other.control_;
    ptr_ = other.ptr_;
    ControlIncreaseStrong();
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockEmplace<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block->GetPtr(), block);
}

// template <typename T, typename U>
// inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);
