#ifndef RECOMMENDATION_ENGINE_H
#define RECOMMENDATION_ENGINE_H

#include <vector>
#include <string>
#include "Product.h"
#include "Database.h"

using namespace std;

class RecommendationEngine {
public:
    static vector<pair<Product, double>> getRecommendations(
        const string& username,
        const Database& db,
        int limit = 5
    );
};

#endif // RECOMMENDATION_ENGINE_H