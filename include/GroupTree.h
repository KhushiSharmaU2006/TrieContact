#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

// One node in the group tree
// Can be a group (like "Work") or a leaf contact name
struct TreeNode {
    string name;
    string parent;          // empty string means root
    bool isContact;         // false = group node, true = contact leaf
    vector<TreeNode*> children;

    TreeNode(string n, string p, bool contact = false)
        : name(n), parent(p), isContact(contact) {}
};

class GroupTree {
private:
    // All nodes stored in a flat map for O(1) access
    map<string, TreeNode*> nodeMap;

    // DFS helper to collect all contact names under a subtree
    void dfsCollect(TreeNode* node, vector<string>& result) {
        if (node->isContact) {
            result.push_back(node->name);
        }
        for (TreeNode* child : node->children) {
            dfsCollect(child, result);
        }
    }

    // DFS helper to print tree with indentation
    void dfsPrint(TreeNode* node, int depth) {
        string indent(depth * 2, ' ');
        string marker = node->isContact ? "[contact] " : "[group]   ";
        cout << indent << marker << node->name << "\n";
        for (TreeNode* child : node->children) {
            dfsPrint(child, depth + 1);
        }
    }

public:
    // Add a group node (parent = "" means top-level root group)
    void addGroup(const string& groupName, const string& parentName = "") {
        if (nodeMap.count(groupName)) return;
        TreeNode* node = new TreeNode(groupName, parentName, false);
        nodeMap[groupName] = node;

        if (!parentName.empty() && nodeMap.count(parentName)) {
            nodeMap[parentName]->children.push_back(node);
        }
    }

    // Assign a contact to a group (contact becomes leaf node)
    void addContactToGroup(const string& contactName, const string& groupName) {
        if (nodeMap.count(contactName)) return;  // already placed
        if (!nodeMap.count(groupName)) addGroup(groupName);

        TreeNode* node = new TreeNode(contactName, groupName, true);
        nodeMap[contactName] = node;
        nodeMap[groupName]->children.push_back(node);
    }

    // Remove a contact from its group
    void removeContact(const string& contactName) {
        if (!nodeMap.count(contactName)) return;
        TreeNode* node = nodeMap[contactName];
        // Remove from parent's children list
        if (nodeMap.count(node->parent)) {
            auto& siblings = nodeMap[node->parent]->children;
            siblings.erase(
                remove_if(siblings.begin(), siblings.end(),
                    [&](TreeNode* n){ return n->name == contactName; }),
                siblings.end()
            );
        }
        delete node;
        nodeMap.erase(contactName);
    }

    // Move a contact to a different group
    void moveContact(const string& contactName, const string& newGroup) {
        removeContact(contactName);
        addContactToGroup(contactName, newGroup);
    }

    // Get all contacts under a group (recursively, via DFS)
    vector<string> getContactsInGroup(const string& groupName) {
        vector<string> result;
        if (!nodeMap.count(groupName)) return result;
        dfsCollect(nodeMap[groupName], result);
        return result;
    }

    // Print the entire tree from all top-level groups
    void printTree() {
        cout << "\n===== Group Hierarchy =====\n";
        for (auto& [name, node] : nodeMap) {
            if (node->parent.empty()) {   // root level groups only
                dfsPrint(node, 0);
            }
        }
    }

    // Get group of a contact
    string getGroup(const string& contactName) {
        if (nodeMap.count(contactName)) return nodeMap[contactName]->parent;
        return "Ungrouped";
    }

    bool exists(const string& name) {
        return nodeMap.count(name) > 0;
    }
};
