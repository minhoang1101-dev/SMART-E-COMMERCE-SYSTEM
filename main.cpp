#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <limits>
#include <algorithm>
#include "Database.h"
#include "RecommendationEngine.h"
#include "ShoppingCart.h"
#include "Utils.h"

using namespace std;

// Clear input stream in case of invalid inputs
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void printBanner() {
    cout << "\n+===================================================+\n";
    cout << "|        SMART E-COMMERCE SHOPPING PLATFORM         |\n";
    cout << "|           With AI-powered Recommendations         |\n";
    cout << "+===================================================+\n";
}

void displayProductTable(const vector<Product>& productsList) {
    if (productsList.empty()) {
        cout << "\nNo products available.\n";
        return;
    }

    cout << "\n+-----+-------------------------------+--------------------+------------+\n";
    cout << "| ID  | Name                          | Category           | Price ($)  |\n";
    cout << "+-----+-------------------------------+--------------------+------------+\n";
    for (const auto& prod : productsList) {
        cout << "| " << left << setw(3) << prod.getId()
             << " | " << left << setw(29) << (prod.getName().length() > 29 ? prod.getName().substr(0, 26) + "..." : prod.getName())
             << " | " << left << setw(18) << (prod.getCategory().length() > 18 ? prod.getCategory().substr(0, 15) + "..." : prod.getCategory())
             << " | " << right << setw(10) << fixed << setprecision(2) << prod.getPrice() << " |\n";
    }
    cout << "+-----+-------------------------------+--------------------+------------+\n";
}

void displayProductDetails(const Product& prod, const string& username, Database& db) {
    db.logView(username, prod.getId()); // Increment views
    const Product* updatedProd = db.getProduct(prod.getId());
    const Product& displayProd = updatedProd ? *updatedProd : prod;

    cout << "\n================= PRODUCT DETAILS =================\n";
    cout << "  ID:          " << displayProd.getId() << "\n";
    cout << "  Name:        " << displayProd.getName() << "\n";
    cout << "  Category:    " << displayProd.getCategory() << "\n";
    cout << "  Price:       $" << fixed << setprecision(2) << displayProd.getPrice() << "\n";
    cout << "  Global Views: " << displayProd.getViews() << " | Purchases: " << displayProd.getPurchases() << "\n";
    cout << "===================================================\n";
}

void displayCart(const ShoppingCart& cart, const Database& db) {
    if (cart.isEmpty()) {
        cout << "\nYour shopping cart is empty.\n";
        return;
    }

    cout << "\n+-----+-------------------------------+------------+-----+------------+\n";
    cout << "| ID  | Name                          | Price ($)  | Qty | Total ($)  |\n";
    cout << "+-----+-------------------------------+------------+-----+------------+\n";
    double grantTotal = 0.0;
    for (const auto& item : cart.getItems()) {
        int prodId = item.first;
        int qty = item.second;
        const Product* prod = db.getProduct(prodId);
        if (!prod) continue;
        double itemTotal = prod->getPrice() * qty;
        grantTotal += itemTotal;
        cout << "| " << left << setw(3) << prodId
             << " | " << left << setw(29) << (prod->getName().length() > 29 ? prod->getName().substr(0, 26) + "..." : prod->getName())
             << " | " << right << setw(10) << fixed << setprecision(2) << prod->getPrice()
             << " | " << right << setw(3) << qty
             << " | " << right << setw(10) << fixed << setprecision(2) << itemTotal << " |\n";
    }
    cout << "+-----+-------------------------------+------------+-----+------------+\n";
    cout << "| " << right << setw(49) << "GRAND TOTAL:" << " | $" << right << setw(9) << fixed << setprecision(2) << grantTotal << " |\n";
    cout << "+--------------------------------------------------+--------------+\n";
}

