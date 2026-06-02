#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>
#include <iostream>

using namespace std;

class Product {
private:
    int id;
    string name;
    string category;
    double price;
    int views;
    int purchases;

public:
    Product();
    Product(int id, const string& name, const string& category, double price, int views = 0, int purchases = 0);

    int getId() const;
    string getName() const;
    string getCategory() const;
    double getPrice() const;
    int getViews() const;
    int getPurchases() const;

    void setName(const string& name);
    void setCategory(const string& category);
    void setPrice(double price);
    void incrementViews(int count = 1);
    void incrementPurchases(int count = 1);

    string serialize() const;
    static Product deserialize(const string& line);
};

#endif // PRODUCT_H