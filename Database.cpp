#include "Database.h"
#include "Utils.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <ctime>

namespace fs = std::filesystem;
using namespace std;

Database::Database(const string& dataDir) : dataDir(dataDir), nextProductId(1), nextOrderId(1) {
    ensureDirectoryExists();
}

void Database::ensureDirectoryExists() {
    if (!dataDir.empty() && !fs::exists(dataDir)) {
        fs::create_directories(dataDir);
    }
}

bool Database::loadAll() {
    products.clear();
    users.clear();
    interactions.clear();
    orders.clear();
    nextProductId = 1;
    nextOrderId = 1;

    // Load Products
    ifstream prodFile(dataDir + "/products.txt");
    if (prodFile.is_open()) {
        string line;
        while (getline(prodFile, line)) {
            line = Utils::trim(line);
            if (line.empty()) continue;
            Product p = Product::deserialize(line);
            if (p.getId() > 0) {
                products[p.getId()] = p;
                if (p.getId() >= nextProductId) {
                    nextProductId = p.getId() + 1;
                }
            }
        }
        prodFile.close();
    }

    // Load Users
    ifstream userFile(dataDir + "/users.txt");
    if (userFile.is_open()) {
        string line;
        while (getline(userFile, line)) {
            line = Utils::trim(line);
            if (line.empty()) continue;
            User u = User::deserialize(line);
            if (!u.getUsername().empty()) {
                users[u.getUsername()] = u;
            }
        }
        userFile.close();
    }

    if (users.empty()) {
        users["admin"] = User("admin", "admin123", "admin");
    }

    // Load Interactions
    ifstream interFile(dataDir + "/interactions.txt");
    if (interFile.is_open()) {
        string line;
        while (getline(interFile, line)) {
            line = Utils::trim(line);
            if (line.empty()) continue;
            auto tokens = Utils::split(line, '|');
            if (tokens.size() >= 4) {
                string username = tokens[0];
                int prodId = stoi(tokens[1]);
                int views = stoi(tokens[2]);
                int purchases = stoi(tokens[3]);
                interactions[username][prodId] = {views, purchases};
            }
        }
        interFile.close();
    }

    // Load Orders
    ifstream orderFile(dataDir + "/orders.txt");
    if (orderFile.is_open()) {
        string line;
        while (getline(orderFile, line)) {
            line = Utils::trim(line);
            if (line.empty()) continue;
            Order o = Order::deserialize(line);
            if (o.getOrderId() > 0) {
                orders.push_back(o);
                if (o.getOrderId() >= nextOrderId) {
                    nextOrderId = o.getOrderId() + 1;
                }
            }
        }
        orderFile.close();
    }

    return true;
}

bool Database::saveAll() {
    ensureDirectoryExists();

    // Save Products
    ofstream prodFile(dataDir + "/products.txt");
    if (!prodFile.is_open()) return false;
    for (const auto& pair : products) {
        prodFile << pair.second.serialize() << "\n";
    }
    prodFile.close();

    // Save Users
    ofstream userFile(dataDir + "/users.txt");
    if (!userFile.is_open()) return false;
    for (const auto& pair : users) {
        userFile << pair.second.serialize() << "\n";
    }
    userFile.close();

    // Save Interactions
    ofstream interFile(dataDir + "/interactions.txt");
    if (!interFile.is_open()) return false;
    for (const auto& userPair : interactions) {
        const string& username = userPair.first;
        for (const auto& prodPair : userPair.second) {
            int prodId = prodPair.first;
            int views = prodPair.second.first;
            int purchases = prodPair.second.second;
            interFile << username << "|" << prodId << "|" << views << "|" << purchases << "\n";
        }
    }
    interFile.close();

    // Save Orders
    ofstream orderFile(dataDir + "/orders.txt");
    if (!orderFile.is_open()) return false;
    for (const auto& o : orders) {
        orderFile << o.serialize() << "\n";
    }
    orderFile.close();

    return true;
}

bool Database::addProduct(const string& name, const string& category, double price) {
    if (name.empty() || category.empty() || price < 0.0) return false;
    int id = nextProductId++;
    products[id] = Product(id, name, category, price);
    return saveAll();
}

bool Database::editProduct(int id, const string& name, const string& category, double price) {
    auto it = products.find(id);
    if (it == products.end()) return false;
    if (!name.empty()) it->second.setName(name);
    if (!category.empty()) it->second.setCategory(category);
    if (price >= 0.0) it->second.setPrice(price);
    return saveAll();
}