void customerFlow(const string& username, Database& db) {
    ShoppingCart cart;

    while (true) {
        cout << "\n>>> CUSTOMER DASHBOARD (" << username << ") <<<\n";
        cout << "1. Browse Product Catalog\n";
        cout << "2. Search Products\n";
        cout << "3. Get Personal Recommendations\n";
        cout << "4. View Shopping Cart\n";
        cout << "5. Order History\n";
        cout << "6. Logout\n";
        cout << "Choose an option: ";

        int choice;
        cin >> choice;
        if (cin.fail()) {
            clearInput();
            cout << "Invalid input. Please try again.\n";
            continue;
        }

        if (choice == 1) {
            // Browse Products
            vector<Product> prods;
            for (const auto& pair : db.getProducts()) {
                prods.push_back(pair.second);
            }
            displayProductTable(prods);

            cout << "\nEnter Product ID to view details (or 0 to go back): ";
            int prodId;
            cin >> prodId;
            if (cin.fail()) {
                clearInput();
                continue;
            }

            if (prodId > 0) {
                const Product* prod = db.getProduct(prodId);
                if (prod) {
                    displayProductDetails(*prod, username, db);
                    cout << "1. Add to Cart\n";
                    cout << "2. Go Back\n";
                    cout << "Choose an option: ";
                    int detailChoice;
                    cin >> detailChoice;
                    if (cin.fail()) {
                        clearInput();
                        continue;
                    }
                    if (detailChoice == 1) {
                        cout << "Enter quantity: ";
                        int qty;
                        cin >> qty;
                        if (cin.fail() || qty <= 0) {
                            clearInput();
                            cout << "Invalid quantity.\n";
                            continue;
                        }
                        cart.addProduct(prodId, qty);
                        cout << "Product added to cart successfully!\n";
                    }
                } else {
                    cout << "Product not found.\n";
                }
            }
        } else if (choice == 2) {
            // Search Products
            cout << "Enter search keyword (for name or category): ";
            string keyword;
            cin >> keyword;
            vector<Product> results;
            for (const auto& pair : db.getProducts()) {
                string nameLower = pair.second.getName();
                string catLower = pair.second.getCategory();
                
                transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                transform(catLower.begin(), catLower.end(), catLower.begin(), ::tolower);
                string keywordLower = keyword;
                transform(keywordLower.begin(), keywordLower.end(), keywordLower.begin(), ::tolower);

                if (nameLower.find(keywordLower) != string::npos || catLower.find(keywordLower) != string::npos) {
                    results.push_back(pair.second);
                }
            }
            displayProductTable(results);
        } else if (choice == 3) {
            // Recommendations
            auto recs = RecommendationEngine::getRecommendations(username, db, 5);
            cout << "\n================ PERSONAL RECOMMENDATIONS ================\n";
            if (recs.empty()) {
                cout << "No recommendations available yet. Browse or purchase products first!\n";
            } else {
                cout << "+-----+-------------------------------+--------------------+------------+---------+\n";
                cout << "| ID  | Name                          | Category           | Price ($)  | Score   |\n";
                cout << "+-----+-------------------------------+--------------------+------------+---------+\n";
                for (const auto& pair : recs) {
                    const Product& prod = pair.first;
                    double score = pair.second;
                    cout << "| " << left << setw(3) << prod.getId()
                         << " | " << left << setw(29) << (prod.getName().length() > 29 ? prod.getName().substr(0, 26) + "..." : prod.getName())
                         << " | " << left << setw(18) << (prod.getCategory().length() > 18 ? prod.getCategory().substr(0, 15) + "..." : prod.getCategory())
                         << " | " << right << setw(10) << fixed << setprecision(2) << prod.getPrice()
                         << " | " << right << setw(7) << fixed << setprecision(1) << score << " |\n";
                }
                cout << "+-----+-------------------------------+--------------------+------------+---------+\n";
            }
        } else if (choice == 4) {
            // View Cart
            displayCart(cart, db);
            if (!cart.isEmpty()) {
                cout << "\n1. Checkout\n";
                cout << "2. Update Quantity\n";
                cout << "3. Remove Product\n";
                cout << "4. Go Back\n";
                cout << "Choose an option: ";
                int cartChoice;
                cin >> cartChoice;
                if (cin.fail()) {
                    clearInput();
                    continue;
                }
                if (cartChoice == 1) {
                    if (db.checkout(username, cart)) {
                        cout << "Checkout completed successfully! Your order has been placed.\n";
                    } else {
                        cout << "Checkout failed.\n";
                    }
                } else if (cartChoice == 2) {
                    cout << "Enter Product ID: ";
                    int pid;
                    cin >> pid;
                    cout << "Enter new quantity: ";
                    int qty;
                    cin >> qty;
                    if (cin.fail()) {
                        clearInput();
                        continue;
                    }
                    cart.updateQuantity(pid, qty);
                    cout << "Cart updated.\n";
                } else if (cartChoice == 3) {
                    cout << "Enter Product ID: ";
                    int pid;
                    cin >> pid;
                    if (cin.fail()) {
                        clearInput();
                        continue;
                    }
                    cart.removeProduct(pid);
                    cout << "Product removed from cart.\n";
                }
            }
        } else if (choice == 5) {
            // Order History
            auto userOrders = db.getOrdersByUser(username);
            cout << "\n================ ORDER HISTORY ================\n";
            if (userOrders.empty()) {
                cout << "You have not placed any orders yet.\n";
            } else {
                for (const auto& o : userOrders) {
                    cout << "Order ID: " << o.getOrderId() << " | Date: " << o.getTimestamp() << "\n";
                    cout << "Total Price: $" << fixed << setprecision(2) << o.getTotalPrice() << "\n";
                    cout << "Items:\n";
                    for (const auto& item : o.getItems()) {
                        cout << "  - " << item.productName << " (x" << item.quantity << ") @ $" << item.priceAtPurchase << " each\n";
                    }
                    cout << "-----------------------------------------------\n";
                }
            }
        } else if (choice == 6) {
            cout << "Logged out successfully.\n";
            break;
        } else {
            cout << "Invalid choice. Please select 1-6.\n";
        }
    }
}

