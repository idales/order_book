#include "test_book.h"

OrderBook test_order_book(OrderBook::OrderCallback executed_order_callback /*= nullptr*/
    , OrderBook::OrderCallback canceled_order_callback /*= nullptr*/)
{
    OrderBook order_book(executed_order_callback, canceled_order_callback);
    std::array<Data, 10> orders =
        {
            Data{Order::Type::Ask, 1003, 50},
            Data{Order::Type::Ask, 1003, 40},
            Data{Order::Type::Ask, 1002, 30},
            Data{Order::Type::Ask, 1001, 20},
            Data{Order::Type::Ask, 1001, 10},
            Data{Order::Type::Bid, 999, 15},
            Data{Order::Type::Bid, 999, 25},
            Data{Order::Type::Bid, 900, 35},
            Data{Order::Type::Bid, 900, 44},
            Data{Order::Type::Bid, 800, 55},
        };
    for (const auto &order : orders)
    {
        order_book.add_order(order.type, order.price, order.quantity);
    }
    return order_book;
}
