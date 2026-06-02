#ifndef SHOPPING_CART_H
#define SHOPPING_CART_H

#include <map>

using namespace std;

class ShoppingCart {
private:
    map<int, int> items; // productId -> quantity

public:
    void addProduct(int productId, int quantity = 1);
    void removeProduct(int productId);
    void updateQuantity(int productId, int quantity);
    void clear();
    const map<int, int>& getItems() const;
    bool isEmpty() const;
};

#endif // SHOPPING_CART_H