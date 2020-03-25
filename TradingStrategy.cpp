//
// Created by sebastiend on 06/10/2018.
//

#include "TradingStrategy.h"


bool Execution::insert_order(long timestamp_,
                  bool is_buy_,
                  float price_,
                  unsigned int quantity_,
                  const char * venue_,
                  const char * symbol_,
                  ordertype type_,
                  unsigned int id_)
{
    e.setTimestamp(timestamp_);
    e.setSide(is_buy_);
    e.setPrice(price_);
    e.setQuantity(quantity_);
    e.setVenue(venue_);
    e.setType(type_);
    e.setSymbol(symbol_);
    e.setOrderID(id_);
};

bool TradingStrategy::process_book_update(BookUpdate &bu){ //for process specific order
    if(!is_working)
        return false;

    if (signal.is_tradeable(bu)) {
        execution.insert_order(bu.get_epochtime(),bu.get_is_buy(),
                               bu.get_price(),bu.get_quantity(),bu.get_venue(),bu.get_symbol(),ordertype::LIMIT,order_id++);
        execution.set_tradeable(true);
    }
    return process_execution();
}


bool TradingStrategy::process_book_update(){ //for process book_builder_to_strategy
    bool order_now=true;
    if(!is_working)
        return false;
    if(bookbuilder_to_strategy.empty())
        return true;

    BookUpdate &bu = bookbuilder_to_strategy.front();
    bookbuilder_to_strategy.pop();

    signal.insert_book_update(bu);
    if(signal.get_size()%2){
        curr_bid = bu.get_price();
        return  false; //not execute since no even order number (not paired bid and ask)
    }
    curr_ask = bu.get_price();

    if(signal.go_long() || signal.go_short()){
        bu.set_price((curr_ask+curr_bid)/2); //set mid-price as market price
        bu.set_is_buy(signal.go_long()); //long for buy, short for sell
        return process_book_update(bu);
    }
    return false;
}

bool TradingStrategy::process_execution() {
    if(!is_working)
        return false;
    Order order;
    if (execution.is_tradeable()) {
        order.setTimestamp(execution.get_order().getTimeStamp());
        order.setType(execution.get_order().getOrderType());
        order.setVenue(execution.get_order().getVenue());
        order.setQuantity(execution.get_order().getQuantity());
        order.setPrice(execution.get_order().getPrice());
        order.setOrderID(execution.get_order().getID());
        order.setSymbol(execution.get_order().getSymbol());
        order.setSide(execution.get_order().isBuy());
        execution.set_tradeable(false);
        //std::cout << "strategy push an order" << std::endl;
        strategy_to_ordermanager.push(order);
    }
    return true;
};


bool TradingStrategy::process_market_response()
{
    if(ordermanager_to_strategy.empty())
        return true;
    ExecutionOrder &order = ordermanager_to_strategy.front();

    //update the position of the filled order

    if(order.getState()==orderstate::FILLED){
        number_of_fills++;
        price_t  add_position=order.getPrice()*order.getQuantity();
        pnl += order.isBuy()? add_position : (-1)*add_position; //buy +, short -
        if(positions.find(order.getSymbol())==positions.end()) positions[order.getSymbol()]=0;
        positions[order.getSymbol()]+=order.isBuy()?order.getPrice()*order.getQuantity():(-1)*order.getPrice()*order.getQuantity(); //buy +, short -
        total_quantity += order.isBuy()?order.getQuantity():(-1)*order.getQuantity();
    }
    else if(order.getState()==orderstate::REJECTED) number_of_rejections++;

    ordermanager_to_strategy.pop();

    return true;
}

bool TradingStrategy::clear_positions(){
    std::unordered_map<std::string,float>::iterator it;
    float left_position=0;
    for(it=positions.begin();it!=positions.end();it++){
        if(it->second!=0) left_position+=it->second;
    }
    if(left_position==0) return false; //not need clear positions

    BookUpdate &bu = bookbuilder_to_strategy.front();
    bookbuilder_to_strategy.pop();

    signal.insert_book_update(bu);
    if(signal.get_size()%2){
        curr_bid = bu.get_price();
        return  false; //not execute since no even order number (not paired bid and ask)
    }
    curr_ask = bu.get_price();
    bu.set_price((curr_ask+curr_bid)/2); //set mid-price as market price
    bu.set_is_buy(left_position <= 0); //left >0 for sell, left <0 for buy

    bu.set_quantity(abs(total_quantity)); //using current mid-price to clear all positions

    return process_book_update(bu);
}

Execution TradingStrategy::get_execution(){
    return execution;
}

float TradingStrategy::get_position(std::string symbol)
{
    if(positions.find(symbol)==positions.end()) return 0; //not found
    return positions[symbol];
}

unsigned int TradingStrategy::get_number_of_rejections() {
    return number_of_rejections;
}

unsigned int TradingStrategy::get_number_of_fills() {
    return number_of_fills;
}

float TradingStrategy::get_pnl() {
    return std::round(pnl);
}

void TradingStrategy::reset_position(){
    positions.clear();
}