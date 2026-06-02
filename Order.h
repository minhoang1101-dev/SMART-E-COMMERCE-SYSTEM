#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <vector>

using namespace std;

struct OrderItem {
    int productId;
    string productName;
    int quantity;
    double priceAtPurchase;

    string serialize() const;
    static OrderItem deserialize(const string& token);
};

class Order {
private:
    int orderId;
    string username;
    vector<OrderItem> items;
    double totalPrice;
    string timestamp;

public:
    Order();
    Order(int orderId, const string& username, const vector<OrderItem>& items, double totalPrice, const string& timestamp);

    int getOrderId() const;
    string getUsername() const;
    const vector<OrderItem>& getItems() const;
    double getTotalPrice() const;
    string getTimestamp() const;

    string serialize() const;
    static Order deserialize(const string& line);
};

#endif // ORDER_H