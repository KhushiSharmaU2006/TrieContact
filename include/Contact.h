#pragma once
#include <string>

using namespace std;

// Core contact data object
// Every structure (Trie, Hashmap, Graph, Heap) points to this
struct Contact {
    string name;
    string phone;
    string email;
    string group;       // e.g. "Work", "Family"
    int frequency;      // how many times this contact was searched/accessed

    Contact() : frequency(0) {}

    Contact(string n, string p, string e, string g)
        : name(n), phone(p), email(e), group(g), frequency(0) {}

    void display() const {
        cout << "  Name   : " << name   << "\n"
             << "  Phone  : " << phone  << "\n"
             << "  Email  : " << email  << "\n"
             << "  Group  : " << group  << "\n"
             << "  Freq   : " << frequency << "\n";
    }
};
