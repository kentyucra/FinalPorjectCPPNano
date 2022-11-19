#include <curses.h>
#include "order.h"
#include <memory>
#include <thread>
#include <string>
#include <algorithm>
#include <math.h>

namespace NCursesDisplay
{
    void DisplayLiveOrders(std::shared_ptr<Orderbook> orderbook, WINDOW *window)
    {
        int row{1};
        int const side_column{2};
        int const price_column{9};
        int const quantity_column{21};
        int y_max{getmaxy(window)};

        mvwprintw(window, row, side_column, "Side");
        mvwprintw(window, row, price_column, "Price $");
        mvwprintw(window, row, quantity_column, "Quantity");
        for (auto it : orderbook->Orders())
        {
            row++;
            if (it._side == Side::ask)
                wattron(window, COLOR_PAIR(2));
            else
                wattron(window, COLOR_PAIR(3));

            mvwprintw(window, row, side_column, it._side == Side::ask ? "Ask" : "Bid");
            if (it._side == Side::ask)
                wattroff(window, COLOR_PAIR(2));
            else
                wattroff(window, COLOR_PAIR(3));
            mvwprintw(window, row, price_column, "%.2f", it._price);
            mvwprintw(window, row, quantity_column, "%.2f", it._quantity);

            if (row >= y_max - 2)
                break;
        }
    }

    std::string barStr(int space, double percen)
    {
        std::string result = "";
        int bars = floor((1.0 * space * percen) / 100.0);
        for (int i = 0; i <= space; i++)
            result += i <= bars ? '|' : ' ';

        return result;
    }

    void DisplayOrderbook(std::shared_ptr<Orderbook> orderbook, WINDOW *window)
    {

        int x_max{getmaxx(window)};
        int y_max{getmaxy(window)};

        int row{1};
        int const price_column{2};
        int const bar_column{12};
        int const quantity_column{x_max - 2 - 13};

        int space_for_bar = quantity_column - bar_column - 3;

        mvwprintw(window, row, price_column, "Price");
        mvwprintw(window, row, bar_column, "Quantity Bar");
        mvwprintw(window, row, quantity_column, "Quantity");

        std::map<double, double> asksByTick = orderbook->AsksByTick();
        float max_quantity_ask = 0.0;

        for (auto it : asksByTick)
            max_quantity_ask = std::max(max_quantity_ask, (float)it.second);

        int index = 0;
        for (auto rit = asksByTick.rbegin(); rit != asksByTick.rend(); rit++)
        {
            row++;
            mvwprintw(window, row, price_column, "%.2f", rit->first);
            wattron(window, COLOR_PAIR(4));
            mvwprintw(window, row, bar_column, barStr(space_for_bar, rit->second * 100.0 / max_quantity_ask).c_str());
            wattroff(window, COLOR_PAIR(4));
            mvwprintw(window, row, quantity_column, "%.2f", rit->second);
            index++;
            if (index > y_max / 2 - 3)
                break;
        }

        row++;
        mvwprintw(window, row, price_column, "          ");
        mvwprintw(window, row, bar_column, "-------------spread-------------");

        std::map<double, double> bidsByTick = orderbook->BidsByTick();
        float max_quantity_bid = 0.0;

        for (auto it : bidsByTick)
            max_quantity_bid = std::max(max_quantity_bid, (float)it.second);

        index = 0;
        for (auto rit = bidsByTick.rbegin(); rit != bidsByTick.rend(); rit++)
        {
            row++;
            mvwprintw(window, row, price_column, "%.2f", rit->first);
            wattron(window, COLOR_PAIR(5));
            mvwprintw(window, row, bar_column, barStr(space_for_bar, rit->second * 100.0 / max_quantity_bid).c_str());
            wattroff(window, COLOR_PAIR(5));
            mvwprintw(window, row, quantity_column, "%.2f", rit->second);
            index++;
            if (index > y_max / 2 - 3)
                break;
        }
    }
    void Display(std::shared_ptr<Orderbook> orderbook, int n = 10)
    {
        initscr();     // start ncurses
        noecho();      // do not print input values
        cbreak();      // terminate ncurses on ctrl + c
        start_color(); // enable color

        int x_max{getmaxx(stdscr)};
        int y_max(getmaxy(stdscr));
        int x_orderbook = (float)x_max * 70.0 / 100.0;
        WINDOW *orderbook_window = newwin(y_max - 1, x_orderbook, 1, 0);
        WINDOW *live_orders_window = newwin(y_max - 1, x_max - x_orderbook - 2, 1, x_orderbook + 1);
        // WINDOW *process_window =
        //     newwin(3 + n, x_max - 1, system_window->_maxy + 1, 0);

        init_pair(1, COLOR_BLUE, COLOR_BLACK);
        init_pair(2, COLOR_WHITE, COLOR_RED);
        init_pair(3, COLOR_WHITE, COLOR_GREEN);
        init_pair(4, COLOR_RED, COLOR_BLACK);
        init_pair(5, COLOR_GREEN, COLOR_BLACK);

        while (1)
        {
            mvwprintw(stdscr, 0, 0, "Orderbook");
            mvwprintw(stdscr, 0, x_orderbook + 1, "Live orders");
            box(orderbook_window, 0, 0);
            box(live_orders_window, 0, 0);
            DisplayOrderbook(orderbook, orderbook_window);
            DisplayLiveOrders(orderbook, live_orders_window);
            wrefresh(orderbook_window);
            wrefresh(live_orders_window);
            refresh();
            std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
        }

        endwin();
    }
};