void adminFlow(Database& db) {
    while (true) {
        cout << "\n>>> ADMINISTRATOR CONTROL PANEL <<<\n";
        cout << "1. Manage Products (Add/Edit/Delete)\n";
        cout << "2. View System Statistics\n";
        cout << "3. View All Orders\n";
        cout << "4. Logout\n";
        cout << "Choose an option: ";

        int choice;
        cin >> choice;
        if (cin.fail()) {
            clearInput();
            cout << "Invalid input. Please try again.\n";
            continue;
        }

        if (choice == 1) {
            cout << "\n1. Add Product\n";
            cout << "2. Edit Product\n";
            cout << "3. Delete Product\n";
            cout << "4. Go Back\n";
            cout << "Choose an option: ";
            int prodChoice;
            cin >> prodChoice;
            if (cin.fail()) {
                clearInput();
                continue;
            }

            if (prodChoice == 1) {
                clearInput();
                cout << "Enter product name: ";
                string name;
                getline(cin, name);
                cout << "Enter category: ";
                string cat;
                getline(cin, cat);
                cout << "Enter price: ";
                double price;
                cin >> price;
                if (cin.fail() || price < 0) {
                    clearInput();
                    cout << "Invalid price. Product add aborted.\n";
                    continue;
                }
                if (db.addProduct(name, cat, price)) {
                    cout << "Product added successfully!\n";
                } else {
                    cout << "Failed to add product.\n";
                }
            } else if (prodChoice == 2) {
                cout << "Enter Product ID to edit: ";
                int pid;
                cin >> pid;
                if (cin.fail()) {
                    clearInput();
                    continue;
                }
                const Product* prod = db.getProduct(pid);
                if (!prod) {
                    cout << "Product not found.\n";
                    continue;
                }
                clearInput();
                cout << "Enter new name (leave blank to keep current: '" << prod->getName() << "'): ";
                string name;
                getline(cin, name);
                cout << "Enter new category (leave blank to keep current: '" << prod->getCategory() << "'): ";
                string cat;
                getline(cin, cat);
                cout << "Enter new price (enter -1 to keep current: $" << prod->getPrice() << "): ";
                double price;
                cin >> price;
                if (cin.fail()) {
                    clearInput();
                    price = -1.0;
                }
                if (db.editProduct(pid, name, cat, price)) {
                    cout << "Product updated successfully!\n";
                } else {
                    cout << "Failed to update product.\n";
                }
            } else if (prodChoice == 3) {
                cout << "Enter Product ID to delete: ";
                int pid;
                cin >> pid;
                if (cin.fail()) {
                    clearInput();
                    continue;
                }
                if (db.deleteProduct(pid)) {
                    cout << "Product deleted successfully!\n";
                } else {
                    cout << "Failed to delete product. ID may be invalid.\n";
                }
            }
        } else if (choice == 2) {
            cout << "\n================ SYSTEM STATISTICS ================\n";
            
            // 1. Most Viewed Products
            auto mostViewed = db.getMostViewedProducts(5);
            cout << "\n--- TOP MOST VIEWED PRODUCTS ---\n";
            cout << "+-----+-------------------------------+--------------------+------------+-------+\n";
            cout << "| ID  | Name                          | Category           | Price ($)  | Views |\n";
            cout << "+-----+-------------------------------+--------------------+------------+-------+\n";
            for (const auto& prod : mostViewed) {
                cout << "| " << left << setw(3) << prod.getId()
                     << " | " << left << setw(29) << (prod.getName().length() > 29 ? prod.getName().substr(0, 26) + "..." : prod.getName())
                     << " | " << left << setw(18) << (prod.getCategory().length() > 18 ? prod.getCategory().substr(0, 15) + "..." : prod.getCategory())
                     << " | " << right << setw(10) << fixed << setprecision(2) << prod.getPrice()
                     << " | " << right << setw(5) << prod.getViews() << " |\n";
            }
            cout << "+-----+-------------------------------+--------------------+------------+-------+\n";

            // 2. Best-Selling Products
            auto bestSelling = db.getBestSellingProducts(5);
            cout << "\n--- TOP BEST SELLING PRODUCTS ---\n";
            cout << "+-----+-------------------------------+--------------------+------------+-----------+\n";
            cout << "| ID  | Name                          | Category           | Price ($)  | Purchases |\n";
            cout << "+-----+-------------------------------+--------------------+------------+-----------+\n";
            for (const auto& prod : bestSelling) {
                cout << "| " << left << setw(3) << prod.getId()
                     << " | " << left << setw(29) << (prod.getName().length() > 29 ? prod.getName().substr(0, 26) + "..." : prod.getName())
                     << " | " << left << setw(18) << (prod.getCategory().length() > 18 ? prod.getCategory().substr(0, 15) + "..." : prod.getCategory())
                     << " | " << right << setw(10) << fixed << setprecision(2) << prod.getPrice()
                     << " | " << right << setw(9) << prod.getPurchases() << " |\n";
            }
            cout << "+-----+-------------------------------+--------------------+------------+-----------+\n";

            // 3. Active Users
            auto activeUsers = db.getActiveUsers(5);
            cout << "\n--- ACTIVE USERS ---\n";
            cout << "+------------------------------+-------------------+\n";
            cout << "| Username                     | Total Interactions |\n";
            cout << "+------------------------------+-------------------+\n";
            for (const auto& userPair : activeUsers) {
                cout << "| " << left << setw(28) << userPair.first
                     << " | " << right << setw(17) << userPair.second << " |\n";
            }
            cout << "+------------------------------+-------------------+\n";
            cout << "===================================================\n";
        } else if (choice == 3) {
            // View All Orders
            const auto& allOrders = db.getOrders();
            cout << "\n================ SYSTEM ORDER HISTORY ================\n";
            if (allOrders.empty()) {
                cout << "No orders have been placed in the system yet.\n";
            } else {
                for (const auto& o : allOrders) {
                    cout << "Order ID: " << o.getOrderId() << " | User: " << o.getUsername() << " | Date: " << o.getTimestamp() << "\n";
                    cout << "Total Price: $" << fixed << setprecision(2) << o.getTotalPrice() << "\n";
                    cout << "Items:\n";
                    for (const auto& item : o.getItems()) {
                        cout << "  - " << item.productName << " (x" << item.quantity << ") @ $" << item.priceAtPurchase << " each\n";
                    }
                    cout << "-----------------------------------------------\n";
                }
            }
        } else if (choice == 4) {
            cout << "Admin logged out successfully.\n";
            break;
        } else {
            cout << "Invalid choice. Please select 1-4.\n";
        }
    }
}

