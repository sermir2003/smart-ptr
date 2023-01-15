#pragma once

#include <exception>

////////////////////////////////////////////////////////////

class EnableSharedFromThisBase;

template <typename T>
class EnableSharedFromThis;

class ControlBlockBase {
public:
    ControlBlockBase() : cnt_strong_ref_(1), cnt_weak_ref_(0), destruction_is_locked_(false) {
    }
    virtual ~ControlBlockBase() = default;
    void IncreaseStrong() {
        ++cnt_strong_ref_;
    }
    virtual void DeleteSource() = 0;
    void DecreaseStrong() {
        --cnt_strong_ref_;
        if (cnt_strong_ref_ == 0) {
            destruction_is_locked_ = true;
            DeleteSource();
            destruction_is_locked_ = false;
        }
        if (!destruction_is_locked_ && cnt_strong_ref_ == 0 && cnt_weak_ref_ == 0) {
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
        if (!destruction_is_locked_ && cnt_strong_ref_ == 0 && cnt_weak_ref_ == 0) {
            delete this;
        }
    }
    bool IsResourceAlive() const {
        return cnt_strong_ref_ != 0;
    }

private:
    size_t cnt_strong_ref_;
    size_t cnt_weak_ref_;
    bool destruction_is_locked_;
};

template <typename T>
class ControlBlockPointer : public ControlBlockBase {
public:
    explicit ControlBlockPointer(T* ptr) : ControlBlockBase(), ptr_(ptr) {
    }
    ~ControlBlockPointer() override {
        if (ptr_) {
            T* to_delete = ptr_;
            ptr_ = nullptr;
            delete to_delete;
        }
    }
    void DeleteSource() override {
        if (ptr_) {
            T* to_delete = ptr_;
            ptr_ = nullptr;
            delete to_delete;
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
            alive_ = false;
            GetPtr()->~T();
        }
    }
    void DeleteSource() override {
        if (alive_) {
            alive_ = false;
            GetPtr()->~T();
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
