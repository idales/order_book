#include "order_book.h"

#include <iomanip>
#include <sstream>

uint64_t Order::next_id = 0;

Order::IdType OrderBook::add_order(Order::Type type, Order::PriceType price, Order::QuantityType quantity)
{
    Order order(type, price, quantity);
    Order::IdType id = order.id();
    auto fully_executed = try_execute(order);
    if (not fully_executed) // place order to book
    {
        std::pair<OrderContainerIterator, bool> res;
        if (type == Order::Type::Bid)
            res = _bid_queue.emplace(order);
        else
            res = _ask_queue.emplace(order);
        assert(res.second);
        auto &it = res.first;
        _id_order_link.emplace(std::make_pair(it->id(), it));
    }
    assert(check_consistency());
    return id;
}

template <typename Container>
bool OrderBook::try_execute(Order &order, Container &container, CompareOrderFunction possible_execution)
{
    Order container_order = Order::make_zero_order();

    for (auto it = container.begin(); it != container.end() && order.quantity() > 0 && possible_execution(it->price(), order.price());)
    {
        // remove first order from book
        container_order = *it;
        _id_order_link.erase(it->id());
        it = container.erase(it);
        // determine execution parameters
        auto execution_quantity = std::min(container_order.quantity(), order.quantity());
        auto execution_price = container_order.price();
        // execution
        auto executed_order = container_order.split(execution_quantity, execution_price);
        send_executed_order(executed_order); //may be full order or part
        auto executed_incoming_order = order.split(execution_quantity, execution_price);
        send_executed_order(executed_incoming_order); //may be full order or part
        // update market data
        if (_transactions_started && _last_price == execution_price)
            _last_quantity += execution_quantity;
        else
            _last_quantity = execution_quantity;
        _last_price = execution_price;
        _transactions_started = true;
    }

    if (container_order.quantity() != 0) // return rest part of order to OrderBook
    {
        auto res = container.emplace(container_order);
        auto &it = res.first;
        assert(res.second);
        _id_order_link.emplace(std::make_pair(it->id(), it));
    }
    return order.quantity() == 0;
}

bool OrderBook::try_execute(Order &order) // return true if incoming order fully executed
{
    if (order.type() == Order::Type::Bid)
    {
        return try_execute(order, _ask_queue, [](Order::PriceType price_order_in_queue, Order::PriceType price_order_come) {
            return price_order_in_queue <= price_order_come;
        });
    }
    else
    {
        return try_execute(order, _bid_queue, [](Order::PriceType price_order_in_queue, Order::PriceType price_order_come) {
            return price_order_in_queue >= price_order_come;
        });
    }
}

void OrderBook::cancel_order(Order::IdType id)
{
    auto order_link = find_order(id);
    if (_canceled_order_callback)
        _canceled_order_callback(*(order_link->second));
    if (order_link->second->type() == Order::Type::Ask)
        _ask_queue.erase(order_link->second);
    else
        _bid_queue.erase(order_link->second);
    _id_order_link.erase(order_link);
    assert(check_consistency());
}

Order OrderBook::get_order(Order::IdType id) const
{
    auto order_link = find_order(id);
    return *(order_link->second);
}

std::pair<bool, OrderBook::PricePosition> OrderBook::PriceAggregator::next_price()
{
    PricePosition price_position;
    if (_cur_pos == _end_pos) // end of container
        return std::make_pair(false, price_position);
    price_position.price = _cur_pos->price();
    price_position.quantity = _cur_pos->quantity();
    while ((++_cur_pos != _end_pos) && (_cur_pos->price() == price_position.price))
    {
        price_position.quantity += _cur_pos->quantity();
    }
    return std::make_pair(true, price_position);
}

