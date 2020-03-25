//
// Created by sebastiend on 06/10/2018.
//

#ifndef IEOR_HOMEWORK5_TRADINGSTRATEGY_H
#define IEOR_HOMEWORK5_TRADINGSTRATEGY_H
#include "AppBase.h"
#include "BookUpdate.h"
#include "Order.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

struct MarketPrice{
    long epoch_time;
    price_t  price;
    bool is_buy;
    MarketPrice(long t=0, price_t p=0, bool i=true): epoch_time(t),price(p),is_buy(i) {}
};

class Signal{
private:
    int size;
    std::vector<BookUpdate> ordbook;
    std::queue<MarketPrice> ma5_record;
    std::queue<MarketPrice> ma20_record;
    float ma5, ma20, prev_ma5, prev_ma20;
    float curr_bid, curr_ask;
public:
    Signal(int _size=0):size(_size){ma5=0; ma20=0; prev_ma5=0; prev_ma20=0; curr_bid=0; curr_ask=0;}
    void insert_book_update(BookUpdate bu){
        ordbook.push_back(BookUpdate(bu));
        size++;
        if(size%2){
            curr_bid=bu.get_price();
            ma5_record.push(MarketPrice(bu.get_epochtime(),bu.get_price(),bu.get_is_buy()));
            ma20_record.push(MarketPrice(bu.get_epochtime(),bu.get_price(),bu.get_is_buy()));
            return; //for odd number, bid/ask are not fully paired (wait for another ask to get even number price)
        }

        //update the ma5min & ma20min
        long curr_time=bu.get_epochtime();
        curr_ask=bu.get_price();

        //update for before (change)
        prev_ma5=ma5;
        prev_ma20=ma20;

        //save in total (not average)
        ma5=ma5*(ma5_record.size()-1); //previous ma5 ma 20 not include current new bid
        ma20=ma20*(ma20_record.size()-1);

        ma5+=curr_ask+curr_bid;
        ma20+=curr_ask+curr_bid;

        ma5_record.push(MarketPrice(bu.get_epochtime(),bu.get_price(),bu.get_is_buy()));
        while(ma5_record.front().epoch_time < curr_time-300 && (!ma5_record.empty())){
            ma5-=ma5_record.front().price;
            ma5_record.pop();//remove the data long before
        }

        ma20_record.push(MarketPrice(bu.get_epochtime(),bu.get_price(),bu.get_is_buy()));
        while(ma20_record.front().epoch_time < curr_time-1200 && (!ma20_record.empty())){
            ma20-=ma20_record.front().price;
            ma20_record.pop();//remove the data long before
        }

        //re-calculate the average
        ma5=ma5/ma5_record.size(); //save in two decimal
        ma20=ma20/ma20_record.size(); //save in two decimal
        std::cout<<ma5<<" "<<ma20<<std::endl;

        return;
    };
    float get_5min_moving_average(){
        if(ordbook.size()%2) return 0; //bid/ask not fully paired, the size is in odd number, so not calculating

        return int(ma5*100)/100.0;
    }
    float get_20min_moving_average(){
        if(ordbook.size()%2) return 0; //bid/ask not fully paired, the size is in odd number, so not calculating
        return int(ma20*100)/100.0;
    }

    bool go_long(){
        float short_ = get_5min_moving_average();
        float long_ = get_20min_moving_average();
        bool now_higher = short_>0 && long_>0 && short_ > long_;
        bool prev_lower = prev_ma5>0 && prev_ma20>0 && prev_ma5 < prev_ma20;

        if(prev_lower && now_higher) return true; //this moment start to rise over, we go long now
        return false;
    }
    bool go_short(){
        float short_ = get_5min_moving_average();
        float long_ = get_20min_moving_average();
        bool now_lower = short_>0 && long_>0 && short_ < long_;
        bool prev_higher = prev_ma5>0 && prev_ma20>0 && prev_ma5 > prev_ma20;

        if(prev_higher && now_lower) return true; //this moment start to rise over, we go long now
        return false;
    }
    bool is_tradeable(BookUpdate &bu){return true;}
    int get_size() {return size;}
};

class Execution{
private:
    Order e;
    bool tradeable;
public:

    Execution():tradeable(false){}
    bool insert_order(long timestamp_,
                      bool is_buy_,
                      float price_,
                      unsigned int quantity_,
                      const char * venue_,
                      const char * symbol_,
                      ordertype type_,
                      unsigned int id_);
    bool is_tradeable() {return tradeable;}
    void set_tradeable(bool is_tradable){tradeable=is_tradable;};
    Order & get_order() {return e;}
};

class TradingStrategy : public AppBase {
private:
    Signal signal;
    Execution execution;
    int order_id;
    std::unordered_map<std::string,float > positions;
    int total_quantity; //total quantity position (might <0 - short)
    unsigned int number_of_rejections;
    unsigned int number_of_fills;
    price_t pnl;
    price_t  curr_bid, curr_ask;
    long prev_time;

public:
    TradingStrategy(
            std::queue<Order> &strategy_to_ordermanager_,
            std::queue<ExecutionOrder> &ordermanager_to_strategy_,
            std::queue<Order> &ordermanager_to_simulator_,
            std::queue<ExecutionOrder> &simulator_to_ordermanager_,
            std::queue<BookUpdate> &bookbuilder_to_strategy_
    ):
    AppBase(strategy_to_ordermanager_,
              ordermanager_to_strategy_,
              ordermanager_to_simulator_,
              simulator_to_ordermanager_,
              bookbuilder_to_strategy_),
              signal(),
              execution(),
              order_id(1),
              number_of_rejections(0),
              number_of_fills(0),
              pnl(0) {curr_bid=0; curr_ask=0; total_quantity=0; prev_time=0;}

    virtual void start() {is_working=true;}
    virtual void stop() {
        positions.clear();
        is_working=false;
    }


    bool process_book_update(BookUpdate &bu);
    bool process_book_update();
    bool process_execution();
    bool process_market_response();
    bool clear_positions();
    Execution get_execution();
    float get_position(std::string symbol);
    unsigned int get_number_of_rejections();
    unsigned int get_number_of_fills();
    void reset_position();
    float get_pnl();

};


#endif //IEOR_HOMEWORK5_TRADINGSTRATEGY_H