bool Database::deleteProduct(int id) {
    auto it = products.find(id);
    if (it == products.end()) return false;
    products.erase(it);
    return saveAll();
}

const map<int, Product>& Database::getProducts() const {
    return products;
}

const Product* Database::getProduct(int id) const {
    auto it = products.find(id);
    if (it != products.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool Database::registerUser(const string& username, const string& password, const string& role) {
    if (username.empty() || password.empty()) return false;
    if (users.find(username) != users.end()) return false;
    users[username] = User(username, password, role);
    return saveAll();
}

const User* Database::authenticateUser(const string& username, const string& password) const {
    auto it = users.find(username);
    if (it != users.end() && it->second.verifyPassword(password)) {
        return &(it->second);
    }
    return nullptr;
}

const map<string, User>& Database::getUsers() const {
    return users;
}

void Database::logView(const string& username, int productId) {
    auto it = products.find(productId);
    if (it != products.end()) {
        it->second.incrementViews();
    }
    if (!username.empty()) {
        interactions[username][productId].first++;
    }
    saveAll();
}

void Database::logPurchase(const string& username, int productId, int quantity) {
    auto it = products.find(productId);
    if (it != products.end()) {
        it->second.incrementPurchases(quantity);
    }
    if (!username.empty()) {
        interactions[username][productId].second += quantity;
    }
    saveAll();
}

const map<int, pair<int, int>>& Database::getUserInteractions(const string& username) const {
    static const map<int, pair<int, int>> empty;
    auto it = interactions.find(username);
    if (it != interactions.end()) {
        return it->second;
    }
    return empty;
}

const map<string, map<int, pair<int, int>>>& Database::getAllInteractions() const {
    return interactions;
}

bool Database::checkout(const string& username, ShoppingCart& cart) {
    if (cart.isEmpty()) return false;

    double total = 0.0;
    vector<OrderItem> orderItems;
    
    for (const auto& item : cart.getItems()) {
        int prodId = item.first;
        int qty = item.second;
        
        auto it = products.find(prodId);
        if (it == products.end()) continue;

        double price = it->second.getPrice();
        total += price * qty;
        
        logPurchase(username, prodId, qty);
        orderItems.push_back({prodId, it->second.getName(), qty, price});
    }

    if (orderItems.empty()) return false;

    time_t now = time(nullptr);
    char buf[100];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    string timestamp(buf);

    int id = nextOrderId++;
    orders.push_back(Order(id, username, orderItems, total, timestamp));
    cart.clear();
    
    return saveAll();
}

const vector<Order>& Database::getOrders() const {
    return orders;
}

vector<Order> Database::getOrdersByUser(const string& username) const {
    vector<Order> userOrders;
    for (const auto& o : orders) {
        if (o.getUsername() == username) {
            userOrders.push_back(o);
        }
    }
    return userOrders;
}

vector<Product> Database::getMostViewedProducts(int limit) const {
    vector<Product> list;
    for (const auto& pair : products) {
        list.push_back(pair.second);
    }
    sort(list.begin(), list.end(), [](const Product& a, const Product& b) {
        return a.getViews() > b.getViews();
    });
    if ((int)list.size() > limit) {
        list.resize(limit);
    }
    return list;
}

vector<Product> Database::getBestSellingProducts(int limit) const {
    vector<Product> list;
    for (const auto& pair : products) {
        list.push_back(pair.second);
    }
    sort(list.begin(), list.end(), [](const Product& a, const Product& b) {
        return a.getPurchases() > b.getPurchases();
    });
    if ((int)list.size() > limit) {
        list.resize(limit);
    }
    return list;
}

vector<pair<string, int>> Database::getActiveUsers(int limit) const {
    map<string, int> activityCounts;
    
    for (const auto& userPair : interactions) {
        const string& username = userPair.first;
        int count = 0;
        for (const auto& prodPair : userPair.second) {
            count += prodPair.second.first + prodPair.second.second;
        }
        activityCounts[username] = count;
    }

    vector<pair<string, int>> sortedUsers;
    for (const auto& pair : activityCounts) {
        sortedUsers.push_back(pair);
    }

    sort(sortedUsers.begin(), sortedUsers.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second;
    });

    if ((int)sortedUsers.size() > limit) {
        sortedUsers.resize(limit);
    }
    return sortedUsers;
}
