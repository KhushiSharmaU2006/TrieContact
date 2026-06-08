#include <iostream>
#include <string>
#include <limits>
#include "include/ContactManager.h"

using namespace std;
void clearInput() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void printBanner() {
    cout << "\n";C
    cout << "        TrieContact | Contact Manager    \n";
    cout << "         DSA Edition  |  C++ CLI         \n";
    
    cout << "\n";
}
void printMenu() {
    cout << "                 MAIN MENU               \n";
    cout << "  CONTACTS\n";
    cout << "   1. Add Contact\n";
    cout << "   2. Delete Contact\n";
    cout << "   3. Update Contact\n";
    cout << "   4. List All Contacts\n";
    cout << "\n  SEARCH\n";
    cout << "   5. Search by Name Prefix   (Trie)\n";
    cout << "   6. Search by Phone Prefix  (Trie)\n";
    cout << "   7. Exact Search by Name    (Hashmap)\n";
    cout << "\n  RELATIONSHIPS\n";
    cout << "   8. Show Related Contacts\n";
    cout << "   9. Add Relationship\n";
    cout << "  10. Shortest Path Between Contacts\n";
    cout << "  11. Suggest Contacts\n";
    cout << "  12. Most Connected Contact\n";
    cout << "\n  GROUPS\n";
    cout << "  13. Show Group Tree\n";
    cout << "  14. Show Contacts in Group\n";
    cout << "  15. Move Contact to Another Group\n";
    cout << "\n  FREQUENCY & RECENT\n";
    cout << "  16. Top K Most Contacted    (Heap)\n";
    cout << "  17. Recently Accessed       (LRU Cache)\n";
    cout << "\n  HISTORY\n";
    cout << "  18. Undo Last Operation     (Stack)\n";
    cout << "  19. Redo Last Operation     (Stack)\n";
    cout << "\n   0. Save & Exit\n";
    cout << "-----------------------------------------\n";
    cout << "  Enter choice: ";
}
// ── Main ─────────────────────────────────────────────────────────────────────

int main() {
    ContactManager manager;

    printBanner();
    cout << "Loading data...\n";

    // Load all CSV files at startup
    manager.loadContacts("data/contacts.csv");
    manager.loadRelationships("data/relationships.csv");
    // Groups are built automatically from the group column in contacts.csv
    // and relationships.csv; group tree is populated during loadContacts

    int choice;

    while (true) {
        printMenu();
        cin >> choice;
        clearInput();

        cout << "\n";

        if (choice == 0) {
            manager.saveContacts("data/contacts.csv");
            cout << "Goodbye!\n";
            break;

        } else if (choice == 1) {
            string name, phone, email, group;
            cout << "  Name  : "; getline(cin, name);
            cout << "  Phone : "; getline(cin, phone);
            cout << "  Email : "; getline(cin, email);
            cout << "  Group : "; getline(cin, group);
            manager.addContact(name, phone, email, group);

        } else if (choice == 2) {
            string name;
            cout << "  Enter name to delete: "; getline(cin, name);
            manager.deleteContact(name);

        } else if (choice == 3) {
            string name, phone, email, group;
            cout << "  Name to update  : "; getline(cin, name);
            cout << "  New Phone       : "; getline(cin, phone);
            cout << "  New Email       : "; getline(cin, email);
            cout << "  New Group       : "; getline(cin, group);
            manager.updateContact(name, phone, email, group);

        } else if (choice == 4) {
            manager.listAll();

        } else if (choice == 5) {
            string prefix;
            cout << "  Enter name prefix: "; getline(cin, prefix);
            manager.searchByNamePrefix(prefix);

        } else if (choice == 6) {
            string prefix;
            cout << "  Enter phone prefix: "; getline(cin, prefix);
            manager.searchByPhonePrefix(prefix);

        } else if (choice == 7) {
            string name;
            cout << "  Enter exact name: "; getline(cin, name);
            manager.searchExact(name);

        } else if (choice == 8) {
            string name;
            cout << "  Enter contact name: "; getline(cin, name);
            manager.showRelatedContacts(name);

        } else if (choice == 9) {
            string a, b; int w;
            cout << "  Contact A   : "; getline(cin, a);
            cout << "  Contact B   : "; getline(cin, b);
            cout << "  Strength(1-10): "; cin >> w; clearInput();
            manager.addRelationship(a, b, w);

        } else if (choice == 10) {
            string src, dst;
            cout << "  Source contact : "; getline(cin, src);
            cout << "  Target contact : "; getline(cin, dst);
            manager.shortestPath(src, dst);

        } else if (choice == 11) {
            string name;
            cout << "  Enter contact name: "; getline(cin, name);
            manager.suggestContacts(name);

        } else if (choice == 12) {
            manager.mostConnected();

        } else if (choice == 13) {
            manager.showGroupTree();

        } else if (choice == 14) {
            string group;
            cout << "  Enter group name: "; getline(cin, group);
            manager.showGroup(group);

        } else if (choice == 15) {
            string name, group;
            cout << "  Contact name  : "; getline(cin, name);
            cout << "  New group     : "; getline(cin, group);
            manager.moveContactGroup(name, group);

        } else if (choice == 16) {
            int k;
            cout << "  How many top contacts? "; cin >> k; clearInput();
            manager.showTopK(k);

        } else if (choice == 17) {
            manager.showRecent();

        } else if (choice == 18) {
            manager.undo();

        } else if (choice == 19) {
            manager.redo();

        } else {
            cout << "  Invalid choice. Try again.\n";
        }
    }

    return 0;
}
