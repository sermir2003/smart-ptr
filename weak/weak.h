#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr();

    WeakPtr(const WeakPtr& other);
    WeakPtr(WeakPtr&& other);

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other);

    template <typename Y>
    WeakPtr(WeakPtr<Y>&& other);

    template <typename Y>
    WeakPtr(const SharedPtr<Y>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s  // TODO: probably operator-s with <Y>

    WeakPtr& operator=(const WeakPtr& other);
    WeakPtr& operator=(WeakPtr&& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset();
    void Swap(WeakPtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const;
    bool Expired() const;
    SharedPtr<T> Lock() const;

private:
    void ControlIncreaseWeak();
    void ControlDecreaseWeak();
    int ControlGetCntStrong() const;
    void Clear();
    ControlBlockBase* control_;
    T* ptr_;
};
template <typename T>
void WeakPtr<T>::Clear() {
    ControlDecreaseWeak();
    control_ = nullptr;
    ptr_ = nullptr;
}
template <typename T>
WeakPtr<T>::~WeakPtr() {
    Clear();
}
template <typename T>
void WeakPtr<T>::ControlIncreaseWeak() {
    if (control_) {
        control_->IncreaseWeak();
    }
}
template <typename T>
void WeakPtr<T>::ControlDecreaseWeak() {
    if (control_) {
        control_->DecreaseWeak();
    }
}
template <typename T>
int WeakPtr<T>::ControlGetCntStrong() const {
    return (control_ ? control_->GetCntStrong() : 0);
}
template <typename T>
WeakPtr<T>::WeakPtr() : control_(nullptr), ptr_(nullptr) {
}
template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other) : control_(other.control_), ptr_(other.ptr_) {
    ControlIncreaseWeak();
}
template <typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& other) : control_(other.control_), ptr_(other.ptr_) {
    other.control_ = nullptr;
    other.ptr_ = nullptr;
}
template <typename T>
template <typename Y>
WeakPtr<T>::WeakPtr(const WeakPtr<Y>& other) : control_(other.control_), ptr_(other.ptr_) {
    ControlIncreaseWeak();
}
template <typename T>
template <typename Y>
WeakPtr<T>::WeakPtr(WeakPtr<Y>&& other) : control_(other.control_), ptr_(other.ptr_) {
    other.control_ = nullptr;
    other.ptr_ = nullptr;
}
template <typename T>
template <typename Y>
WeakPtr<T>::WeakPtr(const SharedPtr<Y>& other) : control_(other.control_), ptr_(other.ptr_) {
    ControlIncreaseWeak();
}
template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) {
    if (&other == this) {
        return *this;
    }
    Clear();
    control_ = other.control_;
    ptr_ = other.ptr_;
    ControlIncreaseWeak();
    return *this;
}
template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other) {
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
void WeakPtr<T>::Reset() {
    Clear();
}
template <typename T>
void WeakPtr<T>::Swap(WeakPtr& other) {
    WeakPtr<T> tmp = std::move(*this);
    *this = std::move(other);
    other = std::move(tmp);
}
template <typename T>
size_t WeakPtr<T>::UseCount() const {
    return ControlGetCntStrong();
}
template <typename T>
bool WeakPtr<T>::Expired() const {
    return ControlGetCntStrong() == 0;
}
template <typename T>
SharedPtr<T> WeakPtr<T>::Lock() const {
    if (control_ && control_->IsResourceAlive()) {
        control_->IncreaseStrong();
        return SharedPtr(ptr_, control_);
    }
    return SharedPtr<T>(nullptr, nullptr);
}
