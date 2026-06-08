#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

// One node in the Trie
// children: map from char → next node (supports all chars, not just a-z)
struct TrieNode {
    map<char, TrieNode*> children;
    bool isEnd;         // true = a complete name/phone ends here
    string fullKey;     // stores the complete string at leaf (for easy retrieval)

    TrieNode() : isEnd(false) {}
};

class Trie {
private:
    TrieNode* root;

    // DFS helper: from current node, collect all complete keys into result
    void dfs(TrieNode* node, string current, vector<string>& result) {
        if (node->isEnd) {
            result.push_back(node->fullKey);
        }
        for (auto& pair : node->children) {
            dfs(pair.second, current + pair.first, result);
        }
    }

    // Recursive helper for deletion — returns true if node can be deleted
    bool deleteHelper(TrieNode* node, const string& key, int depth) {
        if (!node) return false;

        if (depth == (int)key.size()) {
            if (node->isEnd) {
                node->isEnd = false;
                node->fullKey = "";
            }
            // Delete node if it has no children
            return node->children.empty();
        }

        char ch = key[depth];
        if (node->children.find(ch) == node->children.end()) return false;

        bool shouldDelete = deleteHelper(node->children[ch], key, depth + 1);

        if (shouldDelete) {
            delete node->children[ch];
            node->children.erase(ch);
            return node->children.empty() && !node->isEnd;
        }
        return false;
    }

public:
    Trie() {
        root = new TrieNode();
    }

    // Insert a key (lowercased name or phone number)
    void insert(const string& key) {
        TrieNode* curr = root;
        string lowerKey = key;
        for (char& c : lowerKey) c = tolower(c);

        for (char ch : lowerKey) {
            if (curr->children.find(ch) == curr->children.end()) {
                curr->children[ch] = new TrieNode();
            }
            curr = curr->children[ch];
        }
        curr->isEnd = true;
        curr->fullKey = lowerKey;
    }

    // Delete a key from the Trie
    void remove(const string& key) {
        string lowerKey = key;
        for (char& c : lowerKey) c = tolower(c);
        deleteHelper(root, lowerKey, 0);
    }

    // Search prefix → return all matching complete keys via DFS
    vector<string> searchPrefix(const string& prefix) {
        TrieNode* curr = root;
        string lowerPrefix = prefix;
        for (char& c : lowerPrefix) c = tolower(c);

        for (char ch : lowerPrefix) {
            if (curr->children.find(ch) == curr->children.end()) {
                return {};   // prefix not found at all
            }
            curr = curr->children[ch];
        }

        // DFS from this node to collect all complete keys
        vector<string> result;
        dfs(curr, lowerPrefix, result);
        return result;
    }

    // Check if exact key exists
    bool exactSearch(const string& key) {
        TrieNode* curr = root;
        string lowerKey = key;
        for (char& c : lowerKey) c = tolower(c);

        for (char ch : lowerKey) {
            if (curr->children.find(ch) == curr->children.end()) return false;
            curr = curr->children[ch];
        }
        return curr->isEnd;
    }

    // List ALL contacts (DFS from root)
    vector<string> listAll() {
        vector<string> result;
        dfs(root, "", result);
        return result;
    }
};
