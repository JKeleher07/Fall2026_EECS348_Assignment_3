/*
program name: Email organizer
description:
Priortize a CEOs emails based of importance of sender and date sent
inputs: file provided at either runtime or as a prompt from the program
output: Text in the console
collaborators and sources: Claude for base code
author: Jaxon Keleher
Created: 10/1/26
Revised: 10/1/26
*/

#include <iostream>
#include <fstream> 
#include <string>
#include <cstdio>

using namespace std;

// ---------------------------------------------------------------- Email ----

// Structure used to store all information about a single email.
// Additional fields (rank, dateKey, seq) are used for prioritization.
struct Email {
    string sender;   // Sender category (Boss, Peer, etc.)
    string subject;  // Email subject line
    string date;     // Original date from input file (MM-DD-YYYY)

    // Lower rank values indicate greater importance.
    // Boss = 0 is highest priority.
    int rank;

    // Date converted into YYYYMMDD format.
    // This makes dates easy to compare numerically.
    int dateKey;

    // Keeps track of insertion order.
    // Used to break ties when both rank and date are identical.
    long seq;

    // Default constructor initializes values to safe defaults.
    Email() : rank(0), dateKey(0), seq(0) {}
};

// Determines which email should appear first in the priority queue.
// Returns true if email a has higher priority than email b.
bool higherPriority(const Email& a, const Email& b) {

    // First priority: sender importance.
    // Lower rank number means higher priority.
    if (a.rank != b.rank)
        return a.rank < b.rank;

    // Second priority: most recent email.
    if (a.dateKey != b.dateKey)
        return a.dateKey > b.dateKey;

    // Final tiebreaker: earlier arrival in the inbox.
    return a.seq < b.seq;
}

// -------------------------------------------------------------- MaxHeap ----

// Custom max heap implementation used as the email priority queue.
// The highest-priority email is always stored at index 0.
class MaxHeap {
private:

    // Dynamic array that stores heap elements.
    Email* data;

    // Current number of emails stored.
    int count;

    // Maximum number of emails the array can hold before resizing.
    int capacity;

    // Heap index calculations.
    // These helper functions determine relationships between nodes.
    static int parent(int i) { return (i - 1) / 2; }
    static int left(int i)   { return 2 * i + 1; }
    static int right(int i)  { return 2 * i + 2; }

    // Doubles the size of the underlying array when it becomes full.
    void grow() {
        int newCap = capacity * 2;

        // Allocate larger array.
        Email* bigger = new Email[newCap];

        // Copy existing elements.
        for (int i = 0; i < count; i++)
            bigger[i] = data[i];

        // Replace old array.
        delete[] data;
        data = bigger;
        capacity = newCap;
    }

    // Exchanges the positions of two heap nodes.
    void swapAt(int i, int j) {
        Email tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }

    // Restores heap order after insertion.
    // Continues moving an email upward until its parent
    // has equal or greater priority.
    void siftUp(int i) {
        while (i > 0 && higherPriority(data[i], data[parent(i)])) {
            swapAt(i, parent(i));
            i = parent(i);
        }
    }

    // Restores heap order after removing the root.
    // Moves the replacement element down the tree until
    // the heap property is restored.
    void siftDown(int i) {
        while (true) {

            // Assume current node is best.
            int best = i;

            int l = left(i);
            int r = right(i);

            // Check left child.
            if (l < count && higherPriority(data[l], data[best]))
                best = l;

            // Check right child.
            if (r < count && higherPriority(data[r], data[best]))
                best = r;

            // If no child is better, heap is fixed.
            if (best == i)
                break;

            // Move higher-priority child upward.
            swapAt(i, best);
            i = best;
        }
    }

    // Disable copying to prevent issues with dynamic memory.
    MaxHeap(const MaxHeap&);
    MaxHeap& operator=(const MaxHeap&);

public:

    // Creates an empty heap with an initial capacity of 16.
    MaxHeap() : count(0), capacity(16) {
        data = new Email[capacity];
    }

    // Frees dynamic memory when heap is destroyed.
    ~MaxHeap() {
        delete[] data;
    }

    // Returns true if heap contains no emails.
    bool isEmpty() const {
        return count == 0;
    }

    // Returns the current number of emails stored.
    int size() const {
        return count;
    }

