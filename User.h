#ifndef USER_H
#define USER_H

#include <string>
#include "ShoppingCart.h"

using namespace std;

class User {
protected:
    string username;
    string password;
    string role; // "customer" or "admin"

public:
    User();
    User(const string& username, const string& password, const string& role);
    virtual ~User() = default;

    string getUsername() const;
    bool verifyPassword(const string& pwd) const;
    string getRole() const;

    virtual string serialize() const;
    static User deserialize(const string& line);
};

class Customer : public User {
private:
    ShoppingCart cart;

public:
    Customer();
    Customer(const string& username, const string& password);

    ShoppingCart& getCart();
    const ShoppingCart& getCart() const;
};

class Admin : public User {
public:
    Admin();
    Admin(const string& username, const string& password);
};

#endif // USER_H