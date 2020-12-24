# Order book class

C++ class OrderBook demonstrates the logic of the simplified market order book.
Only simple limit orders are supported.

## Rules of order execution

Following rules is used for order execution.

1. If an bid order comes in at a price greater or equal than the lowest ask price, then we execute order by ask price. The buyer buys at his proposed price or less. The seller sells at his proposed price.

2. Either if an ask order comes in at a price lower or equal to the highest bid price in the order book, then order executed by bid price. The seller sells at his proposed price or more. The buyer buys at his proposed price.

Let's allow partial execution of orders. If the order has not been fully executed, the rest of the order added to the book.

Orders with the same price and type are executed in ascending order of their id.

## Code structure

There are two classes in the library: Order, and OrderBook. Order is helper class representing market order.
Order has type, price, quantity, and id fields. Id is assigned to order internally. Type, price, and quantity are
assigned at the time of order creation. Type can either be Bid, or Ask.

OrderBook class represents market order book. It has following methods.

- **add_order** - to add order to order book. Once order was added to the book, it tries to execute according to the above rules. Returns order id.
- **cancel_order** - cancels order by its id. If order doesn't exist in the book (for example, executed) OrderNotFound exception generated.
- **get_order** - retrieves order information by its id. Also generates OrderNotFound exception if order not found.
- **market_data_1_json** - retrieves market data level 1 information in json format.
- **market_data_2_json** - retrieves market data level 2 information in json format.
- **orderbook_info_json** - retrieves current order book information aggregated by price.

Constructor of OrderBook accepts two optional parameters.

- **executed_order_callback** - callback function accepts Order as parameter. It is called when order is executed.
- **canceled_order_callback** - callback function accepts Order as parameter. It is called when order is canceled.

## Building

Project configured to build code as a static library. CMake minimum version 3.10 and compiler with C++14 support required.  Also ```googletest``` should be installed in the system. If not, use [this instructions](https://gist.github.com/Cartexius/4c437c084d6e388288201aadf9c8cdd5). Use following steps to build the application.

- ```mkdir build && cd build``` to create build folder.
- ```cmake ..``` to create cmake configuration.
- ```cmake --build .``` to build the application.
- ```ctest``` to run tests with cmake.

Static library is output to lib folder.

## Testing

Source code of tests are presented in the folder ```tests```.
Corresponding executable file which can be run with [command line parameters](https://sites.google.com/site/burlachenkok/articles/gtest_usage) is in the folder ```build/tests/order_book_gtest```.
