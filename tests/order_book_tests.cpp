#include <gtest/gtest.h>
#include <array>

#include "order_book.h"
#include "test_book.h"

TEST(ORDER_BOOK, EmtpyTest)
{
    OrderBook order_book;
    SUCCEED();
}

TEST(ORDER_BOOK, OrderAdd)
{
    const Order::PriceType price = 1000;
    const Order::QuantityType quantity = 100;
    const std::array<Order::Type, 2> all_order_types = {Order::Type::Bid, Order::Type::Ask};
    for (auto order_type : all_order_types)
    {
        OrderBook order_book;
        auto id = order_book.add_order(order_type, 1000, 100);
        auto order = order_book.get_order(id);
        ASSERT_EQ(price, order.price());
        ASSERT_EQ(quantity, order.quantity());
        ASSERT_EQ(order_type, order.type());
    }
}

TEST(ORDER_BOOK, OrderCancel)
{
    const Order::PriceType price = 1000;
    const Order::QuantityType quantity = 100;
    const std::array<Order::Type, 2> all_order_types = {Order::Type::Bid, Order::Type::Ask};
    for (auto order_type : all_order_types)
    {
        std::vector<Order> canceled_orders;
        OrderBook order_book(nullptr, [&canceled_orders](Order order) { canceled_orders.push_back(order); });
        auto id = order_book.add_order(order_type, 1000, 100);
        order_book.cancel_order(id);
        ASSERT_EQ(canceled_orders.size(), 1);
        auto &order = canceled_orders[0];
        ASSERT_EQ(price, order.price());
        ASSERT_EQ(quantity, order.quantity());
        ASSERT_EQ(order_type, order.type());
        ASSERT_THROW(
            {
                auto order = order_book.get_order(id);
            },
            OrderBook::NotFoundException);
    }
}

TEST(ORDER_BOOK, OrderBookMarketData2)
{
    OrderBook order_book = test_order_book();
    auto market_data_2_json = order_book.market_data_2_json();
    auto res = R"V({
    "best_ask": {
        "price": 1001,
        "quantity": 30
    },
    "best_bid": {
        "price": 999,
        "quantity": 40
    },
    "asks": [
        {
            "price": 1001,
            "quantity": 30
        },
        {
            "price": 1002,
            "quantity": 30
        },
        {
            "price": 1003,
            "quantity": 90
        }
    ],
    "bids": [
        {
            "price": 999,
            "quantity": 40
        },
        {
            "price": 900,
            "quantity": 79
        },
        {
            "price": 800,
            "quantity": 55
        }
    ]
}
)V";
    ASSERT_STREQ(market_data_2_json.c_str(), res);
}

TEST(ORDER_BOOK, OrderAddBid)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1000, 300);
    auto market_data_2_json = order_book.market_data_2_json();
    auto res = R"V({
    "best_ask": {
        "price": 1001,
        "quantity": 30
    },
    "best_bid": {
        "price": 1000,
        "quantity": 300
    },
    "asks": [
        {
            "price": 1001,
            "quantity": 30
        },
        {
            "price": 1002,
            "quantity": 30
        },
        {
            "price": 1003,
            "quantity": 90
        }
    ],
    "bids": [
        {
            "price": 1000,
            "quantity": 300
        },
        {
            "price": 999,
            "quantity": 40
        },
        {
            "price": 900,
            "quantity": 79
        },
        {
            "price": 800,
            "quantity": 55
        }
    ]
}
)V";
    ASSERT_STREQ(market_data_2_json.c_str(), res);
}

TEST(ORDER_BOOK, OrderAddAsk)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Ask, 1000, 300);
    auto market_data_2_json = order_book.market_data_2_json();
    auto res = R"V({
    "best_ask": {
        "price": 1000,
        "quantity": 300
    },
    "best_bid": {
        "price": 999,
        "quantity": 40
    },
    "asks": [
        {
            "price": 1000,
            "quantity": 300
        },
        {
            "price": 1001,
            "quantity": 30
        },
        {
            "price": 1002,
            "quantity": 30
        },
        {
            "price": 1003,
            "quantity": 90
        }
    ],
    "bids": [
        {
            "price": 999,
            "quantity": 40
        },
        {
            "price": 900,
            "quantity": 79
        },
        {
            "price": 800,
            "quantity": 55
        }
    ]
}
)V";
    ASSERT_STREQ(market_data_2_json.c_str(), res);
}

TEST(ORDER_BOOK, OrderExecutionBidBigPriceMarket1)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1002, 45);
    auto result = R"V({
    "best_ask": {
        "price": 1002,
        "quantity": 15
    },
    "best_bid": {
        "price": 999,
        "quantity": 40
    },
    "last_transaction": {
        "price": 1002,
        "quantity": 15
    }
}
)V";
    auto market_data_1_json = order_book.market_data_1_json();
    ASSERT_STREQ(market_data_1_json.c_str(), result);
}

