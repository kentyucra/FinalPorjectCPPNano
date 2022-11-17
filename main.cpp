#include "socket.h"
#include "messagequeue.h"
#include "order.h"

// STL
#include <vector>
#include <thread>
#include <type_traits>
#include <map>
#include <mutex>
#include <memory>

// Boost libraries
#include <boost/json/parse.hpp>
#include <boost/json/src.hpp>
#include <boost/describe.hpp>

// curses
#include <curses.h>

MessageQueue<Order> orderQueue;
struct info
{
    std::string feed;
    std::string product_id;
    std::vector<std::vector<double>> bids;
    std::vector<std::vector<double>> asks;
};
BOOST_DESCRIBE_STRUCT(info, (), (feed, product_id, bids, asks));

struct snapshot : info
{
    int numLevels;
};
BOOST_DESCRIBE_STRUCT(snapshot, (), (numLevels, feed, product_id, bids, asks));

void consumeMessage(std::string &&msg)
{
    // std::cout << msg << std::endl;
    auto jv = boost::json::parse(msg);
    auto object = jv.as_object();
    if (object.find("event") != object.end())
        return;

    // std::cout << msg << std::endl;
    auto newjv = boost::json::parse(msg);
    info nf;
    if (object.find("numLevels") != object.end())
        nf = boost::json::value_to<snapshot>(newjv);
    else
        nf = boost::json::value_to<info>(newjv);
    for (auto order : nf.asks)
        orderQueue.send(Order(order[0], order[1], Side::ask));

    for (auto order : nf.bids)
        orderQueue.send(Order(order[0], order[1], Side::bid));

    // auto nf = boost::json::value_to<info>(jv);
}

void readMessageFromWebSocket()
{
    Socket cryptoSocket("www.cryptofacilities.com", "/ws/v1");
    cryptoSocket.Start();
    auto const subscribeXBT = R"({"event":"subscribe","feed":"book_ui_1","product_ids":["PI_XBTUSD"]})";
    cryptoSocket.Write(subscribeXBT);

    while (1)
    {
        cryptoSocket.Read();
        std::string message = std::move(cryptoSocket.Message());
        consumeMessage(std::move(message));
        cryptoSocket.ClearBuffer();
    }

    auto const unsubscribeXBT = R"({"event":"unsubscribe","feed":"book_ui_1","product_ids":["PI_XBTUSD"]})";
    cryptoSocket.Write(unsubscribeXBT);
    cryptoSocket.Close();
}

void processFromMessageQueue(std::shared_ptr<Orderbook> orderbook)
{
    int steps = 100;
    while (steps--)
    {
        Order order = orderQueue.receive();
        orderbook->insertOrder(std::move(order));
        std::cout << "step " << steps << "= " << order;
        // std::this_thread::sleep_for(1ms);
    }
    // std::cout << *orderbook;
}

// Sends a WebSocket message and prints the response
int main(int argc, char **argv)
{
    std::shared_ptr<Orderbook> orderbook = std::make_shared<Orderbook>();
    std::thread readFromWS(readMessageFromWebSocket);
    std::thread processMessages(processFromMessageQueue, orderbook);
    // int steps = 1000;
    // while (steps--)
    // {
    //     Order order = orderQueue.receive();
    //     // std::cout << order;
    //     orderbook.insertOrder(std::move(order));
    // }
    // std::cout << orderbook;
    readFromWS.join();
    processMessages.join();

    initscr(); // start ncurses

    // readMessageFromWebSocket();
    return 0;
}