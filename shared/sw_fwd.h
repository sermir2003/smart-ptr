#pragma once

#include <exception>

////////////////////////////////////////////////////////////

class ControlBlockBase {
public:
    ControlBlockBase() : cnt_strong_ref_(1), cnt_weak_ref_(0) {
    }
    virtual ~ControlBlockBase() = default;
    void IncreaseStrong() {
        ++cnt_strong_ref_;
    }
    virtual void DeleteSource() = 0;
    void DecreaseStrong() {
        --cnt_strong_ref_;
        if (cnt_strong_ref_ == 0) {
            DeleteSource();
        }
        if (cnt_strong_ref_ == 0 && cnt_weak_ref_ == 0) {
            delete this;
        }
    }
    int GetCntStrong() const {
        return cnt_strong_ref_;
    }
    void IncreaseWeak() {
        ++cnt_weak_ref_;
    }
    void DecreaseWeak() {
        --cnt_weak_ref_;
        if (cnt_strong_ref_ == 0 && cnt_weak_ref_ == 0) {
            delete this;
        }
    }
    bool IsResourceAlive() const {
        return cnt_strong_ref_ != 0;
    }

private:
    size_t cnt_strong_ref_;
    size_t cnt_weak_ref_;
};

template <typename T>
class ControlBlockPointer : public ControlBlockBase {
public:
    explicit ControlBlockPointer(T* ptr) : ControlBlockBase(), ptr_(ptr) {
    }
    ~ControlBlockPointer() override {
        if (ptr_) {
            delete ptr_;
            ptr_ = nullptr;
        }
    }
    void DeleteSource() override {
        if (ptr_) {
            delete ptr_;
            ptr_ = nullptr;
        }
    }

private:
    T* ptr_;
};

template <typename T>
class ControlBlockEmplace : public ControlBlockBase {
public:
    template <typename... Args>
    explicit ControlBlockEmplace(Args&&... args) : ControlBlockBase() {
        new (&storage_) T{std::forward<Args>(args)...};
        alive_ = true;
    }
    T* GetPtr() {
        return reinterpret_cast<T*>(&storage_);
    }
    ~ControlBlockEmplace() override {
        if (alive_) {
            GetPtr()->~T();
            alive_ = false;
        }
    }
    void DeleteSource() override {
        if (alive_) {
            GetPtr()->~T();
            alive_ = false;
        }
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
    bool alive_;
};

////////////////////////////////////////////////////////////

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
