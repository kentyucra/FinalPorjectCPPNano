
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <deque>
#include <string>

enum Side
{
    bid,
    ask
};

class Order
{
public:
    Order(float price, float quantity, Side side) : _price(price), _quantity(quantity), _side(side)
    {
        _time = std::chrono::system_clock::now().time_since_epoch() / std::chrono::nanoseconds(1);
    }

    friend std::ostream &operator<<(std::ostream &os, const Order &order)
    {
        os << "price = " << order._price << ",  quantity = " << order._quantity << ", side"
           << (order._side == Side::ask ? "ask" : "bid") << ", time = " << order._time << std::endl;
        return os;
    }

    float _price;
    float _quantity;
    Side _side;
    unsigned long _time;
};

class Orderbook
{

public:
    void insertOrder(Order &&order)
    {
        if (order._side == Side::ask)
        {
            // Delete this when not using asks
            // _asks.emplace_back(std::move(order));
            _asks.push_back(order);
            if (_asksByTick.find(order._price) == _asksByTick.end())
                _asksByTick[order._price] = std::vector<Order>();
            _asksByTick[order._price].push_back(order);
            _asksQuantityByTick[order._price] += order._quantity;
        }
        else
        {
            // Delete this when not using bids
            // _bids.emplace_back(std::move(order));
            _bids.push_back(order);
            if (_bidsByTick.find(order._price) == _bidsByTick.end())
                _bidsByTick[order._price] = std::vector<Order>();
            _bidsByTick[order._price].push_back(order);
            _bidsQuantityByTick[order._price] += order._quantity;
        }

        _orders.push_back(order);
        if (_orders.size() > _MAX_ORDERS)
            _orders.pop_front();
    }

    std::deque<Order> Orders()
    {
        return _orders;
    }

    friend std::ostream &operator<<(std::ostream &os, const Orderbook &orderbook)
    {
        os << "Size _asks = " << orderbook._asks.size() << std::endl;
        os << "Size _bids = " << orderbook._bids.size() << std::endl;
        // Print quantities
        for (auto it : orderbook._asksQuantityByTick)
            os << "ask[" << it.first << "] -> " << it.second << std::endl;

        for (auto it : orderbook._bidsQuantityByTick)
            os << "bid[" << it.first << "] -> " << it.second << std::endl;

        if (orderbook._bids.size() > 0 && orderbook._asks.size() > 0)
        {
            std::vector<Order>::const_iterator itMin, itMax;
            itMin = std::min_element(orderbook._asks.begin(), orderbook._asks.end(), cmp);
            itMax = std::max_element(orderbook._asks.begin(), orderbook._asks.end(), cmp);
            os << "Asks(min, max) = " << itMin->_price << ", " << itMax->_price << std::endl;
            itMin = std::min_element(orderbook._bids.begin(), orderbook._bids.end(), cmp);
            itMax = std::max_element(orderbook._bids.begin(), orderbook._bids.end(), cmp);
            os << "Bids(min, max) = " << itMin->_price << ", " << itMax->_price << std::endl;
        }
        return os;
    }

private:
    static bool cmp(const Order &order1, const Order &order2)
    {
        return order1._price < order2._price;
    }

    std::map<double, std::vector<Order>> _bidsByTick;
    std::map<double, std::vector<Order>> _asksByTick;

    std::map<double, double> _bidsQuantityByTick;
    std::map<double, double> _asksQuantityByTick;

    std::deque<Order> _orders;
    size_t _MAX_ORDERS{1000};

    std::vector<Order> _bids;
    std::vector<Order> _asks;
};