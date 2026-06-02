#include "RecommendationEngine.h"
#include <map>
#include <algorithm>

using namespace std;

vector<pair<Product, double>> RecommendationEngine::getRecommendations(
    const string& username, 
    const Database& db, 
    int limit
) {
    const auto& allProducts = db.getProducts();
    const auto& userInteractions = db.getUserInteractions(username);

    // 1. Identify the user's favorite category
    map<string, int> categoryInteractions;
    for (const auto& interaction : userInteractions) {
        int prodId = interaction.first;
        int views = interaction.second.first;
        int purchases = interaction.second.second;

        const Product* prod = db.getProduct(prodId);
        if (prod) {
            categoryInteractions[prod->getCategory()] += (views + purchases);
        }
    }

    string favoriteCategory = "";
    int maxInteractions = 0;
    for (const auto& catPair : categoryInteractions) {
        if (catPair.second > maxInteractions) {
            maxInteractions = catPair.second;
            favoriteCategory = catPair.first;
        }
    }

    // 2. Score all products in the database
    vector<pair<Product, double>> scoredProducts;

    for (const auto& prodPair : allProducts) {
        const Product& prod = prodPair.second;
        int prodId = prod.getId();

        int userViews = 0;
        int userPurchases = 0;

        auto it = userInteractions.find(prodId);
        if (it != userInteractions.end()) {
            userViews = it->second.first;
            userPurchases = it->second.second;
        }

        // Category bonus
        double categoryBonus = 0.0;
        if (!favoriteCategory.empty() && prod.getCategory() == favoriteCategory) {
            categoryBonus = 5.0; // Bonus magnitude
        }

        // score = view_count * 0.2 + purchase_count * 0.8 + category_bonus
        double score = (userViews * 0.2) + (userPurchases * 0.8) + categoryBonus;

        scoredProducts.push_back({prod, score});
    }

    // 3. Sort products by score descending
    sort(scoredProducts.begin(), scoredProducts.end(), [](const pair<Product, double>& a, const pair<Product, double>& b) {
        if (a.second != b.second) {
            return a.second > b.second; // higher score first
        }
        if (a.first.getPurchases() != b.first.getPurchases()) {
            return a.first.getPurchases() > b.first.getPurchases();
        }
        return a.first.getViews() > b.first.getViews();
    });

    // 4. Return top N products
    if ((int)scoredProducts.size() > limit) {
        scoredProducts.resize(limit);
    }

    return scoredProducts;
}
