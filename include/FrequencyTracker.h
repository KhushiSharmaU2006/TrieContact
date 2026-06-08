#pragma once
#include <string>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <iostream>
#include "Contact.h"

using namespace std;

// ─────────────────────────────────────────────
//  LRU CACHE  (Doubly Linked List + Hashmap)
//  Tracks the last N accessed contacts in order
// ─────────────────────────────────────────────
class LRUCache {
private:
    int capacity;
    list<string> dll;                        // front = most recent
    map<string, list<string>::iterator> mp;  // name → iterator in DLL

public:
    LRUCache(int cap = 10) : capacity(cap) {}

    // Call this every time a contact is accessed
    void access(const string& name) {
        if (mp.count(name)) {
            dll.erase(mp[name]);   // remove from current position
        } else if ((int)dll.size() >= capacity) {
            string evicted = dll.back();
            dll.pop_back();
            mp.erase(evicted);
        }
        dll.push_front(name);
        mp[name] = dll.begin();
    }

    // Remove a contact (e.g. on delete)
    void remove(const string& name) {
        if (mp.count(name)) {
            dll.erase(mp[name]);
            mp.erase(name);
        }
    }

    // Return recently accessed contacts in order (most recent first)
    vector<string> getRecent() {
        return vector<string>(dll.begin(), dll.end());
    }

    void display() {
        cout << "\n===== Recently Accessed (Last " << capacity << ") =====\n";
        int i = 1;
        for (const string& name : dll) {
            cout << "  " << i++ << ". " << name << "\n";
        }
    }
};

// ─────────────────────────────────────────────
//  MIN-HEAP for Top-K Most Contacted
//  Uses a min-heap of size K so we keep only
//  the K highest frequency contacts efficiently
// ─────────────────────────────────────────────
class FrequencyHeap {
public:
    // Extract top K contacts by frequency from a full contact list
    // Input: vector of Contact pointers
    vector<Contact*> topK(vector<Contact*>& allContacts, int k) {
        // min-heap: (frequency, Contact*)
        // Using min-heap of size K:
        //   if heap size < K → push
        //   if new frequency > heap top → pop top, push new
        // Result: heap contains K highest frequency contacts
        auto cmp = [](Contact* a, Contact* b){ return a->frequency > b->frequency; };
        priority_queue<Contact*, vector<Contact*>, decltype(cmp)> minHeap(cmp);

        for (Contact* c : allContacts) {
            if ((int)minHeap.size() < k) {
                minHeap.push(c);
            } else if (c->frequency > minHeap.top()->frequency) {
                minHeap.pop();
                minHeap.push(c);
            }
        }

        // Extract into vector (will be in ascending order, reverse for display)
        vector<Contact*> result;
        while (!minHeap.empty()) {
            result.push_back(minHeap.top());
            minHeap.pop();
        }
        reverse(result.begin(), result.end());  // highest first
        return result;
    }

    void display(vector<Contact*>& topContacts) {
        cout << "\n===== Top " << topContacts.size() << " Most Contacted =====\n";
        int rank = 1;
        for (Contact* c : topContacts) {
            cout << "  " << rank++ << ". " << c->name
                 << "  |  " << c->phone
                 << "  |  accessed " << c->frequency << " time(s)\n";
        }
    }
};
