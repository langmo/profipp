#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#pragma once

#include <map>
#include <vector>
namespace profinet::tools
{

template<typename K, typename V> class MapView
{
public:
    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = V;
        using pointer           = V*;  // or also value_type*
        using reference         = V&;  // or also value_type&
        iterator(typename std::map<K, V>::iterator mapIterator_) : mapIterator{mapIterator_} 
        { 
        }
        iterator(const iterator& iterator) : mapIterator(iterator.mapIterator) 
        { 
        }
        iterator& operator=(const iterator& other) 
        {
            this->mapIterator = other.mapIterator;
            return *this;
        }
        bool operator==(const iterator& other) const 
        {
            return this->mapIterator == other.mapIterator;
        }
        bool operator!=(const iterator& other) const 
        {
            return this->mapIterator != other.mapIterator;
        }

        V& operator*() const 
        {
            return (mapIterator->second);
        }

        V* operator->() const 
        {
            return &(mapIterator->second);
        }

        iterator& operator++() 
        {
            ++mapIterator;
            return *this;
        }

        iterator operator++(int) 
        {
            iterator tmp{*this};
            ++(*this);
            return tmp;
        }

    private:
        typename std::map<K, V>::iterator mapIterator;
    };
    class const_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = V;
        using pointer           = V*;  // or also value_type*
        using reference         = V&;  // or also value_type&
        const_iterator(typename std::map<K, V>::const_iterator mapIterator_) : mapIterator{mapIterator_} 
        { 
        }
        bool operator==(const const_iterator& other) const 
        {
            return this->mapIterator == other.mapIterator;
        }
        bool operator!=(const const_iterator& other) const 
        {
            return this->mapIterator != other.mapIterator;
        }

        const V& operator*() 
        {
            return (mapIterator->second);
        }

        const V* operator->() 
        {
            return &(mapIterator->second);
        }

        const_iterator& operator++() 
        {
            ++mapIterator;
            return *this;
        }

        const_iterator operator++(int) 
        {
            const_iterator tmp{*this};
            ++(*this);
            return tmp;
        }

    private:
        typename std::map<K, V>::const_iterator mapIterator;
    };

    MapView() 
    {
    } 
    iterator begin() 
    { 
        return iterator{map.begin()}; 
    }
    iterator end() 
    { 
        return iterator{map.end()}; 
    }
    const_iterator begin() const
    { 
        return const_iterator{map.cbegin()}; 
    }
    const_iterator end() const
    { 
        return const_iterator{map.cend()}; 
    }

    V* operator[](const K& key )
    {
        auto it{map.find(key)};
        if(it != map.end())
            return &it->second;
        else
            return nullptr;
    }
    const V* operator[](const K& key ) const
    {
        auto it{map.find(key)};
        if(it != map.end())
            return &it->second;
        else
            return nullptr;
    }
protected:
    std::map<K, V>  map{};
};

template<typename V> class VectorView
{
public:
    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = V;
        using pointer           = V*;  // or also value_type*
        using reference         = V&;  // or also value_type&
        iterator(typename std::vector<V>::iterator listIterator_) : listIterator{listIterator_} 
        { 
        }
        iterator(const iterator& iterator) : listIterator(iterator.listIterator) 
        { 
        }
        iterator& operator=(const iterator& other) 
        {
            this->listIterator = other.listIterator;
            return *this;
        }
        bool operator==(const iterator& other) const 
        {
            return this->listIterator == other.listIterator;
        }
        bool operator!=(const iterator& other) const 
        {
            return this->listIterator != other.listIterator;
        }

        V& operator*() const 
        {
            return *listIterator;
        }

        V* operator->() const 
        {
            return &(listIterator);
        }

        iterator& operator++() 
        {
            ++listIterator;
            return *this;
        }

        iterator operator++(int) 
        {
            iterator tmp{*this};
            ++(*this);
            return tmp;
        }

    private:
        typename std::vector<V>::iterator listIterator;
    };
    class const_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = V;
        using pointer           = V*;  // or also value_type*
        using reference         = V&;  // or also value_type&
        const_iterator(typename std::vector<V>::const_iterator listIterator_) : listIterator{listIterator_} 
        { 
        }
        bool operator==(const const_iterator& other) const 
        {
            return this->listIterator == other.listIterator;
        }
        bool operator!=(const const_iterator& other) const 
        {
            return this->listIterator != other.listIterator;
        }

        const V& operator*() 
        {
            return *listIterator;
        }

        const V* operator->() 
        {
            return &(listIterator);
        }

        const_iterator& operator++() 
        {
            ++listIterator;
            return *this;
        }

        const_iterator operator++(int) 
        {
            const_iterator tmp{*this};
            ++(*this);
            return tmp;
        }

    private:
        typename std::vector<V>::const_iterator listIterator;
    };
    
    VectorView() 
    {
    } 

    iterator begin() 
    { 
        return list.begin(); 
    }
    iterator end() 
    { 
        return list.end(); 
    }
    const_iterator begin() const
    { 
        return list.cbegin(); 
    }
    const_iterator end() const
    { 
        return list.cend(); 
    }

    V* operator[](std::size_t pos)
    {
        if(pos < 0 || pos >= list.size())
            return nullptr;
        return list[pos];
    }
    const V* operator[](std::size_t pos) const
    {
        if(pos < 0 || pos >= list.size())
            return nullptr;
        return list[pos];
    }
protected:
    std::vector<V>  list{};
};

}
#endif