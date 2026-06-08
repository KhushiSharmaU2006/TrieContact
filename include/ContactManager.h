#pragma once
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "Contact.h"
#include "Trie.h"
#include "Graph.h"
#include "GroupTree.h"
#include "FrequencyTracker.h"
#include "UndoManager.h"

using namespace std;

class ContactManager {
private:
    // ── The 4 core lookup structures ──────────────────
    Trie nameTrie;
    Trie phoneTrie;
    map<string, Contact*> nameMap;   // lowercase name → Contact*
    map<string, Contact*> phoneMap;  // phone string  → Contact*

    // ── Advanced structures ───────────────────────────
    RelationshipGraph graph;
    GroupTree groupTree;
    LRUCache lruCache;
    FrequencyHeap freqHeap;
    UndoManager undoMgr;

    // Helper: normalize to lowercase
    string lower(string s) {
        for (char& c : s) c = tolower(c);
        return s;
    }

    // Internal raw insert — used by loadFromFile and addContact
    void rawInsert(Contact* c) {
        string lname = lower(c->name);
        nameTrie.insert(lname);
        phoneTrie.insert(c->phone);
        nameMap[lname] = c;
        phoneMap[c->phone] = c;
        graph.addNode(c->name);
        groupTree.addContactToGroup(c->name, c->group);
    }

    // Internal raw remove — used by deleteContact and update
    void rawRemove(Contact* c) {
        string lname = lower(c->name);
        nameTrie.remove(lname);
        phoneTrie.remove(c->phone);
        nameMap.erase(lname);
        phoneMap.erase(c->phone);
        graph.removeNode(c->name);
        groupTree.removeContact(c->name);
        lruCache.remove(c->name);
    }

public:
    ContactManager() : lruCache(10) {}

    // ─────────────────────────────────────────────────
    //  FILE I/O
    // ─────────────────────────────────────────────────

