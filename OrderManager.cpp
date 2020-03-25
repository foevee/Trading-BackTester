//
// Created by sebastiend on 06/10/2018.
//

#include "OrderManager.h"


bool OrderManager::handle_execution_order(){
    if(!is_working)
        return false;
    if (simulator_to_ordermanager.empty())
        return true;

    const ExecutionOrder &e = simulator_to_ordermanager.front();
    simulator_to_ordermanager.pop();

    list_orders[e.getID()].setState(e.getState());
    if(e.getState()==orderstate::FILLED){
        //Note: There are two positions, one in TradingStrategy class, the other in OrderManager class here, we need to operate for twice
        if(positions.find(e.getSymbol())==positions.end()) positions[e.getSymbol()]=0;
        positions[e.getSymbol()]+=e.isBuy()?e.getPrice()*e.getQuantity():(-1)*e.getPrice()*e.getQuantity();
        list_orders.erase(e.getID()); //erase the filled order
    }


    ordermanager_to_strategy.push(e); //send back to the strategy
    return true;
};


unsigned int OrderManager::get_number_of_open_orders()
{
    int count=0;
    std::unordered_map<unsigned int, ExecutionOrder>::iterator it;
    for(it=list_orders.begin();it!=list_orders.end();it++){
        orderstate st=(*it).second.getState();
        if(st==orderstate::OPEN || st==orderstate::ACKNOWLEDGED) count++;
    }
    return count;
}


unsigned int OrderManager::get_number_of_non_acknowledged_orders()
{
    int count=0;
    std::unordered_map<unsigned int, ExecutionOrder>::iterator it;
    for(it=list_orders.begin();it!=list_orders.end();it++){
        orderstate st=(*it).second.getState();
        if(st==orderstate::OPEN) count++;
    }
    return count;
}

float OrderManager::get_position(std::string symbol)
{
    //only can use iterator to search a unordered_map
    std::unordered_map<std::string,float>::const_iterator got = positions.find(symbol);
    if(got==positions.end()) return 0;
    return positions[symbol];
}

bool OrderManager::handle_order(){
    if(!is_working)
        return false;
    if (strategy_to_ordermanager.empty())
        return true;
    const Order &e = strategy_to_ordermanager.front();
    strategy_to_ordermanager.pop();

    order_id++;
    order=e;
    //std::cout << "TRY ORDER..." << list_orders.size() << std::endl;

    std::unordered_map<std::string,float >::const_iterator got = positions.find (order.getSymbol());
    float curr_position=(got!=positions.end())?got->second:0; //the existing position from the positions map
    float add_position=e.isBuy()?e.getQuantity()*e.getPrice():-1.0*(e.getQuantity()*e.getPrice());


    if(std::abs(curr_position+add_position)>100000000 || e.getQuantity()<10 || list_orders.size()>10){
        execution_order=e;
        execution_order.setOrderID(order_id); //not setExecutedID, since not being executed
        execution_order.setState(orderstate::REJECTED); //here reject will not be sent to simulator, so will not be count in open/acknowledged order
        //return the rejected order to the strategy
        ordermanager_to_strategy.push(execution_order);
        return false;
    }

    //successful order, return to simulator first, then in "handle_execution_order", we will return to the strategy
    order.setOrderID(order_id);
    list_orders[order_id]=order; //use order to assign a execution_order in the list_orders
    list_orders[order_id].setState(orderstate::OPEN);
    ordermanager_to_simulator.push(order);
    return true;
};
