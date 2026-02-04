#ifndef HUNGARIAN_ALGORITHM_H
#define HUNGARIAN_ALGORITHM_H

#include <vector>
#include <limits>

class HungarianAlgorithm {
public:
    // Solve the assignment problem
    // costMatrix: 2D cost matrix (rows = tracks, cols = detections)
    // Returns: vector of assignments (track index -> detection index, -1 if unassigned)
    static std::vector<int> solve(const std::vector<std::vector<float>>& costMatrix);
    
private:
    static const float INF;
};

#endif // HUNGARIAN_ALGORITHM_H