TEST(ORDER_BOOK, OrderExecutionBidBigPriceExecutedTransactions)
{
    std::vector<Order> executed_orders;
    OrderBook order_book = test_order_book([&executed_orders](Order order){ executed_orders.push_back(order); });
    order_book.add_order(Order::Type::Bid, 1002, 45);
    constexpr int size = 6;
    ASSERT_EQ(executed_orders.size(), size);
    std::array<Data, size> results = {
        Data{Order::Type::Ask, 1001, 20},
        Data{Order::Type::Bid, 1001, 20}, // partial execution of incoming order
        Data{Order::Type::Ask, 1001, 10},
        Data{Order::Type::Bid, 1001, 10}, // partial execution of incoming order
        Data{Order::Type::Ask, 1002, 15},
        Data{Order::Type::Bid, 1002, 15} // final execution of incoming order
    };
    for (auto i = 0; i < size; i++)
    {
        const auto &order = executed_orders[i];
        const auto &result = results[i];
        ASSERT_EQ(order.type(), result.type);
        ASSERT_EQ(order.price(), result.price);
        ASSERT_EQ(order.quantity(), result.quantity);
    }
}

TEST(ORDER_BOOK, OrderExecutionBidBigPrice)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1002, 45);
    auto result = R"V({
    "best_ask": {
        "price": 1002,
        "quantity": 15
    },
    "best_bid": {
        "price": 999,
        "quantity": 40
    },
    "last_transaction": {
        "price": 1002,
        "quantity": 15
    },
    "asks": [
        {
            "price": 1002,
            "quantity": 15
        },
        {
            "price": 1003,
            "quantity": 90
        }
    ],
    "bids": [
        {
            "price": 999,
            "quantity": 40
        },
        {
            "price": 900,
            "quantity": 79
        },
        {
            "price": 800,
            "quantity": 55
        }
    ]
}
)V";
    auto market_data_2_json = order_book.market_data_2_json();
    ASSERT_STREQ(market_data_2_json.c_str(), result);
}



TEST(ORDER_BOOK, OrderExecutionBidSmallPrice)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1001, 45);
    auto result = R"V({
    "best_ask": {
        "price": 1002,
        "quantity": 30
    },
    "best_bid": {
        "price": 1001,
        "quantity": 15
    },
    "last_transaction": {
        "price": 1001,
        "quantity": 30
    },
    "asks": [
        {
            "price": 1002,
            "quantity": 30
        },
        {
            "price": 1003,
            "quantity": 90
        }
    ],
    "bids": [
        {
            "price": 1001,
            "quantity": 15
        },
        {
            "price": 999,
            "quantity": 40
        },
        {
            "price": 900,
            "quantity": 79
        },
        {
            "price": 800,
            "quantity": 55
        }
    ]
}
)V";
    auto market_data_2_json = order_book.market_data_2_json();
    ASSERT_STREQ(market_data_2_json.c_str(), result);
}

TEST(ORDER_BOOK, OrderExecutionAskBigPrice)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Ask, 900, 80);
    auto result = R"V({
    "best_ask": {
        "price": 1001,
        "quantity": 30
    },
    "best_bid": {
        "price": 900,
        "quantity": 39
    },
    "last_transaction": {
        "price": 900,
        "quantity": 40
    },
    "asks": [
        {
            "price": 1001,
            "quantity": 30
        },
        {
            "price": 1002,
            "quantity": 30
        },
        {
            "price": 1003,
            "quantity": 90
        }
    ],
    "bids": [
        {
            "price": 900,
            "quantity": 39
        },
        {
            "price": 800,
            "quantity": 55
        }
    ]
}
)V";
    auto market_data_2_json = order_book.market_data_2_json();
    ASSERT_STREQ(market_data_2_json.c_str(), result);
}

TEST(ORDER_BOOK, OrderExecutionAskSmallPrice)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Ask, 980, 80);
    auto result = R"V({
    "best_ask": {
        "price": 980,
        "quantity": 40
    },
    "best_bid": {
        "price": 900,
        "quantity": 79
    },
    "last_transaction": {
        "price": 999,
        "quantity": 40
    },
    "asks": [
        {
            "price": 980,
            "quantity": 40
        },
        {
            "price": 1001,
            "quantity": 30
        },
        {
            "price": 1002,
            "quantity": 30
        },
        {
            "price": 1003,
            "quantity": 90
        }
    ],
    "bids": [
        {
            "price": 900,
            "quantity": 79
        },
        {
            "price": 800,
            "quantity": 55
        }
    ]
}
)V";
    auto market_data_2_json = order_book.market_data_2_json();
    ASSERT_STREQ(market_data_2_json.c_str(), result);
}


TEST(ORDER_BOOK, OrderExecutionBidBigPriceBigQuantity)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1010, 300);
    auto result = R"V({
    "best_bid": {
        "price": 1010,
        "quantity": 150
    }
    "last_transaction": {
        "price": 1003,
        "quantity": 90
    },
    "asks": [

    ],
    "bids": [
        {
            "price": 1010,
            "quantity": 150
        },
        {
            "price": 999,
            "quantity": 40
        },
        {
            "price": 900,
            "quantity": 79
        },
        {
            "price": 800,
            "quantity": 55
        }
    ]
}
)V";
    auto market_data_2_json = order_book.market_data_2_json();
    ASSERT_STREQ(market_data_2_json.c_str(), result);
}
