//
// Created by sebastiend on 06/10/2018.
//

#ifndef IEOR_HOMEWORK5_BOOKUPDATE_H
#define IEOR_HOMEWORK5_BOOKUPDATE_H

#include <string.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono>

typedef float price_t;
typedef unsigned int quantity_t;
typedef unsigned int level_t;



class BookUpdate {
private:
    level_t level;
    price_t price;
    quantity_t quantity;
    char venue[20];
    char symbol[20];
    bool is_buy;
    unsigned long epoch_time;
public:
    BookUpdate(level_t level_,
            price_t price_,
            quantity_t quantity_,
            const char *venue_,
            bool is_buy_,
            const char *symbol_,
               unsigned long epoch_time_):
            level(level_),
            price(price_),
            quantity(quantity_),
            is_buy(is_buy_),
            epoch_time(epoch_time_)
    {
        strcpy(venue,venue_);
        strcpy(symbol,symbol_);
    }
    price_t get_price() const {return price;}
    quantity_t get_quantity() const {return quantity;}
    level_t get_level() const {return level;}
    const char * get_venue() const {return venue;}
    bool get_is_buy() const {return is_buy;}
    const char * get_symbol() const {return symbol;}
    long get_epochtime() {return epoch_time;}
    friend bool operator < (const BookUpdate& a, const BookUpdate& b){ //overload bookupdate comparision for sort
        return a.epoch_time < b.epoch_time;
    }
    void set_price (price_t p) {price=p;}
    void set_quantity (quantity_t q) {quantity=q;}
    void set_is_buy (bool is_b) {is_buy=is_b;}
    void set_epoch_time (long t) {epoch_time=t;}
};


#endif //IEOR_HOMEWORK5_BOOKUPDATE_H
