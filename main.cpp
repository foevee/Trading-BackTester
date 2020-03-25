#include <iostream>

#include "my_unit_test.h"
#include "Order.h"
#include "BookUpdate.h"
#include "MarketSimulator.h"
#include "OrderManager.h"
#include "TradingStrategy.h"
#include "MDReader.h"
#include "BookUpdate.h"
#include "BookBuilder.h"

void reset_communication(
        std::queue<Order> &strategy_to_ordermanager,
        std::queue<ExecutionOrder> &ordermanager_to_strategy,
        std::queue<Order> &ordermanager_to_simulator,
        std::queue<ExecutionOrder> &simulator_to_ordermanager
        )
{
    while (!strategy_to_ordermanager.empty()) strategy_to_ordermanager.pop();
    while (!ordermanager_to_strategy.empty()) ordermanager_to_strategy.pop();
    while (!ordermanager_to_simulator.empty()) ordermanager_to_simulator.pop();
    while (!simulator_to_ordermanager.empty()) simulator_to_ordermanager.pop();
}

int main() {
    /* Create an order o1 on the stack with the following parameter
     *
     * (100,true,1,10,1000,"JPM","APPL",ordertype::LIMIT);
     *
     * */

    std::queue<Order> strategy_to_ordermanager;
    std::queue<ExecutionOrder> ordermanager_to_strategy;
    std::queue<Order> ordermanager_to_simulator;
    std::queue<ExecutionOrder> simulator_to_ordermanager;
    std::queue<BookUpdate> bookbuilder_to_strategy;

    MDReader reader("C:\\Users\\lenovo\\Desktop\\CLionProjects\\Project7\\EUR_USD_Week1.csv",",",30); //MODIFY THIS LINE AND THE LINE 23 in the BookBuilder.h  //336500


    // Get the data from CSV File
    std::vector<BookUpdate> dataList = reader.getData();
    // Print the content of row by row on screen if you need
    //for(const BookUpdate &vec : dataList)
    //{
    //    std::cout << vec.get_venue() << std::endl;
    //    std::cout << vec.get_price() << std::endl;
    //}

    /* STEP1: You need to modify the function getData to convert a string into a epoch time */
    /* Example: https://www.epochconverter.com */


    /* Unit test will test to see if the first line of the csv file is well parsed */
    /* You will need to provide all the unit tests you need to perform this verification */
    /* ONLY FOR THE FIRST LINE */

    TEST_FUNCTION(dataList[0].get_price(),1.1595); //bid
    TEST_FUNCTION(dataList[0].get_epochtime(),0);
    TEST_STRING(dataList[0].get_venue(),"GAIN");
    TEST_FUNCTION(dataList[0].get_is_buy(),true);
    TEST_FUNCTION(dataList[1].get_price(),1.1598); //ask
    TEST_FUNCTION(dataList[1].get_epochtime(),0);
    TEST_STRING(dataList[1].get_venue(),"GAIN");
    TEST_FUNCTION(dataList[1].get_is_buy(),false);


    /* STEP2: You need to modify the class Signal to send a Signal to Long or Short based on
     * the 5-min and 20-min moving average.
     * You can create many more unit tests if necessary.
     * The goal here is to be sure that your moving average for 5 min and 20 min works
    */

    Signal sig1;
    sig1.insert_book_update(BookUpdate(0,10,10000,"GAIN",true,"EURUSD",0));
    sig1.insert_book_update(BookUpdate(0,12,10000,"GAIN",false,"EURUSD",0));
    sig1.insert_book_update(BookUpdate(0,14,10000,"GAIN",true,"EURUSD",280));
    sig1.insert_book_update(BookUpdate(0,15,10000,"GAIN",false,"EURUSD",280));
    sig1.insert_book_update(BookUpdate(0,14,10000,"GAIN",true,"EURUSD",290));
    sig1.insert_book_update(BookUpdate(0,15,10000,"GAIN",false,"EURUSD",290));
    TEST_FUNCTION(sig1.get_5min_moving_average(),float(13.33));
    sig1.insert_book_update(BookUpdate(0,14,10000,"GAIN",true,"EURUSD",300));
    sig1.insert_book_update(BookUpdate(0,16,10000,"GAIN",false,"EURUSD",300));
    TEST_FUNCTION(sig1.get_20min_moving_average(),13.75);

    /* STEP3: Generate Signal
     * if the short moving average becomes higher than the long moving average
     * you should go long so it means you will provide a signal to go long
     * if the short moving average becomes lower than the long moving average
     * you should go short so it means you will provide a signal to go short.
     * You will provide the unit tests to verify if the function
     * go_long() and go_short() works
    */

    TEST_FUNCTION(sig1.go_long(),false);
    TEST_FUNCTION(sig1.go_short(),false);

    sig1.insert_book_update(BookUpdate(0,10,10000,"GAIN",true,"EURUSD",500));
    sig1.insert_book_update(BookUpdate(0,10,10000,"GAIN",false,"EURUSD",500));
    TEST_FUNCTION(sig1.get_5min_moving_average(),13.5);
    TEST_FUNCTION(sig1.get_20min_moving_average(),13);
    sig1.insert_book_update(BookUpdate(0,8,10000,"GAIN",true,"EURUSD",550));
    sig1.insert_book_update(BookUpdate(0,8,10000,"GAIN",false,"EURUSD",550));


    sig1.insert_book_update(BookUpdate(0,7,10000,"GAIN",true,"EURUSD",600));
    sig1.insert_book_update(BookUpdate(0,7,10000,"GAIN",false,"EURUSD",600));
    TEST_FUNCTION(sig1.get_5min_moving_average(),10);
    TEST_FUNCTION(sig1.get_20min_moving_average(),11.42);// The first time 5_min_Moving_Average < 10_min_Moving_Average

    TEST_FUNCTION(sig1.go_long(),false);
    TEST_FUNCTION(sig1.go_short(),true); //now the short signal appears


    /* STEP4: Modify trading strategy to use the class signal correctly
     * It means that you will use go_long and go_short.
     * You will send an order buy or sell based on the return of these functions
     * You will provide the unit tests to test this part
    */

    TradingStrategy ts0(strategy_to_ordermanager,
                        ordermanager_to_strategy,
                        ordermanager_to_simulator,
                        simulator_to_ordermanager,
                        bookbuilder_to_strategy);
    ts0.start();

    if(sig1.go_long()){
        BookUpdate bu(0, 13.3, 10000, "GAIN", true, "EURUSD", 610);
        ts0.process_book_update(bu);
    }
    else if(sig1.go_short()){
        BookUpdate bu(0, 13.3, 10000, "GAIN", false, "EURUSD", 610);
        ts0.process_book_update(bu);
    }
    //check if the short order has been filled
    TEST_FUNCTION(ts0.get_execution().get_order().getPrice(),13.3);
    TEST_FUNCTION(ts0.get_execution().get_order().getQuantity(),10000);
    TEST_FUNCTION(ts0.get_execution().get_order().isBuy(),false); //prove its a short order



    /* STEP5: Run your Trading Strategy in the back tester
     * In this part the Market Simulator will display the fills that your trading strategy
     * need. It won't reject any orders and always fill them
    */

    while (!strategy_to_ordermanager.empty()){strategy_to_ordermanager.pop();} //clear record before

    TradingStrategy ts1(strategy_to_ordermanager,
                        ordermanager_to_strategy,
                        ordermanager_to_simulator,
                        simulator_to_ordermanager,
                        bookbuilder_to_strategy);

    MarketSimulator simulator(strategy_to_ordermanager,
                              ordermanager_to_strategy,
                              ordermanager_to_simulator,
                              simulator_to_ordermanager,
                              bookbuilder_to_strategy);

    OrderManager order_manager(strategy_to_ordermanager,
                               ordermanager_to_strategy,
                               ordermanager_to_simulator,
                               simulator_to_ordermanager,
                               bookbuilder_to_strategy);

    BookBuilder book_builder(strategy_to_ordermanager,
                             ordermanager_to_strategy,
                             ordermanager_to_simulator,
                             simulator_to_ordermanager,
                             bookbuilder_to_strategy);


    /* The book builder fills the queue of BookUpdate
    * for simulation purpose
    */
    book_builder.start(); //read all book data from csv
    simulator.start();
    order_manager.start();
    ts1.start();

    while(!bookbuilder_to_strategy.empty())
    {
        if(bookbuilder_to_strategy.size()==1){ //the last day, clear all positions
            if(ts1.clear_positions()){
                order_manager.handle_order();
                simulator.handle_order();
                order_manager.handle_execution_order();
                ts1.process_market_response();
                order_manager.handle_execution_order();
                ts1.process_market_response();
            }
        }
        //in process_book_update, we only process when there is MA signal
        else if(ts1.process_book_update()){
            order_manager.handle_order();
            simulator.handle_order();
            order_manager.handle_execution_order();
            ts1.process_market_response();
            order_manager.handle_execution_order();
            ts1.process_market_response();
        }
    }




    /* STEP7: You need to modify the function get_pnl in MarketSimulator and TradingStrategy
     * to calculate the PNL or Just return the value. At the end of the execution you should have the PNL
     * in both class be the same.
    */

    TEST_FUNCTION(ts1.get_pnl(),simulator.get_pnl());
    TEST_FUNCTION(std::round(ts1.get_pnl()),37630);//loss in the end


    PRINT_RESULTS();
    return 0;
}
