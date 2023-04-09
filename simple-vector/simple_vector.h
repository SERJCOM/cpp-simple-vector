#pragma once
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include "array_ptr.h"
#include <cassert>
#include <iostream>
#include <iterator>
 

class ReserveProxyObj{
public:
    ReserveProxyObj(size_t capicacity){
        this->capicacity = capicacity;
    }

    size_t GetCapicacity(){
        return capicacity;
    }

private:
    size_t capicacity = 0;
}; 


ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
 
template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;
 
    SimpleVector() noexcept = default;
 
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size):data_(size) {
        size_ = size;
        capacity_ = size;
        for(size_t i = 0; i < size; i++){
            data_[i] = Type{};
        }
    }
 
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value):data_(size) {
        size_ = size;
        capacity_ = size;
        for(size_t i = 0; i < size; i++){
            data_[i] = value;
        }
    }

    SimpleVector(size_t size, Type&& value):data_(size) {
        size_ = size;
        capacity_ = size;
        for(size_t i = 0; i < size; i++){
            data_[i] = std::move(value);
        }
    }

 
    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init):data_(init.size()) {
        SimpleVector<Type> temp(init.size());
        temp.CreateSimpleVector(init.begin(), init.end());
 
        swap(temp);
    }
 
    SimpleVector(const SimpleVector& temp):data_(temp.size_){
        SimpleVector temp_vec(temp.GetSize());
        temp_vec.CreateSimpleVector(temp.begin(), temp.end());
 
        swap(temp_vec);
    }
 
    SimpleVector(ReserveProxyObj res): SimpleVector(){
        
        Reserve(res.GetCapicacity());
    }

    SimpleVector(SimpleVector&& vec){
        delete[] data_.Get();
        data_.swap(vec.data_);
        ArrayPtr<Type> temp;
        vec.data_.swap(temp);
        size_ = std::exchange(vec.size_, 0);
        capacity_ = std::exchange(vec.capacity_, static_cast<size_t>(0));
    }
 
 
    void swap(SimpleVector& other) noexcept { 
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
 
    SimpleVector& operator=(const SimpleVector& rhs){

        if(this != &rhs){
            SimpleVector<Type> temp(rhs);
            swap(temp);
        }
        
        return *this;
    }
 
    void PushBack(const Type& value) { 
        if(size_ == 0 && capacity_ == 0 ){
            ArrayPtr<Type> temp(1);
            data_.swap(temp);
            data_[0] = value;
            size_++;
            capacity_ = 1;
        }
        else if(size_ == capacity_){
            ArrayPtr<Type> temp(size_ * 2);
            std::copy(data_.Get(), data_.Get() + size_, temp.Get());
            temp[size_] = value;
            data_.swap(temp);
            capacity_ = size_ * 2;
            size_++;
        }
 
        else{
            data_[size_] = value;
            size_++;
        }
    }

    void PushBack(Type&& value) { 
        if(size_ == 0 && capacity_ == 0 ){
            ArrayPtr<Type> temp(1);
            data_.swap(temp);
            data_[0] = std::move(value);
            size_++;
            capacity_ = 1;
        }
        else if(size_ == capacity_){
            ArrayPtr<Type> temp(size_ * 2);
            copy_m(begin(), begin() + size_,temp.Get());

            temp[size_] = std::move(value);
            data_.swap(temp);
            capacity_ = size_ * 2;
            size_++;
        }
 
        else{
            data_[size_] = std::move(value);
            size_++;
        }
    }


 
    void PopBack() noexcept{
        if(size_ != 0){
            Resize(size_ - 1);
        }
    }
 
 
    Iterator Insert(ConstIterator pos, const Type& value) { 
        size_t index = pos - begin();

        if(size_ == capacity_){
            if(capacity_ == 0){
                SimpleVector<Type> temp(index == 0 ? 1 : index, value);
                data_.swap(temp.data_);
                data_[index] = value;
                capacity_ = index == 0 ? 1 : index;
                size_ = 1;
            }
            else{
                ArrayPtr<Type> temp(size_ * 2);
                std::copy(data_.Get(), data_.Get() + index, temp.Get());
                temp[index] = value;
                std::copy(data_.Get() + index, data_.Get() + size_, temp.Get() + index + 1);
                data_.swap(temp);     
                capacity_ = size_ * 2;
                size_++;  
            }
        }
        else{
            std::copy_backward(data_.Get() + index, data_.Get() + size_, data_.Get() + size_ + 1);
            data_[index] = value;
            size_++;
        }
 
        return begin() + index;
    }

    Iterator Insert(ConstIterator pos, Type&& value) { 
        size_t index = pos - begin();

        if(size_ == capacity_){
            if(capacity_ == 0){
                SimpleVector<Type> temp(index == 0 ? 1 : index, std::move(value));
                data_.swap(temp.data_);
                data_[index] = std::move(value);
                capacity_ = index == 0 ? 1 : index;
                size_ = 1;
            }
            else{
                ArrayPtr<Type> temp(size_ * 2);
                copy_m(begin(), begin() + size_, temp.Get());
                temp[index] = std::move(value);
                copy_m(begin() + index, begin() + size_, temp.Get() + index - 1);
                
                data_.swap(temp);     
                capacity_ = size_ * 2;
                size_++;  
            }
        }
        else{
            for(auto it = end(); it != begin() + index; it--){
                *it = std::move(*(it - 1)); 
            }
            data_[index] = std::move(value);
            size_++;
        }
 
        return begin() + index;
    }

    Iterator Erase(ConstIterator pos) { 
        size_t index = pos - begin();
        copy_m(begin() + index, end() - 1, begin() + index, 1);
        size_--;
        return data_.Get() + index;
    }
 

    void Reserve(size_t new_capacity){
        if(new_capacity > capacity_){
            ArrayPtr<Type> temp(new_capacity);
            copy_m(begin(), end(), temp.Get());
            data_.swap(temp);
            capacity_ = new_capacity;
            return;
        }
    }
 
    
 
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }
 
    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        // Напишите тело самостоятельно
        return capacity_;
    }
 
    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }
 
    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return data_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if(index >= size_){
            throw std::out_of_range(": (");
        }
        return data_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if(index >= size_){
            throw std::out_of_range(": (");
        }
        return data_[index];
    }
 
    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }
 
    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size < size_){
            size_ = new_size;
        }
        else if(new_size > size_ && new_size < capacity_){

            for(auto it = begin() + size_; it != begin() + new_size; it++){
                *it = std::move(Type{});
            }

            size_ = new_size;
        }
        else{
            ArrayPtr<Type> temp(new_size);
            
            size_t index = 0;
            for(auto it = begin() + size_; it != begin() + new_size; it++){
                temp[index] = std::move(Type{});
                index++;
            }
            
            copy_m(begin(), begin() + size_, temp.Get());

            data_.swap(temp);
            size_ = new_size;
            capacity_ = new_size;
        }
    }
 
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return data_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
    return (data_.Get() + size_);
    }
 
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return data_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
    return (data_.Get() + size_);
    }
 
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return data_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return (data_.Get() + size_);
    }
 
    template<typename T>
    void CreateSimpleVector(T begin, T end){
        int i = 0;
        for(auto temp = begin; temp != end; temp++){
            data_[i] = *(begin + i);
            i++;
        }
    }

 
private:

    template<typename T1, typename T2>
    void copy_m(T1 first, T1 last, T2 first_, int range = 0){
        size_t index = 0;
        for(auto it = first; it != last; it++){
            first_[index] = std::move(*(it + range));
            index++;
        }
    }
 
    
    
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> data_;
};


 
template <typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    
    return (lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}
 
template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
 
    return !(lhs == rhs);
}
 
template <typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    //return rhs < lhs;
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
 
template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs < rhs) || (lhs == rhs);
}
 
template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    //return rhs < lhs;
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}
 
template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs > rhs) || (lhs == rhs);
} 
 




