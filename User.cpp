#include "User.h"
#include "Utils.h"

using namespace std;

User::User() : username(""), password(""), role("") {}

User::User(const string& username, const string& password, const string& role)
    : username(username), password(password), role(role) {}

string User::getUsername() const { return username; }

bool User::verifyPassword(const string& pwd) const { return password == pwd; }

string User::getRole() const { return role; }

string User::serialize() const {
    return username + "|" + password + "|" + role;
}

User User::deserialize(const string& line) {
    auto tokens = Utils::split(line, '|');
    if (tokens.size() >= 3) {
        return User(tokens[0], tokens[1], tokens[2]);
    }
    return User();
}

Customer::Customer() : User("", "", "customer") {}

Customer::Customer(const string& username, const string& password)
    : User(username, password, "customer") {}

ShoppingCart& Customer::getCart() { return cart; }

const ShoppingCart& Customer::getCart() const { return cart; }

Admin::Admin() : User("", "", "admin") {}

Admin::Admin(const string& username, const string& password)
    : User(username, password, "admin") {}
