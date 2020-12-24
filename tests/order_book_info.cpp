#include <gtest/gtest.h>
#include <array>
#include "order_book.h"
#include "test_book.h"

TEST(ORDER_BOOK, OrderBookInfo)
{
    OrderBook order_book = test_order_book();
    auto orderbook_info_json = order_book.orderbook_info_json();
    auto res = R"V({
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
    ASSERT_STREQ(orderbook_info_json.c_str(), res);
}

TEST(ORDER_BOOK, OrderBookInfoLimit)
{
    OrderBook order_book = test_order_book();
    auto orderbook_info_json = order_book.orderbook_info_json(2,1);
    auto res = R"V({
    "asks": [
        {
            "price": 1001,
            "quantity": 30
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
        }
    ]
}
)V";
    ASSERT_STREQ(orderbook_info_json.c_str(), res);
}

TEST(ORDER_BOOK, OrderAddBidInfo)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1000, 300);
    auto orderbook_info_json = order_book.orderbook_info_json();
    auto res = R"V({
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
    ASSERT_STREQ(orderbook_info_json.c_str(), res);
}

TEST(ORDER_BOOK, OrderAddAskInfo)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Ask, 1000, 300);
    auto orderbook_info_json = order_book.orderbook_info_json();
    auto res = R"V({
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
    ASSERT_STREQ(orderbook_info_json.c_str(), res);
}

TEST(ORDER_BOOK, OrderExecutionBidBigPriceInfo)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1002, 45);
    auto result = R"V({
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
    auto orderbook_info_json = order_book.orderbook_info_json();
    ASSERT_STREQ(orderbook_info_json.c_str(), result);
}
TEST(ORDER_BOOK, OrderExecutionBidSmallPriceInfo)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1001, 45);
    auto result = R"V({
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
    auto orderbook_info_json = order_book.orderbook_info_json();
    ASSERT_STREQ(orderbook_info_json.c_str(), result);
}

TEST(ORDER_BOOK, OrderExecutionAskBigPriceInfo)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Ask, 900, 80);
    auto result = R"V({
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
    auto orderbook_info_json = order_book.orderbook_info_json();
    ASSERT_STREQ(orderbook_info_json.c_str(), result);
}

TEST(ORDER_BOOK, OrderExecutionAskSmallPriceInfo)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Ask, 980, 80);
    auto result = R"V({
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
    auto orderbook_info_json = order_book.orderbook_info_json();
    ASSERT_STREQ(orderbook_info_json.c_str(), result);
}


TEST(ORDER_BOOK, OrderExecutionBidBigPriceBigQuantityInfo)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1010, 300);
    auto result = R"V({
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
    auto orderbook_info_json = order_book.orderbook_info_json();
    ASSERT_STREQ(orderbook_info_json.c_str(), result);
}

TEST(ORDER_BOOK, OrderExecutionAskBigPriceBigQuantityInfo)
{
    OrderBook order_book = test_order_book();
    order_book.add_order(Order::Type::Bid, 1010, 300);
    auto result = R"V({
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
    auto orderbook_info_json = order_book.orderbook_info_json();
    ASSERT_STREQ(orderbook_info_json.c_str(), result);
}
