#pragma once
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "order.hpp"

class OrderBook
{
public:
    /// @brief callback type for executed and canceled orders
    using OrderCallback = std::function<void(Order)>;

    struct Exception : public std::runtime_error
    {
        Exception(const std::string &message) throw()
            : std::runtime_error(message) {}
    };
    struct NotFoundException : public Exception
    {
        NotFoundException(const std::string &message) throw()
            : Exception(message) {}
    };
    /// @brief Creates OrderBook.
    ///
    /// @param executed_order_callback std::function which accepts executed orders. May be nullptr.
    /// @param canceled_order_callback std::function which accepts canceled orders. May be nullptr.
    OrderBook(OrderCallback executed_order_callback = nullptr, OrderCallback canceled_order_callback = nullptr)
        : _executed_order_callback(executed_order_callback), _canceled_order_callback(canceled_order_callback) {}

    /// @brief Add order to OrderBook.
    ///
    /// @param type the Order type. Can be either Order::Type::Bid, or Order::Type::Ask
    /// @param price Order price
    /// @param quantity Order quantity
    Order::IdType add_order(Order::Type type, Order::PriceType price, Order::QuantityType quantity);

    /// @brief Cancel order. Can throw NotFoundException
    ///
    /// @param id order id
    void cancel_order(Order::IdType id);

    /// @brief Get order copy. Can be used to print order info. Can throw NotFoundException
    ///
    /// @param id order id
    Order get_order(Order::IdType id) const;

    /// @brief orderbook information in JSON format
    ///
    /// @param bid_order_limit max number bid positions, -1 means output all bid positions
    /// @param ask_order_limit max number ask positions, -1 means output all ask positions
    std::string orderbook_info_json(int bid_order_limit = -1, int ask_order_limit = -1) const;

    /// @brief market data 1 in JSON format
    std::string market_data_1_json() const;

    /// @brief market data 2 in JSON format
    std::string market_data_2_json(int bid_order_limit = -1, int ask_order_limit = -1) const;

private:
    struct AskOrderSort
    {
        bool operator()(const Order &o1, const Order &o2)
        {
            return o1.price() < o2.price() || (o1.price() == o2.price() && o1.id() < o2.id());
        }
    };
    struct BidOrderSort
    {
        bool operator()(const Order &o1, const Order &o2)
        {
            return o1.price() > o2.price() || (o1.price() == o2.price() && o1.id() < o2.id());
        }
    };

    using OrderContainerAsk = std::set<Order, AskOrderSort>;
    using OrderContainerBid = std::set<Order, BidOrderSort>;
    using OrderContainerIterator = std::set<Order>::iterator;
    using IdOrderLink = std::map<Order::IdType, OrderContainerIterator>;

    OrderContainerAsk _ask_queue;
    OrderContainerBid _bid_queue;
    IdOrderLink _id_order_link;
    OrderCallback _executed_order_callback = nullptr;
    OrderCallback _canceled_order_callback = nullptr;

    bool _transactions_started = false;
    Order::PriceType _last_price = 0;
    Order::QuantityType _last_quantity = 0;

    IdOrderLink::const_iterator find_order(Order::IdType id) const
    {
        auto order_link = _id_order_link.find(id);
        if (order_link == _id_order_link.end())
        {
            throw OrderBook::NotFoundException(std::string("Order id ") + std::to_string(id) + "not found");
        }
        return order_link;
    }
    bool try_execute(Order &order);
    using CompareOrderFunction = std::function<bool (Order::PriceType, Order::PriceType)>;
    template<typename Container>
    bool try_execute(Order &order, Container &container, CompareOrderFunction possible_execution);
    void send_executed_order(Order order)
    {
        if (_executed_order_callback) _executed_order_callback(order);
    }
    struct PricePosition
    {
        Order::PriceType price = 0;
        Order::QuantityType quantity = 0;
    };

    class PriceAggregator
    {
    public:
        PriceAggregator(OrderContainerIterator start_pos, OrderContainerIterator end_pos)
            : _cur_pos(start_pos), _end_pos(end_pos){};
        std::pair<bool, PricePosition> next_price();
    private:
        OrderContainerIterator _cur_pos;
        const OrderContainerIterator _end_pos;
    };

    PriceAggregator make_price_aggregator(Order::Type type) const;

    friend void out_orders_json(std::ostream &out_str, int order_limit, OrderBook::PriceAggregator &aggregator);
    friend void output_best_ask_json(std::ostream &out_str,
                     const std::pair<bool, OrderBook::PricePosition> &ask_price_position_pair);
    friend void output_best_bid_json(std::ostream &out_str,
                    const std::pair<bool, OrderBook::PricePosition> &bid_price_position_pair);
    void orderbook_info_json_internal(std::ostream &out_str,int bid_order_limit, int ask_order_limit) const;
    void market_data_1_json_internal(std::ostream &out_str, bool &next_comma) const;
    bool check_consistency() const
    {
        return _ask_queue.size() +_bid_queue.size() == _id_order_link.size();
    }
};
