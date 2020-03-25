//
// Created by sebastiend on 06/10/2018.
//

#include "MarketSimulator.h"
#include "fstream"

MarketSimulator::MarketSimulator(
        std::queue<Order> &strategy_to_ordermanager_,
        std::queue<ExecutionOrder> &ordermanager_to_strategy_,
        std::queue<Order> &ordermanager_to_simulator_,
        std::queue<ExecutionOrder> &simulator_to_ordermanager_,
        std::queue<BookUpdate> &bookbuilder_to_strategy_
        ):AppBase(strategy_to_ordermanager_,
                ordermanager_to_strategy_,
                ordermanager_to_simulator_,
                 simulator_to_ordermanager_,
                 bookbuilder_to_strategy_),
                 execution_id(0),
                 pnl(0)
     {
        list_symbols = {"EURUSD","EUR/USD"};
     }


bool MarketSimulator::handle_order(){
    if(!is_working)
        return false;
    if(ordermanager_to_simulator.empty())
        return true;
    const Order &o = ordermanager_to_simulator.front();
    ordermanager_to_simulator.pop();

    const bool is_tradeable = list_symbols.find(o.getSymbol()) != list_symbols.end();

    ExecutionOrder new_execution(o);
    if(is_tradeable){
        new_execution.setState(o.getQuantity()>=1000?orderstate::ACKNOWLEDGED:orderstate::REJECTED);
        new_execution.setExecutionID(execution_id++);
        simulator_to_ordermanager.push(new_execution);
        new_execution.setState(orderstate::FILLED);
        if(o.getTimeStamp()<440000){
            std::cout << "simulator push a fill|" <<
                      new_execution.getTimeStamp() << "|" <<
                      new_execution.getPrice() << "|"  <<
                      new_execution.isBuy()  << "|" <<
                      new_execution.getQuantity() << "|" <<
                      std::endl;

            std::fstream fs;
            //use .open() to write in append mode without overwritting before
            fs.open("C:\\Users\\lenovo\\Desktop\\CLionProjects\\Project7\\output.txt", std::fstream::app);
            fs<< "simulator push a fill|" <<
              new_execution.getTimeStamp() << "|" <<
              new_execution.getPrice() << "|"  <<
              new_execution.isBuy()  << "|" <<
              new_execution.getQuantity() << "|" <<
              std::endl;
            fs.close();
        }

        float new_position=new_execution.getQuantity()*new_execution.getPrice();
        pnl += new_execution.isBuy()? new_position : (-1)*new_position; //buy + , sell -
        simulator_to_ordermanager.push(new_execution);
    }
    else{
        // std::cout << "simulator push a reject" << std::endl;
        new_execution.setState(orderstate::REJECTED);
        new_execution.setExecutionID(execution_id++);
        simulator_to_ordermanager.push(new_execution);
    }
    return true;
}


price_t MarketSimulator::get_pnl() {
    return std::round(pnl);
};