#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <iostream>
#include <climits>

using namespace std;

// Undirected weighted graph
// Each node = contact name
// Each edge = relationship with a strength weight
class RelationshipGraph {
private:
    // adjacency list: name → list of (connected name, weight)
    map<string, vector<pair<string, int>>> adjList;

public:
    // Add a contact node (no edges yet)
    void addNode(const string& name) {
        if (adjList.find(name) == adjList.end()) {
            adjList[name] = {};
        }
    }

    // Remove a node and all its edges
    void removeNode(const string& name) {
        adjList.erase(name);
        // Remove all edges pointing to this node
        for (auto& pair : adjList) {
            auto& edges = pair.second;
            edges.erase(
                remove_if(edges.begin(), edges.end(),
                    [&](const ::pair<string,int>& e){ return e.first == name; }),
                edges.end()
            );
        }
    }

    // Add undirected edge between two contacts
    void addRelationship(const string& a, const string& b, int weight = 1) {
        addNode(a);
        addNode(b);
        adjList[a].push_back({b, weight});
        adjList[b].push_back({a, weight});
    }

    // BFS: find all contacts reachable from a given contact (direct + indirect)
    vector<pair<string,int>> getRelated(const string& name) {
        vector<pair<string,int>> result;
        if (adjList.find(name) == adjList.end()) return result;

        set<string> visited;
        queue<string> q;
        q.push(name);
        visited.insert(name);

        while (!q.empty()) {
            string curr = q.front(); q.pop();
            for (auto& edge : adjList[curr]) {
                if (!visited.count(edge.first)) {
                    visited.insert(edge.first);
                    result.push_back(edge);
                    q.push(edge.first);
                }
            }
        }
        return result;
    }

    // Direct connections only (1-hop neighbours)
    vector<pair<string,int>> getDirectConnections(const string& name) {
        if (adjList.find(name) == adjList.end()) return {};
        return adjList[name];
    }

    // Dijkstra: shortest relationship path between two contacts
    // Returns {distance, path}
    pair<int, vector<string>> shortestPath(const string& src, const string& dst) {
        map<string, int> dist;
        map<string, string> prev;
        // min-heap: (distance, node)
        priority_queue<pair<int,string>, vector<pair<int,string>>, greater<>> pq;

        for (auto& node : adjList) dist[node.first] = INT_MAX;
        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            for (auto& [v, w] : adjList[u]) {
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        // Reconstruct path
        vector<string> path;
        if (dist[dst] == INT_MAX) return {-1, {}};
        for (string at = dst; at != src; at = prev[at]) path.push_back(at);
        path.push_back(src);
        reverse(path.begin(), path.end());
        return {dist[dst], path};
    }

    // Friends of friends (2-hop suggestion)
    vector<string> suggestContacts(const string& name) {
        set<string> direct, suggestions;
        if (!adjList.count(name)) return {};

        for (auto& [neighbor, w] : adjList[name]) direct.insert(neighbor);

        for (auto& neighbor : direct) {
            for (auto& [fof, w] : adjList[neighbor]) {
                if (fof != name && !direct.count(fof))
                    suggestions.insert(fof);
            }
        }
        return vector<string>(suggestions.begin(), suggestions.end());
    }

    // Most connected contact (highest degree)
    string mostConnected() {
        string best = "";
        int maxDeg = 0;
        for (auto& [node, edges] : adjList) {
            if ((int)edges.size() > maxDeg) {
                maxDeg = edges.size();
                best = node;
            }
        }
        return best;
    }

    bool hasNode(const string& name) {
        return adjList.count(name) > 0;
    }
};
