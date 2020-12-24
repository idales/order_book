#pragma once

#include "order_book.h"

struct Data
{
    Order::Type type;
    Order::PriceType price;
    Order::QuantityType quantity;
};

OrderBook test_order_book(OrderBook::OrderCallback executed_order_callback = nullptr
, OrderBook::OrderCallback canceled_order_callback = nullptr);
