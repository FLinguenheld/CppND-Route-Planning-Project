#include "route_planner.h"
#include "route_model.h"
#include <algorithm>

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    this->start_node = &model.FindClosestNode(start_x, start_y);
    this->end_node = &model.FindClosestNode(end_x, end_y);
}

float RoutePlanner::CalculateHValue(RouteModel::Node const *node) {
    return node->distance(*this->end_node);
}

// Expand the current node by adding all unvisited neighbors to the open list
void RoutePlanner::AddNeighbors(RouteModel::Node *current_node) {

    current_node->FindNeighbors();
    for (RouteModel::Node *n : current_node->neighbors) {

        n->parent = current_node;
        n->h_value = this->CalculateHValue(n);
        n->g_value = current_node->g_value + current_node->distance(*n);

        n->visited = true;
        this->open_list.emplace_back(n);
    }
}

// Used to sort open_list
bool CompareSumHG(const RouteModel::Node *n1, const RouteModel::Node *n2)
{
    return (n1->h_value + n1->g_value) > (n2->h_value + n2->g_value);
}

// Sort open_list, return the node with the lowest h+g value
RouteModel::Node *RoutePlanner::NextNode() {

    std::sort(this->open_list.begin(), this->open_list.end(), CompareSumHG);

    RouteModel::Node *node = this->open_list.back();
    this->open_list.pop_back();
    return node;
}

// Follow the parent's nodes up to the start node to build the path
std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node) {

    // Create path_found vector
    distance = 0.0f;
    std::vector<RouteModel::Node> path_found;

    // while (current_node->x != start_node->x && current_node->y != start_node->y)
    while (current_node != this->start_node) // Comparison of arguments or ptr address ?
    {
        distance += current_node->distance(*current_node->parent);
        path_found.insert(path_found.begin(), *current_node);
        current_node = current_node->parent;
    }

    path_found.insert(path_found.begin(), *current_node); // Add start node

    distance *= m_Model.MetricScale(); // Multiply the distance by the scale of the map to get meters.
    return path_found;
}

// A* Search algorithm
void RoutePlanner::AStarSearch() {

    RouteModel::Node *current_node = this->start_node;
    current_node->visited = true;
    this->AddNeighbors(current_node);

    while (this->open_list.size() > 0)
    {
        current_node = this->NextNode();

        // if (current_node->x == this->end_node->x && current_node->y == this->end_node->y)
        if (current_node == this->end_node)
        {
            this->m_Model.path = this->ConstructFinalPath(current_node);
            return;
        }

        this->AddNeighbors(current_node);
    }
}
