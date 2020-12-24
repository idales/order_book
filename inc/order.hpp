#pragma once

#include <cassert>
#include <cinttypes>

class Order
{
public:
    enum class Type
    {
        Ask,
        Bid
    };
    using IdType = uint64_t;
    using PriceType = int32_t;
    using QuantityType = uint32_t;

    Order(Type type, PriceType price, QuantityType quantity)
        : _type(type), _price(price), _quantity(quantity), _id(++next_id) {}

    // return order with the same id, price, type but split original _quantity
    // by new quantity and rest which saved in current order
    Order split(QuantityType quantity, PriceType execution_price)
    {
        assert(quantity <= _quantity);
        assert( _type == Type::Ask && execution_price >= _price || _type == Type::Bid && execution_price <= _price);
        Order new_order = *this;
        new_order._quantity = quantity;
        new_order._price = execution_price;
        _quantity -= quantity;
        return new_order;
    }
    Type type() const { return _type; }
    PriceType price() const { return _price; }
    QuantityType quantity() const { return _quantity; }
    IdType id() const { return _id; }
    static Order make_zero_order() // create explicitly zero order to save some order from split
    {
        return Order();
    }
private:
    Order(): _price(0), _quantity(0), _id(0)  {}
    static IdType next_id; // id generator for new order
    IdType _id;
    PriceType _price;
    QuantityType _quantity;
    Type _type;
};