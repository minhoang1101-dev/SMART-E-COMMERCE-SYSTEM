#include "ShoppingCart.h"

void ShoppingCart::addProduct(int productId, int quantity) {
    if (quantity <= 0) return;
    items[productId] += quantity;
}

void ShoppingCart::removeProduct(int productId) {
    items.erase(productId);
}

void ShoppingCart::updateQuantity(int productId, int quantity) {
    if (quantity <= 0) {
        removeProduct(productId);
    } else {
        items[productId] = quantity;
    }
}

void ShoppingCart::clear() {
    items.clear();
}

const std::map<int, int>& ShoppingCart::getItems() const {
    return items;
}

bool ShoppingCart::isEmpty() const {
    return items.empty();
}
