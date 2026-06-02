#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <map>
#include <string>
#include "Product.h"
#include "User.h"
#include "Order.h"

using namespace std;

class Database {
private:
    string dataDir;
    map<int, Product> products;
    map<string, User> users;
    // username -> { productId -> {viewCount, purchaseCount} }
    map<string, map<int, pair<int, int>>> interactions;
    vector<Order> orders;

    int nextProductId;
    int nextOrderId;

    void ensureDirectoryExists();

public:
    Database(const string& dataDir = "data");

    // Load and Save
    bool loadAll();
    bool saveAll();

    // Product Management
    bool addProduct(const string& name, const string& category, double price);
    bool editProduct(int id, const string& name, const string& category, double price);
    bool deleteProduct(int id);
    const map<int, Product>& getProducts() const;
    const Product* getProduct(int id) const;

    // User Management
    bool registerUser(const string& username, const string& password, const string& role);
    const User* authenticateUser(const string& username, const string& password) const;
    const map<string, User>& getUsers() const;

    // Interaction Logging
    void logView(const string& username, int productId);
    void logPurchase(const string& username, int productId, int quantity);
    const map<int, pair<int, int>>& getUserInteractions(const string& username) const;
    const map<string, map<int, pair<int, int>>>& getAllInteractions() const;

    // Order Processing
    bool checkout(const string& username, ShoppingCart& cart);
    const vector<Order>& getOrders() const;
    vector<Order> getOrdersByUser(const string& username) const;

    // Statistics
    vector<Product> getMostViewedProducts(int limit = 5) const;
    vector<Product> getBestSellingProducts(int limit = 5) const;
    vector<pair<string, int>> getActiveUsers(int limit = 5) const;
};

#endif // DATABASE_H