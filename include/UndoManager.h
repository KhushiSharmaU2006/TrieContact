#pragma once
#include <string>
#include <stack>
#include <iostream>
#include "Contact.h"

using namespace std;

// What kind of operation happened
enum OperationType {
    OP_ADD,
    OP_DELETE,
    OP_UPDATE
};

// A Command snapshot — stores enough info to reverse or redo
struct Command {
    OperationType type;
    Contact oldState;   // state before the operation (used for undo)
    Contact newState;   // state after the operation (used for redo)

    Command(OperationType t, Contact oldC, Contact newC)
        : type(t), oldState(oldC), newState(newC) {}
};

class UndoManager {
private:
    stack<Command> undoStack;
    stack<Command> redoStack;

public:
    // Call this BEFORE every add/update/delete
    void recordOperation(OperationType type, Contact oldState, Contact newState) {
        undoStack.push(Command(type, oldState, newState));
        // Any new operation clears the redo history
        while (!redoStack.empty()) redoStack.pop();
    }

    // Returns the command to reverse, or empty if nothing to undo
    bool canUndo() { return !undoStack.empty(); }
    bool canRedo() { return !redoStack.empty(); }

    // Pop from undo stack, push to redo stack, return what to reverse
    Command undo() {
        Command cmd = undoStack.top();
        undoStack.pop();
        redoStack.push(cmd);
        return cmd;
    }

    // Pop from redo stack, push back to undo stack, return what to reapply
    Command redo() {
        Command cmd = redoStack.top();
        redoStack.pop();
        undoStack.push(cmd);
        return cmd;
    }

    void printStatus() {
        cout << "  Undo stack size: " << undoStack.size() << "\n"
             << "  Redo stack size: " << redoStack.size() << "\n";
    }
};