int main() {
    Database db("data");
    db.loadAll();

    printBanner();

    while (true) {
        cout << "\n=============== MAIN MENU ===============\n";
        cout << "1. Login\n";
        cout << "2. Register New Customer Account\n";
        cout << "3. Exit System\n";
        cout << "Choose an option: ";

        int choice;
        cin >> choice;
        if (cin.fail()) {
            clearInput();
            cout << "Invalid input. Please try again.\n";
            continue;
        }

        if (choice == 1) {
            cout << "\nEnter username: ";
            string username;
            cin >> username;
            cout << "Enter password: ";
            string password;
            cin >> password;

            const User* user = db.authenticateUser(username, password);
            if (user) {
                cout << "\nWelcome back, " << username << "! Successfully logged in as: " << user->getRole() << "\n";
                if (user->getRole() == "admin") {
                    adminFlow(db);
                } else {
                    customerFlow(username, db);
                }
            } else {
                cout << "\nAuthentication failed. Invalid username or password.\n";
            }
        } else if (choice == 2) {
            cout << "\nEnter desired username (no spaces): ";
            string username;
            cin >> username;
            cout << "Enter desired password: ";
            string password;
            cin >> password;

            if (db.registerUser(username, password, "customer")) {
                cout << "\nRegistration successful! You can now log in.\n";
            } else {
                cout << "\nRegistration failed. Username may already be taken.\n";
            }
        } else if (choice == 3) {
            cout << "\nThank you for using Smart E-Commerce Platform. Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice. Please select 1-3.\n";
        }
    }

    return 0;
}
