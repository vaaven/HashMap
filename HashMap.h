//
// Created by vaaven on 22.01.2023.
// Robin Hood Hash Map implementation
//
// In my implementation it's important that size_ * 2 <= capacity_ <= size_ * 4 (except small cases)
// It's the reason why we can say that O(capacity_) = O(size_)
//

#include <functional>
#include <algorithm>
#include <exception>
#include <vector>
#include <iostream>

#ifndef MY_HASH_MAP_HASH_MAP_H
#define MY_HASH_MAP_HASH_MAP_H

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
public:
    size_t capacity_{0};
    size_t size_{0};
    std::vector<std::pair<KeyType, ValueType>> val_;
    std::vector<size_t> PSL;
    std::vector<size_t> status; // 0 == empty; 1 == used; 2 == deleted
    Hash hash_;

    void _rebuild() {
        std::vector<std::pair<KeyType, ValueType>> vec;
        for (iterator it = begin(); it != end(); ++it) {
            vec.push_back(std::make_pair((KeyType) it->first, (ValueType) it->second));
        }
        val_.clear();
        PSL.clear();
        status.clear();
        if (size_ * 2 >= capacity_) {
            size_ = 0;
            capacity_ *= 2;
            val_.resize(capacity_);
            PSL.assign(capacity_, 0);
            status.assign(capacity_, 0);
            for (std::pair<KeyType, ValueType> i: vec) {
                insert(i);
            }
        } else {
            size_ = 0;
            capacity_ /= 2;
            val_.resize(capacity_);
            PSL.assign(capacity_, 0);
            status.assign(capacity_, 0);
            for (std::pair<KeyType, ValueType> i: vec) {
                insert(i);
            }
        }
    }

