#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <cstdint>

using namespace std;

struct LZPair {
    uint16_t index;
    char nextChar;
};

struct Node {
    LZPair data;
    Node* next;
    Node(LZPair p) : data(p), next(nullptr) {}
};

class LinkedList {
public:
    Node* head;
    Node* tail;

    LinkedList() : head(nullptr), tail(nullptr) {}

    void append(LZPair p) {
        Node* newNode = new Node(p);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    ~LinkedList() {
        Node* current = head;
        while (current) {
            Node* tmp = current;
            current = current->next;
            delete tmp;
        }
    }
};

// Compress input and return linked list of compressed pairs
LinkedList lz78_compress(const string& input) {
    map<string, uint16_t> dictionary;
    string current = "";
    uint16_t dictSize = 1;
    LinkedList compressedList;

    for (char ch : input) {
        string next = current + ch;
        if (dictionary.find(next) == dictionary.end()) {
            uint16_t index = current == "" ? 0 : dictionary[current];
            compressedList.append({index, ch});
            dictionary[next] = dictSize++;
            current = "";
        } else {
            current = next;
        }
    }

    if (!current.empty()) {
        compressedList.append({dictionary[current], '-'});
    }

    return compressedList;
}

string std_file_read(string fileName)
{
    ifstream inFile(fileName);
    string text, line;

    if (!inFile) {
        cout << "\nError in opening file: " << fileName << endl;
        return "";
    }

    while (getline(inFile, line)) {
        text += line + "\n";
    }

    inFile.close();

    return text;

}
// Save compressed data to a binary file
void save_compressed_to_file(const LinkedList& list, const string& filename) {
    ofstream outFile(filename, ios::binary);
    if (!outFile) {
        cout << "Error opening file for writing\n";
        return;
    }

    Node* current = list.head;
    while (current) {
        // Write uint16_t index (2 bytes)
        outFile.write(reinterpret_cast<const char*>(&current->data.index), sizeof(uint16_t));
        // Write char nextChar (1 byte)
        outFile.write(&current->data.nextChar, sizeof(char));
        current = current->next;
    }

    outFile.close();
}

// Load compressed data from a binary file and return linked list
LinkedList load_compressed_from_file(const string& filename) {
    LinkedList list;
    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        cerr << "Error opening file for reading\n";
        return list;
    }

    while (true) {
        uint16_t index;
        char nextChar;

        inFile.read(reinterpret_cast<char*>(&index), sizeof(uint16_t));
        if (inFile.eof()) break;

        inFile.read(&nextChar, sizeof(char));
        if (inFile.eof()) break;

        list.append({index, nextChar});
    }

    inFile.close();
    return list;
}

// Decompress from linked list of pairs
string lz78_decompress(Node* head) {
    const int MAX_SIZE = 10000;
    string dictionary[MAX_SIZE];
    dictionary[0] = "";
    uint16_t dictSize = 1;
    string result = "";

    Node* current = head;
    while (current) {
        uint16_t index = current->data.index;
        char nextChar = current->data.nextChar;

        string entry = dictionary[index];
        if (nextChar != '-') {
            entry += nextChar;
        }

        result += entry;
        dictionary[dictSize++] = entry;

        current = current->next;
    }

    return result;
}

int main() {
    string input,fileName;
    cout << "Enter File Name to compress: ";
    getline(cin,fileName);
    input=std_file_read(fileName+".txt");

    // Compress
    LinkedList compressed = lz78_compress(input);

    // Save compressed data to file
    string filename = "compressed.lz78";
    save_compressed_to_file(compressed, filename);
    cout << "Compressed data saved to " << filename << endl;

    // Load compressed data from file
    LinkedList loadedCompressed = load_compressed_from_file(filename);

    // Decompress
    string decompressed = lz78_decompress(loadedCompressed.head);
    cout << "Decompressed string: " << decompressed << endl;

    return 0;
}
