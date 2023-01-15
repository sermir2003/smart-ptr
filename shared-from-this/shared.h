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
    template <typename K, typename S>
    friend inline bool operator==(const SharedPtr<K>& left, const SharedPtr<S>& right);
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

    template <typename Y>
    void InitWeakThis(EnableSharedFromThis<Y>* e);

    void PerhapsInitWeakThis();
    ControlBlockBase* control_;
    T* ptr_;
};
template <typename T>
SharedPtr<T>::SharedPtr() : control_(nullptr), ptr_(nullptr) {
    PerhapsInitWeakThis();
}
template <typename T>
SharedPtr<T>::SharedPtr(std::nullptr_t) : control_(nullptr), ptr_(nullptr) {
    PerhapsInitWeakThis();
}
template <typename T>
template <typename Y>
SharedPtr<T>::SharedPtr(Y* ptr) : control_(new ControlBlockPointer(ptr)), ptr_(ptr) {
    PerhapsInitWeakThis();
}
template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other) : control_(other.control_), ptr_(other.ptr_) {
    ControlIncreaseStrong();
    PerhapsInitWeakThis();
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
    PerhapsInitWeakThis();
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
    PerhapsInitWeakThis();
}
template <typename T>
SharedPtr<T>::SharedPtr(T* ptr, ControlBlockBase* block) : control_(block), ptr_(ptr) {
    PerhapsInitWeakThis();
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
template <typename T>
template <typename Y>
void SharedPtr<T>::InitWeakThis(EnableSharedFromThis<Y>* e) {
    e->weak_this_ = *this;
}
template <typename T>
void SharedPtr<T>::PerhapsInitWeakThis() {
    if constexpr (std::is_convertible_v<T, EnableSharedFromThisBase>) {
        InitWeakThis(this->ptr_);
    }
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockEmplace<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block->GetPtr(), block);
}

template <typename K, typename S>
inline bool operator==(const SharedPtr<K>& left, const SharedPtr<S>& right) {
    return left.control_ && right.control_ && left.control_ == right.control_;
}

class EnableSharedFromThisBase {};

template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
    // private:
    WeakPtr<T> weak_this_;
};
template <typename T>
SharedPtr<T> EnableSharedFromThis<T>::SharedFromThis() {
    return SharedPtr<T>(weak_this_);
}
template <typename T>
SharedPtr<const T> EnableSharedFromThis<T>::SharedFromThis() const {
    return SharedPtr<const T>(weak_this_);
}
template <typename T>
WeakPtr<T> EnableSharedFromThis<T>::WeakFromThis() noexcept {
    return WeakPtr<T>(weak_this_);
}
template <typename T>
WeakPtr<const T> EnableSharedFromThis<T>::WeakFromThis() const noexcept {
    return WeakPtr<const T>(weak_this_);
}