public:

    class iterator {
    private:
        HashMap *map_;
        size_t ind_{};
    public:

        iterator() = default;

        iterator(HashMap *map, const size_t &ind) : map_(map), ind_(ind) {}

        iterator &operator++() {
            if (ind_ == map_->capacity_)
                return *this;
            ++ind_;
            while (ind_ < map_->capacity_ && map_->status[ind_] != 1)
                ++ind_;
            return *this;
        }

        iterator operator++(int) {
            if (ind_ == map_->capacity_)
                return *this;
            size_t old = ind_;
            ++ind_;
            while (ind_ < map_->capacity_ && map_->status[ind_] != 1)
                ++ind_;
            return iterator(map_, old);
        }

        bool operator==(const iterator &oth) const {
            return ind_ == oth.ind_ && map_ == oth.map_;
        }

        bool operator!=(const iterator &oth) const {
            return ind_ != oth.ind_ || map_ != oth.map_;
        }

        std::pair<const KeyType, ValueType> &operator*() {
            return (std::pair<const KeyType, ValueType> &) (map_->val_[ind_]);
        }

        std::pair<const KeyType, ValueType> *operator->() {
            return (std::pair<const KeyType, ValueType> *) (&map_->val_[ind_]);
        }

    };

    class const_iterator {
    private:
        const HashMap *map_{nullptr};
        size_t ind_ = 0;
    public:

        const_iterator() = default;

        const_iterator(const HashMap *map, const size_t &ind) : map_(map), ind_(ind) {}

        const_iterator &operator++() {
            if (ind_ == map_->capacity_)
                return *this;
            ++ind_;
            while (ind_ < map_->capacity_ && map_->status[ind_] != 1)
                ++ind_;
            return *this;
        }

        const_iterator operator++(int) {
            if (ind_ == map_->capacity_)
                return *this;
            size_t old = ind_;
            ++ind_;
            while (ind_ < map_->capacity_ && map_->status[ind_] != 1)
                ++ind_;
            return const_iterator(map_, old);
        }

        bool operator==(const const_iterator &oth) const {
            return ind_ == oth.ind_ && map_ == oth.map_;
        }

        bool operator!=(const const_iterator &oth) const {
            return ind_ != oth.ind_ || map_ != oth.map_;
        }

        const std::pair<const KeyType, ValueType> &operator*() const {
            return (std::pair<const KeyType, ValueType> &) (map_->val_[ind_]);
        }

        const std::pair<const KeyType, ValueType> *operator->() const {
            return (std::pair<const KeyType, ValueType> *) (&map_->val_[ind_]);
        }

    };

    explicit HashMap(Hash hash = Hash()) : hash_(hash) {
        capacity_ = 2;
        val_.resize(capacity_);
        PSL.assign(capacity_, 0);
        status.assign(capacity_, 0);
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>> &list, Hash hash = Hash()) : hash_(hash) {
        capacity_ = 2;
        val_.resize(capacity_);
        PSL.assign(capacity_, 0);
        status.assign(capacity_, 0);
        for (std::pair<KeyType, ValueType> i: list) {
            insert(i);
        }
    }

    template<typename Iterator>
    explicit HashMap(Iterator begin, Iterator end, Hash hash = Hash()): hash_(hash) {
        capacity_ = 2;
        val_.resize(capacity_);
        PSL.assign(capacity_, 0);
        status.assign(capacity_, 0);
        for (Iterator it = begin; it != end; ++it) {
            insert(*it);
        }
    }

    void insert(const std::pair<KeyType, ValueType> &el) {
        size_t pos = hash_(el.first) % capacity_;
        size_t current_PSL = 1;
        std::pair<KeyType, ValueType> cur = el;
        while (status[pos] && !(status[pos] > 0 && val_[pos].first == cur.first)) {
            if (status[pos] != 2 && PSL[pos] < current_PSL) {
                std::swap(PSL[pos], current_PSL);
                swap(val_[pos], cur);
            }
            ++pos;
            ++current_PSL; // if I remove this it will be linear probing lmao
            if (pos == capacity_) {
                pos = 0;
            }
        }
        if (status[pos] == 1)
            return;
        PSL[pos] = current_PSL;
        val_[pos] = cur;
        status[pos] = 1;
        ++size_;
        if (size_ * 2 >= capacity_) {
            _rebuild();
        }
    }

    void erase(const KeyType &key) {
        size_t pos = hash_(key) % capacity_;
        while (val_[pos].first != key && status[pos]) {
            ++pos;
            if (pos == capacity_) {
                pos = 0;
            }
        }
        if (status[pos] == 0 || status[pos] == 2)
            return;
        status[pos] = 2;
        --size_;
        if (size_ * 4 < capacity_) {
            _rebuild();
        }
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    ~HashMap() = default;

    Hash hash_function() const {
        return hash_;
    }

    const ValueType &at(const KeyType &key) const {
        size_t pos = hash_(key) % capacity_;
        while (status[pos] && val_[pos].first != key) {
            ++pos;
            if (pos == capacity_) {
                pos = 0;
            }
        }
        if (status[pos] != 1)
            throw std::out_of_range("No such key in HashMap");
        return val_[pos].second;
    }

    ValueType &operator[](const KeyType &key) {
        if (find(key) == end()) {
            insert(std::make_pair(key, ValueType()));
        }
        size_t pos = hash_(key) % capacity_;
        while (status[pos] != 1 || val_[pos].first != key) {
            ++pos;
            if (pos == capacity_) {
                pos = 0;
            }
        }
        return val_[pos].second;
    }

    void clear() {
        size_ = 0;
        capacity_ = 2;
        val_.resize(capacity_);
        PSL.assign(capacity_, 0);
        status.assign(capacity_, 0);
    }

    iterator begin() {
        size_t pos = 0;
        while (pos < capacity_ && status[pos] != 1) {
            ++pos;
        }
        return iterator(this, pos);
    }

    iterator end() {
        return iterator(this, capacity_);
    }

    const_iterator begin() const {
        size_t pos = 0;
        while (pos < capacity_ && status[pos] != 1) {
            ++pos;
        }
        return const_iterator(this, pos);
    }

    const_iterator end() const {
        return const_iterator(this, capacity_);
    }

    const_iterator find(const KeyType &key) const {
        size_t pos = hash_(key) % capacity_;
        while (status[pos] && !(val_[pos].first == key)) {
            ++pos;
            if (pos == capacity_) {
                pos = 0;
            }
        }
        if (status[pos] != 1) {
            return end();
        }
        return const_iterator(this, pos);
    }

    iterator find(KeyType key) {
        size_t pos = hash_(key) % capacity_;
        while (status[pos] != 0 && !(val_[pos].first == key)) {
            ++pos;
            if (pos == capacity_) {
                pos = 0;
            }
        }
        if (status[pos] != 1) {
            return end();
        }
        return iterator(this, pos);
    }

};


#endif //MY_HASH_MAP_HASH_MAP_H