    // Load contacts from CSV: name,phone,email,group
    void loadContacts(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "[WARNING] Could not open " << filename << "\n";
            return;
        }
        string line;
        getline(file, line); // skip header row
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string name, phone, email, group;
            getline(ss, name,  ',');
            getline(ss, phone, ',');
            getline(ss, email, ',');
            getline(ss, group, ',');
            // Trim whitespace
            auto trim = [](string& s){ 
                size_t start = s.find_first_not_of(" \t\r");
                size_t end = s.find_last_not_of(" \t\r");
                s = (start == string::npos) ? "" : s.substr(start, end-start+1);
            };
            trim(name); trim(phone); trim(email); trim(group);
            if (!name.empty() && !phone.empty()) {
                rawInsert(new Contact(name, phone, email, group));
            }
        }
        cout << "[INFO] Contacts loaded from " << filename << "\n";
    }

    // Load relationships from CSV: nameA,nameB,weight
    void loadRelationships(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) { cout << "[WARNING] Could not open " << filename << "\n"; return; }
        string line;
        getline(file, line); // skip header
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string a, b, wStr;
            getline(ss, a, ',');
            getline(ss, b, ',');
            getline(ss, wStr, ',');
            auto trim = [](string& s){
                size_t start = s.find_first_not_of(" \t\r");
                size_t end = s.find_last_not_of(" \t\r");
                s = (start == string::npos) ? "" : s.substr(start, end-start+1);
            };
            trim(a); trim(b); trim(wStr);
            int w = wStr.empty() ? 1 : stoi(wStr);
            if (!a.empty() && !b.empty()) graph.addRelationship(a, b, w);
        }
        cout << "[INFO] Relationships loaded from " << filename << "\n";
    }

    // Save all contacts back to CSV
    void saveContacts(const string& filename) {
        ofstream file(filename);
        file << "name,phone,email,group\n";
        for (auto& [key, c] : nameMap) {
            file << c->name  << ","
                 << c->phone << ","
                 << c->email << ","
                 << c->group << "\n";
        }
        cout << "[INFO] Contacts saved to " << filename << "\n";
    }

    // ─────────────────────────────────────────────────
    //  CRUD OPERATIONS
    // ─────────────────────────────────────────────────

    void addContact(const string& name, const string& phone,
                    const string& email, const string& group) {
        if (nameMap.count(lower(name))) {
            cout << "[ERROR] Contact '" << name << "' already exists.\n";
            return;
        }
        Contact* c = new Contact(name, phone, email, group);
        undoMgr.recordOperation(OP_ADD, Contact(), *c);
        rawInsert(c);
        cout << "[OK] Contact '" << name << "' added.\n";
    }

    void deleteContact(const string& name) {
        string lname = lower(name);
        if (!nameMap.count(lname)) {
            cout << "[ERROR] Contact '" << name << "' not found.\n";
            return;
        }
        Contact* c = nameMap[lname];
        undoMgr.recordOperation(OP_DELETE, *c, Contact());
        rawRemove(c);
        delete c;
        cout << "[OK] Contact '" << name << "' deleted.\n";
    }

    // Update: delete old entry, reinsert with new values
    void updateContact(const string& name, const string& newPhone,
                       const string& newEmail, const string& newGroup) {
        string lname = lower(name);
        if (!nameMap.count(lname)) {
            cout << "[ERROR] Contact '" << name << "' not found.\n";
            return;
        }
        Contact* old = nameMap[lname];
        Contact oldSnapshot = *old;  // save for undo

        // Remove old entries from all structures
        rawRemove(old);

        // Update the object and reinsert
        old->phone = newPhone;
        old->email = newEmail;
        old->group = newGroup;
        rawInsert(old);

        undoMgr.recordOperation(OP_UPDATE, oldSnapshot, *old);
        cout << "[OK] Contact '" << name << "' updated.\n";
    }

    // ─────────────────────────────────────────────────
    //  SEARCH
    // ─────────────────────────────────────────────────

    // Prefix search by name → Trie DFS → hashmap for full details
    void searchByNamePrefix(const string& prefix) {
        vector<string> keys = nameTrie.searchPrefix(prefix);
        if (keys.empty()) { cout << "  No contacts found for prefix '" << prefix << "'\n"; return; }
        cout << "\n  Found " << keys.size() << " contact(s):\n";
        cout << string(40, '-') << "\n";
        for (const string& key : keys) {
            if (nameMap.count(key)) {
                nameMap[key]->display();
                cout << string(40, '-') << "\n";
            }
        }
    }

    // Prefix search by phone
    void searchByPhonePrefix(const string& prefix) {
        vector<string> keys = phoneTrie.searchPrefix(prefix);
        if (keys.empty()) { cout << "  No contacts found for phone prefix '" << prefix << "'\n"; return; }
        cout << "\n  Found " << keys.size() << " contact(s):\n";
        cout << string(40, '-') << "\n";
        for (const string& key : keys) {
            if (phoneMap.count(key)) {
                phoneMap[key]->display();
                cout << string(40, '-') << "\n";
            }
        }
    }

    // Exact search by name → O(1) hashmap + update LRU + frequency
    void searchExact(const string& name) {
        string lname = lower(name);
        if (!nameMap.count(lname)) {
            cout << "  Contact '" << name << "' not found.\n";
            return;
        }
        Contact* c = nameMap[lname];
        c->frequency++;
        lruCache.access(c->name);
        cout << "\n"; c->display();
    }

    // List all contacts
    void listAll() {
        vector<string> keys = nameTrie.listAll();
        if (keys.empty()) { cout << "  No contacts stored.\n"; return; }
        cout << "\n  Total contacts: " << keys.size() << "\n";
        cout << string(40, '=') << "\n";
        for (const string& key : keys) {
            if (nameMap.count(key)) {
                nameMap[key]->display();
                cout << string(40, '-') << "\n";
            }
        }
    }

    // ─────────────────────────────────────────────────
    //  GRAPH FEATURES
    // ─────────────────────────────────────────────────

    void showRelatedContacts(const string& name) {
        auto related = graph.getDirectConnections(name);
        if (related.empty()) { cout << "  No direct relationships for '" << name << "'\n"; return; }
        cout << "\n  Direct connections of '" << name << "':\n";
        for (auto& [contact, weight] : related)
            cout << "    → " << contact << "  (strength: " << weight << ")\n";
    }

    void addRelationship(const string& a, const string& b, int weight) {
        graph.addRelationship(a, b, weight);
        cout << "[OK] Relationship added: " << a << " ↔ " << b << " (weight " << weight << ")\n";
    }

    void shortestPath(const string& src, const string& dst) {
        auto [dist, path] = graph.shortestPath(src, dst);
        if (dist == -1) { cout << "  No path found between '" << src << "' and '" << dst << "'\n"; return; }
        cout << "\n  Shortest path (" << src << " → " << dst << "):\n  ";
        for (int i = 0; i < (int)path.size(); i++) {
            cout << path[i];
            if (i+1 < (int)path.size()) cout << " → ";
        }
        cout << "\n  Total distance: " << dist << "\n";
    }

    void suggestContacts(const string& name) {
        auto suggestions = graph.suggestContacts(name);
        if (suggestions.empty()) { cout << "  No suggestions for '" << name << "'\n"; return; }
        cout << "\n  People '" << name << "' might know:\n";
        for (auto& s : suggestions) cout << "    • " << s << "\n";
    }

    void mostConnected() {
        string name = graph.mostConnected();
        if (name.empty()) { cout << "  Graph is empty.\n"; return; }
        cout << "\n  Most connected contact: " << name << "\n";
    }

    // ─────────────────────────────────────────────────
    //  GROUP TREE
    // ─────────────────────────────────────────────────

    void showGroup(const string& groupName) {
        auto contacts = groupTree.getContactsInGroup(groupName);
        if (contacts.empty()) { cout << "  No contacts in group '" << groupName << "'\n"; return; }
        cout << "\n  Contacts in '" << groupName << "':\n";
        for (auto& name : contacts) cout << "    • " << name << "\n";
    }

    void showGroupTree() { groupTree.printTree(); }

    void moveContactGroup(const string& contactName, const string& newGroup) {
        groupTree.moveContact(contactName, newGroup);
        // Also update the Contact object's group field
        string lname = lower(contactName);
        if (nameMap.count(lname)) nameMap[lname]->group = newGroup;
        cout << "[OK] '" << contactName << "' moved to group '" << newGroup << "'\n";
    }

    // ─────────────────────────────────────────────────
    //  FREQUENCY & LRU
    // ─────────────────────────────────────────────────

    void showTopK(int k) {
        vector<Contact*> all;
        for (auto& [key, c] : nameMap) all.push_back(c);
        auto top = freqHeap.topK(all, k);
        if (top.empty()) { cout << "  No frequency data yet. Search some contacts first.\n"; return; }
        freqHeap.display(top);
    }

    void showRecent() { lruCache.display(); }

    // ─────────────────────────────────────────────────
    //  UNDO / REDO
    // ─────────────────────────────────────────────────

    void undo() {
        if (!undoMgr.canUndo()) { cout << "  Nothing to undo.\n"; return; }
        Command cmd = undoMgr.undo();
        if (cmd.type == OP_ADD) {
            // Undo an add = delete the contact
            rawRemove(nameMap[lower(cmd.newState.name)]);
            cout << "[UNDO] Removed '" << cmd.newState.name << "'\n";
        } else if (cmd.type == OP_DELETE) {
            // Undo a delete = re-add the contact
            rawInsert(new Contact(cmd.oldState));
            cout << "[UNDO] Restored '" << cmd.oldState.name << "'\n";
        } else if (cmd.type == OP_UPDATE) {
            // Undo an update = revert to old state
            rawRemove(nameMap[lower(cmd.newState.name)]);
            rawInsert(new Contact(cmd.oldState));
            cout << "[UNDO] Reverted '" << cmd.oldState.name << "' to previous state\n";
        }
    }

    void redo() {
        if (!undoMgr.canRedo()) { cout << "  Nothing to redo.\n"; return; }
        Command cmd = undoMgr.redo();
        if (cmd.type == OP_ADD) {
            rawInsert(new Contact(cmd.newState));
            cout << "[REDO] Re-added '" << cmd.newState.name << "'\n";
        } else if (cmd.type == OP_DELETE) {
            rawRemove(nameMap[lower(cmd.oldState.name)]);
            cout << "[REDO] Re-deleted '" << cmd.oldState.name << "'\n";
        } else if (cmd.type == OP_UPDATE) {
            rawRemove(nameMap[lower(cmd.oldState.name)]);
            rawInsert(new Contact(cmd.newState));
            cout << "[REDO] Re-applied update to '" << cmd.newState.name << "'\n";
        }
    }
};
