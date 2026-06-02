#include <iostream>
#include <cassert>
#include <filesystem>
#include "Database.h"
#include "RecommendationEngine.h"
#include "ShoppingCart.h"
#include "Utils.h"

namespace fs = std::filesystem;
using namespace std;

void cleanTestDir(const string& dir) {
    if (fs::exists(dir)) {
        fs::remove_all(dir);
    }
}

void testDatabasePersistence() {
    cout << "[TEST] Running Database Persistence Test...\n";
    string testDir = "data_test";
    cleanTestDir(testDir);

    {
        Database db(testDir);
        
        // Register some users
        assert(db.registerUser("alice", "pass123", "customer") == true);
        assert(db.registerUser("bob", "pass456", "customer") == true);
        assert(db.registerUser("admin", "admin123", "admin") == true);
        // Duplicate registration should fail
        assert(db.registerUser("alice", "newpass", "customer") == false);

        // Add products
        assert(db.addProduct("Laptop", "Electronics", 999.99) == true);
        assert(db.addProduct("Python Book", "Books", 29.99) == true);
        assert(db.addProduct("C++ Book", "Books", 49.99) == true);

        // Verify loaded in-memory
        assert(db.getProducts().size() == 3);
        assert(db.getUsers().size() == 3);

        // Add some interactions
        db.logView("alice", 1); // Laptop view
        db.logView("alice", 2); // Python Book view
        db.logView("alice", 2); // Python Book second view
        db.logPurchase("alice", 2, 1); // Python Book purchase
    } // db goes out of scope and saves files

    // Reload in a new instance and check
    {
        Database db2(testDir);
        db2.loadAll();

        assert(db2.getProducts().size() == 3);
        assert(db2.getUsers().size() == 3);

        const Product* p1 = db2.getProduct(1);
        const Product* p2 = db2.getProduct(2);
        
        assert(p1 != nullptr);
        assert(p2 != nullptr);

        assert(p1->getName() == "Laptop");
        assert(p1->getCategory() == "Electronics");
        assert(p1->getPrice() == 999.99);
        assert(p1->getViews() == 1);
        assert(p1->getPurchases() == 0);

        assert(p2->getName() == "Python Book");
        assert(p2->getCategory() == "Books");
        assert(p2->getPrice() == 29.99);
        assert(p2->getViews() == 2);
        assert(p2->getPurchases() == 1);

        const auto& aliceInteractions = db2.getUserInteractions("alice");
        assert(aliceInteractions.at(1).first == 1); // laptop view
        assert(aliceInteractions.at(2).first == 2); // book view
        assert(aliceInteractions.at(2).second == 1); // book purchase
    }

    cleanTestDir(testDir);
    cout << "[TEST] Database Persistence Test: PASSED\n\n";
}

void testRecommendationEngine() {
    cout << "[TEST] Running Recommendation Engine Test...\n";
    string testDir = "data_test_rec";
    cleanTestDir(testDir);

    Database db(testDir);
    db.addProduct("Gaming Laptop", "Electronics", 1200.00); // ID 1
    db.addProduct("USB-C Hub", "Electronics", 35.00);      // ID 2
    db.addProduct("C++ Programming", "Books", 50.00);       // ID 3
    db.addProduct("Design Patterns", "Books", 60.00);       // ID 4
    db.addProduct("Espresso Machine", "Home", 250.00);      // ID 5

    // User interaction history:
    db.logView("user1", 3);
    db.logView("user1", 3);
    db.logView("user1", 3);
    db.logView("user1", 3);
    db.logPurchase("user1", 3, 2);

    db.logView("user1", 1);
    db.logPurchase("user1", 1, 1);

    auto recs = RecommendationEngine::getRecommendations("user1", db, 5);

    assert(recs.size() == 5);
    
    assert(recs[0].first.getId() == 3);
    assert(abs(recs[0].second - 7.4) < 0.001);

    assert(recs[1].first.getId() == 4);
    assert(abs(recs[1].second - 5.0) < 0.001);

    assert(recs[2].first.getId() == 1);
    assert(abs(recs[2].second - 1.0) < 0.001);

    assert(abs(recs[3].second - 0.0) < 0.001);
    assert(abs(recs[4].second - 0.0) < 0.001);

    cleanTestDir(testDir);
    cout << "[TEST] Recommendation Engine Test: PASSED\n\n";
}

void testCheckoutFlow() {
    cout << "[TEST] Running Shopping Cart & Checkout Test...\n";
    string testDir = "data_test_cart";
    cleanTestDir(testDir);

    Database db(testDir);
    db.addProduct("Item A", "General", 10.00); // ID 1
    db.addProduct("Item B", "General", 15.00); // ID 2

    ShoppingCart cart;
    cart.addProduct(1, 2); // 2 of Item A ($20.00)
    cart.addProduct(2, 3); // 3 of Item B ($45.00)

    assert(db.checkout("customer1", cart) == true);
    assert(cart.isEmpty() == true);

    const auto& orders = db.getOrders();
    assert(orders.size() == 1);
    assert(orders[0].getUsername() == "customer1");
    assert(orders[0].getTotalPrice() == 65.00);
    assert(orders[0].getItems().size() == 2);

    // Verify interaction records were updated by purchase
    const auto& interactions = db.getUserInteractions("customer1");
    assert(interactions.at(1).second == 2); // 2 purchases of Item A
    assert(interactions.at(2).second == 3); // 3 purchases of Item B

    cleanTestDir(testDir);
    cout << "[TEST] Shopping Cart & Checkout Test: PASSED\n\n";
}

int main() {
    cout << "==================================================\n";
    cout << "           RUNNING C++ OOP TEST SUITE             \n";
    cout << "==================================================\n\n";

    testDatabasePersistence();
    testRecommendationEngine();
    testCheckoutFlow();

    cout << "All verification tests passed successfully!\n";
    return 0;
}
