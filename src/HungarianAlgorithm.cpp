#include "HungarianAlgorithm.h"
#include <algorithm>

const float HungarianAlgorithm::INF = std::numeric_limits<float>::max();

std::vector<int> HungarianAlgorithm::solve(const std::vector<std::vector<float>>& costMatrix) {
    if (costMatrix.empty() || costMatrix[0].empty()) {
        return std::vector<int>();
    }
    
    int rows = costMatrix.size();
    int cols = costMatrix[0].size();
    
    // Create working copy of cost matrix
    std::vector<std::vector<float>> cost = costMatrix;
    
    // Result: assignment[i] = j means row i is assigned to column j
    std::vector<int> assignment(rows, -1);
    std::vector<bool> rowCovered(rows, false);
    std::vector<bool> colCovered(cols, false);
    
    // Step 1: Subtract row minimum from each row
    for (int i = 0; i < rows; ++i) {
        float minVal = *std::min_element(cost[i].begin(), cost[i].end());
        if (minVal < INF) {
            for (int j = 0; j < cols; ++j) {
                if (cost[i][j] < INF) {
                    cost[i][j] -= minVal;
                }
            }
        }
    }
    
    // Step 2: Subtract column minimum from each column
    for (int j = 0; j < cols; ++j) {
        float minVal = INF;
        for (int i = 0; i < rows; ++i) {
            minVal = std::min(minVal, cost[i][j]);
        }
        if (minVal < INF) {
            for (int i = 0; i < rows; ++i) {
                if (cost[i][j] < INF) {
                    cost[i][j] -= minVal;
                }
            }
        }
    }
    
    // Step 3: Greedy assignment of zeros
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (cost[i][j] == 0.0f && !rowCovered[i] && !colCovered[j]) {
                assignment[i] = j;
                rowCovered[i] = true;
                colCovered[j] = true;
                break;
            }
        }
    }
    
    // Simple greedy approach for remaining unassigned rows
    for (int i = 0; i < rows; ++i) {
        if (assignment[i] == -1) {
            float minCost = INF;
            int minCol = -1;
            for (int j = 0; j < cols; ++j) {
                if (!colCovered[j] && cost[i][j] < minCost) {
                    minCost = cost[i][j];
                    minCol = j;
                }
            }
            if (minCol != -1 && minCost < INF) {
                assignment[i] = minCol;
                colCovered[minCol] = true;
            }
        }
    }
    
    return assignment;
}
