#include "Order.h"
#include "Utils.h"
#include <algorithm>

using namespace std;

// OrderItem Implementation
string OrderItem::serialize() const {
    string safeName = productName;
    replace(safeName.begin(), safeName.end(), ',', ' ');
    return to_string(productId) + "," + safeName + "," + to_string(quantity) + "," + to_string(priceAtPurchase);
}

OrderItem OrderItem::deserialize(const string& token) {
    auto fields = Utils::split(token, ',');
    if (fields.size() >= 4) {
        int id = stoi(fields[0]);
        string name = fields[1];
        int qty = stoi(fields[2]);
        double price = stod(fields[3]);
        return {id, name, qty, price};
    }
    return {0, "", 0, 0.0};
}

// Order Implementation
Order::Order() : orderId(0), username(""), totalPrice(0.0), timestamp("") {}

Order::Order(int orderId, const string& username, const vector<OrderItem>& items, double totalPrice, const string& timestamp)
    : orderId(orderId), username(username), items(items), totalPrice(totalPrice), timestamp(timestamp) {}

int Order::getOrderId() const { return orderId; }
string Order::getUsername() const { return username; }
const vector<OrderItem>& Order::getItems() const { return items; }
double Order::getTotalPrice() const { return totalPrice; }
string Order::getTimestamp() const { return timestamp; }

string Order::serialize() const {
    string itemsSerialized = "";
    for (size_t i = 0; i < items.size(); ++i) {
        itemsSerialized += items[i].serialize();
        if (i < items.size() - 1) {
            itemsSerialized += ";";
        }
    }
    return to_string(orderId) + "|" + username + "|" + to_string(totalPrice) + "|" + timestamp + "|" + itemsSerialized;
}

Order Order::deserialize(const string& line) {
    auto tokens = Utils::split(line, '|');
    if (tokens.size() >= 5) {
        int id = stoi(tokens[0]);
        string user = tokens[1];
        double total = stod(tokens[2]);
        string ts = tokens[3];
        
        vector<OrderItem> orderItems;
        if (tokens.size() > 4 && !tokens[4].empty()) {
            auto itemTokens = Utils::split(tokens[4], ';');
            for (const auto& itemToken : itemTokens) {
                if (!itemToken.empty()) {
                    orderItems.push_back(OrderItem::deserialize(itemToken));
                }
            }
        }
        return Order(id, user, orderItems, total, ts);
    }
    return Order();
}