    // Adds a new email to the heap.
    void insert(const Email& e) {

        // Expand storage if necessary.
        if (count == capacity)
            grow();

        // Place email at end of heap.
        data[count] = e;

        // Restore heap ordering.
        siftUp(count);

        count++;
    }

    // Returns the highest-priority email without removing it.
    // Caller must verify heap is not empty first.
    const Email& peek() const {
        return data[0];
    }

    // Removes and returns the highest-priority email.
    Email extractMax() {

        // Save email currently at root.
        Email top = data[0];

        count--;

        if (count > 0) {

            // Move final element to root.
            data[0] = data[count];

            // Restore heap ordering.
            siftDown(0);
        }

        return top;
    }
};

// -------------------------------------------------------------- Helpers ----

// Removes leading and trailing whitespace from a string.
string trim(const string& s) {
    size_t start = 0, end = s.size();

    while (start < end &&
           (s[start] == ' ' || s[start] == '\t' ||
            s[start] == '\r' || s[start] == '\n'))
        start++;

    while (end > start &&
           (s[end - 1] == ' ' || s[end - 1] == '\t' ||
            s[end - 1] == '\r' || s[end - 1] == '\n'))
        end--;

    return s.substr(start, end - start);
}

// Returns 0-4 for a valid sender category, 5 otherwise.
int categoryRank(const string& cat) {
    if (cat == "Boss")            return 0;
    if (cat == "Subordinate")     return 1;
    if (cat == "Peer")            return 2;
    if (cat == "ImportantPerson") return 3;
    if (cat == "OtherPerson")     return 4;
    return 5;
}

// Converts MM-DD-YYYY into YYYYMMDD for easy comparison.
// Returns -1 if the date is invalid.
int dateToKey(const string& date) {
    int mm, dd, yyyy;
    char extra;

    if (sscanf(date.c_str(), "%d-%d-%d%c",
               &mm, &dd, &yyyy, &extra) != 3)
        return -1;

    if (mm < 1 || mm > 12 ||
        dd < 1 || dd > 31 ||
        yyyy < 0)
        return -1;

    return yyyy * 10000 + mm * 100 + dd;
}

// Parses the contents following the EMAIL command.
// Expected format:
// SenderCategory, Subject, MM-DD-YYYY
bool parseEmail(const string& args, Email& out) {
    size_t c1 = args.find(',');
    if (c1 == string::npos) return false;

    size_t c2 = args.find(',', c1 + 1);
    if (c2 == string::npos) return false;

    out.sender = trim(args.substr(0, c1));
    out.subject = trim(args.substr(c1 + 1, c2 - c1 - 1));
    out.date = trim(args.substr(c2 + 1));

    out.rank = categoryRank(out.sender);
    out.dateKey = dateToKey(out.date);

    return out.rank >= 0 && out.rank <= 4 && out.dateKey >= 0;
}

// ----------------------------------------------------------------- Main ----
int main(int argc, char* argv[]) {
    string filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        cout << "Enter input file name: ";
        getline(cin, filename);
        filename = trim(filename);
    }

    ifstream in(filename.c_str());
    if (!in.is_open()) {
        cout << "Error: could not open file \"" << filename << "\"." << endl;
        return 1;
    }

    MaxHeap inbox;
    long arrival = 0;
    string line;

    while (getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (line.compare(0, 6, "EMAIL ") == 0) {
            Email e;
            if (parseEmail(line.substr(6), e)) {
                e.seq = arrival++;
                inbox.insert(e);
            } else {
                cout << "Warning: skipping invalid EMAIL command: " << line << endl;
            }
        }
        else if (line == "NEXT") {
            if (inbox.isEmpty()) {
                cout << "There are no emails to read." << endl;
            } else {
                const Email& e = inbox.peek();
                cout << "Next email:" << endl;
                cout << "Sender: "  << e.sender  << endl;
                cout << "Subject: " << e.subject << endl;
                cout << "Date: "    << e.date    << endl;
            }
        }
        else if (line == "READ") {
            if (inbox.isEmpty()) {
                cout << "There are no emails to read." << endl;
            } else {
                inbox.extractMax();
            }
        }
        else if (line == "COUNT") {
            cout << "There are " << inbox.size() << " emails to read." << endl;
        }
        else {
            cout << "Warning: unrecognized command: " << line << endl;
        }
    }

    return 0;
}
