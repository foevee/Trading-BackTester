# Trading-BackTester
Building a trading backtester in C++

## Part I Overview of the system
The general system design of the trading backtester is as follows.

<img width="650" alt="general_design" src="img/system_overview.png">

There are 6 parts in the backtester: Gateway In, Gateway Out, Book Builder, Strategy, Order Manager and Market Simulator.
The general architecture deisgn is shown below.

<img width="800" alt="architecture" src="img/general_architecture_design.png">


### Trading Strategy


### Market Simulator
The Market Simulator is here to simulate the response from the market. In the implementation, I will reduce the functionality for now to accept a limit order and fills it. It will reject any orders whose quantity is lower 1000.

## Part II Communications
