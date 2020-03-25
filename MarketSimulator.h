//
// Created by sebastiend on 06/10/2018.
//

#ifndef IEOR_HOMEWORK5_MARKETSIMULATOR_H
#define IEOR_HOMEWORK5_MARKETSIMULATOR_H
#include "AppBase.h"
#include "Order.h"
#include <set>
#include <queue>
#include <string>
#include <iostream>

class MarketSimulator : public AppBase {
public:
    unsigned execution_id;
    std::set<std::string> list_symbols;
    price_t pnl;
    MarketSimulator(
             std::queue<Order> &strategy_to_ordermanager_,
             std::queue<ExecutionOrder> &ordermanager_to_strategy_,
             std::queue<Order> &ordermanager_to_simulator_,
             std::queue<ExecutionOrder> &simulator_to_ordermanager_,
             std::queue<BookUpdate> &bookbuilder_to_strategy_
    );
    bool handle_order();

    virtual void start() {is_working=true;}
    virtual void stop() {is_working=false;}
    price_t get_pnl();

};


#endif //IEOR_HOMEWORK5_MARKETSIMULATOR_H
