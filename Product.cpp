#include "Product.h"
#include "Utils.h"

using namespace std;

Product::Product() : id(0), name(""), category(""), price(0.0), views(0), purchases(0) {}

Product::Product(int id, const string& name, const string& category, double price, int views, int purchases)
    : id(id), name(name), category(category), price(price), views(views), purchases(purchases) {}

int Product::getId() const { return id; }
string Product::getName() const { return name; }
string Product::getCategory() const { return category; }
double Product::getPrice() const { return price; }
int Product::getViews() const { return views; }
int Product::getPurchases() const { return purchases; }

void Product::setName(const string& n) { name = n; }
void Product::setCategory(const string& cat) { category = cat; }
void Product::setPrice(double p) { price = p; }
void Product::incrementViews(int count) { views += count; }
void Product::incrementPurchases(int count) { purchases += count; }

string Product::serialize() const {
    return to_string(id) + "|" + name + "|" + category + "|" + to_string(price) + "|" + to_string(views) + "|" + to_string(purchases);
}

Product Product::deserialize(const string& line) {
    auto tokens = Utils::split(line, '|');
    if (tokens.size() >= 6) {
        int id = stoi(tokens[0]);
        string name = tokens[1];
        string category = tokens[2];
        double price = stod(tokens[3]);
        int views = stoi(tokens[4]);
        int purchases = stoi(tokens[5]);
        return Product(id, name, category, price, views, purchases);
    }
    return Product();
}