OrderBook::PriceAggregator OrderBook::make_price_aggregator(Order::Type type) const
{
    if (type == Order::Type::Ask)
        return OrderBook::PriceAggregator(_ask_queue.cbegin(), _ask_queue.cend());
    else
        return OrderBook::PriceAggregator(_bid_queue.cbegin(), _bid_queue.cend());
}
void out_orders_json(std::ostream &out_str, int order_limit, OrderBook::PriceAggregator &aggregator)
{
    bool next_iteration = false;

    while (order_limit < 0 || order_limit-- != 0)
    {
        auto price_position_pair = aggregator.next_price();
        if (!price_position_pair.first)
            break; // no more prices in the queue
        if (next_iteration)
        {
            out_str << "," << std::endl;
        }
        out_str << std::setw(8) << " "
                << "{" << std::endl
                << std::setw(12) << " "
                << "\"price\": " << price_position_pair.second.price << "," << std::endl
                << std::setw(12) << " "
                << "\"quantity\": " << price_position_pair.second.quantity << std::endl
                << std::setw(8) << " "
                << "}";
        next_iteration = true;
    }
}

void OrderBook::orderbook_info_json_internal(std::ostream &out_str,int bid_order_limit, int ask_order_limit) const
{
    out_str << R"V(    "asks": [
)V";
    {
        auto aggregator = make_price_aggregator(Order::Type::Ask);
        out_orders_json(out_str, ask_order_limit, aggregator);
    }
    out_str << std::endl
            << std::setw(4) << " "
            << "]," << std::endl
            << std::setw(4) << " "
            << "\"bids\": [" << std::endl;
    {
        auto aggregator = make_price_aggregator(Order::Type::Bid);
        out_orders_json(out_str, bid_order_limit, aggregator);
    }
    out_str << std::endl
            << std::setw(4) << " "
            << "]" << std::endl;
}

std::string OrderBook::orderbook_info_json(int bid_order_limit /*= -1*/, int ask_order_limit /*= -1*/) const
{
    std::ostringstream out_str;
    out_str << "{" << std::endl;
    orderbook_info_json_internal(out_str, bid_order_limit, ask_order_limit);
    out_str << "}" << std::endl;
    return out_str.str();
}


void output_best_ask_json(std::ostream &out_str,
                     const std::pair<bool, OrderBook::PricePosition> &ask_price_position_pair)
{
    if (ask_price_position_pair.first)
    {
        const auto &price_position = ask_price_position_pair.second;
        out_str << R"V(
    "best_ask": {
        "price": )V"
            << price_position.price << R"V(,
        "quantity": )V"
            << price_position.quantity << R"V(
    })V";
    }
}

void output_best_bid_json(std::ostream &out_str,
                     const std::pair<bool, OrderBook::PricePosition> &bid_price_position_pair)
{
    if (bid_price_position_pair.first)
    {
        const auto &price_position = bid_price_position_pair.second;
        out_str << R"V(
    "best_bid": {
        "price": )V"
            << price_position.price << R"V(,
        "quantity": )V"
            << price_position.quantity << R"V(
    })V";
    }
}

void OrderBook::market_data_1_json_internal(std::ostream &out_str, bool &next_comma) const
{
    auto ask_price_position_pair = make_price_aggregator(Order::Type::Ask).next_price();
    output_best_ask_json(out_str, ask_price_position_pair);
    next_comma = ask_price_position_pair.first;
    auto bid_price_position_pair = make_price_aggregator(Order::Type::Bid).next_price();
    if (next_comma && bid_price_position_pair.first)
        out_str << ",";
    output_best_bid_json(out_str, bid_price_position_pair);
    if (_transactions_started)
    {
        if (next_comma) out_str << ",";
        out_str << R"V(
    "last_transaction": {
        "price": )V" << _last_price << R"V(,
        "quantity": )V" << _last_quantity << R"V(
    })V";
    }
}

std::string OrderBook::market_data_1_json() const
{
    std::ostringstream out_str;
    out_str << "{";
    bool next_comma = false;
    market_data_1_json_internal(out_str, next_comma);
    out_str << std::endl << "}" << std::endl;
    return out_str.str();
}

std::string OrderBook::market_data_2_json(int bid_order_limit /*= -1*/, int ask_order_limit /*= -1*/) const
{
    std::ostringstream out_str;
    out_str << "{";
    bool next_comma = false;
    market_data_1_json_internal(out_str, next_comma);
    out_str << "," << std::endl;
    orderbook_info_json_internal(out_str, bid_order_limit, ask_order_limit);
    out_str << "}" << std::endl;
    return out_str.str();
}
