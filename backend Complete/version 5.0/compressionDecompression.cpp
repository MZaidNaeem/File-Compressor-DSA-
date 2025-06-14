#include<iostream>
#include<unordered_map>
#include<fstream>
#include <bitset>
#include <cstdint>
#include <map>

using namespace std;

class HuffmanNode
{
public:
    char ch;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char c, int f)
    {
        ch = c;
        freq = f;
        left = right = nullptr;
    }
};

class MinHeapTree
{
public:
    HuffmanNode* heap[1000];
    int size;

    MinHeapTree()
    {
        size=0;
    }

    void insert(HuffmanNode *currentNode)
    {
        heap[size]=currentNode;
        heapifyUp(size);
        size++;
    }

    HuffmanNode* extractMin()
    {
        if (size == 0)
        {
            return nullptr;
        }
        HuffmanNode* root = heap[0];
        heap[0] = heap[--size];
        heapifyDown(0);
        return root;

    }

    void swap(int i, int j)
    {
        HuffmanNode* temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
    }

    void heapifyDown(int i)
    {
        int smallest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        // Check left child
        if (left < size)
        {
            if (heap[left]->freq == heap[smallest]->freq)
            {
                if (heap[left]->ch < heap[smallest]->ch)
                {
                    smallest = left;
                }
            }
            else if (heap[left]->freq < heap[smallest]->freq)
            {
                smallest = left;
            }
        }

        // Check right child
        if (right < size)
        {
            if (heap[right]->freq == heap[smallest]->freq)
            {
                if (heap[right]->ch < heap[smallest]->ch)
                {
                    smallest = right;
                }
            }
            else if (heap[right]->freq < heap[smallest]->freq)
            {
                smallest = right;
            }
        }

        if (smallest != i)
        {
            swap(i, smallest);
            heapifyDown(smallest);
        }
    }


    void heapifyUp(int i)
    {
        int parent = (i - 1) / 2;
        if (i && (
                    (heap[i]->freq < heap[parent]->freq) ||
                    (heap[i]->freq == heap[parent]->freq && heap[i]->ch < heap[parent]->ch)
                ))
        {
            swap(i, parent);
            heapifyUp(parent);
        }
    }


    int getSize()
    {
        return size;
    }


    void printHeap()
    {
        cout << "\nHeap contents (char, freq):\n";
        for (int i = 0; i < size; i++)
        {
            cout << "(" << heap[i]->ch << ", " << heap[i]->freq << ") ";
        }
        cout << endl;
    }

};


void printHuffmanTree(HuffmanNode* root, string prefix = "")
{
    if (root == nullptr)
    {
        return;
    }

    if (!root->left && !root->right)
    {
        cout << prefix << " : '" << root->ch << "' (" << root->freq << ")" << endl;
        return;
    }

    // traverse to left and right node separately
    printHuffmanTree(root->left, prefix + "0");
    printHuffmanTree(root->right, prefix + "1");
}








void buildCodes(HuffmanNode* root, unordered_map<char,string>& huffmanCodes, string code = "")
{
    if (!root)
    {
        return;
    }

    // Leaf node
    if (!root->left && !root->right)
    {

        huffmanCodes[root->ch] = code;
        return;
    }

    buildCodes(root->left,huffmanCodes, code + "0");
    buildCodes(root->right,huffmanCodes, code + "1");
}

string encode(string data,unordered_map<char,string>& huffmanCodes)
{

    string output;
    for (char c : data)
    {
        output += huffmanCodes[c];
    }
    return output;
}

string decode(HuffmanNode* root, string encoded)
{
    string decoded = "";
    HuffmanNode* current = root;

    for (int i=0; i<encoded.size(); i++)
    {

        char bit = encoded[i];
        if (bit == '0')
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }

        if (!current->left && !current->right)
        {
            decoded += current->ch;
            current = root;  // go back to root for next character
        }
    }

    return decoded;
}
string std_file_read(string fileName)
{
    ifstream inFile(fileName);
    string text, line;

    if (!inFile)
    {
        cout << "\nError in opening file: " << fileName << endl;
        return "";
    }

    while (getline(inFile, line))
    {
        text += line + "\n";
    }

    inFile.close();

    return text;

}


void std_binary_file_write(string encoded, unordered_map<char,int> freq, string fileName)
{
    ofstream outFile(fileName + "--huffman", ios::binary);
    if (!outFile)
    {
        cout << "\nCannot open file for writing.\n";
        return;
    }

    // Write the header "huffman" (7 bytes)
    outFile.write("huffman", 7);


    // Write the number of unique chars (2 bytes) i use unit16_t instead of the integer because it take only 2 bits
    int uniqueChars = freq.size();
    outFile.write(reinterpret_cast<const char*>(&uniqueChars), sizeof(uniqueChars));

    // Write frequency table entries one by one
    for (auto& pair : freq)
    {
        outFile.put(pair.first);                    // write character (1 byte)
        int f = pair.second;
        outFile.write(reinterpret_cast<const char*>(&f), sizeof(f));  // write frequency (4 bytes)
    }

    // Write the number of valid bits in last byte (1 byte)
    int validBitsInLastByte = encoded.size() % 8;
    if (validBitsInLastByte == 0) validBitsInLastByte = 8;
    outFile.put(static_cast<char>(validBitsInLastByte));

    // Write the encoded bit string as bytes
    char byte = 0;
    int bitCount = 0;
    for (char bit : encoded)
    {
        int bitValue = (bit == '1') ? 1 : 0;
        byte = (byte << 1) | bitValue;
        bitCount++;
        if (bitCount == 8)
        {
            outFile.put(byte);
            byte = 0;
            bitCount = 0;
        }
    }
    if (bitCount > 0)
    {
        byte = byte << (8 - bitCount); // pad remaining bits with zeros on right
        outFile.put(byte);
    }

    outFile.close();
}


unordered_map<char,int> readFrequencyTable(ifstream& inFile)
{
    unordered_map<char,int> freq;

    // Read number of unique chars
    int uniqueChars;
    inFile.read(reinterpret_cast<char*>(&uniqueChars), sizeof(uniqueChars));

    // Read frequency table entries
    for (int i = 0; i < uniqueChars; i++)
    {
        char ch;
        int f;

        // Use read instead of get
        inFile.read(reinterpret_cast<char*>(&ch), sizeof(ch));
        inFile.read(reinterpret_cast<char*>(&f), sizeof(f));

        freq[ch] = f;

        //cout<<endl;
        //cout<<ch<<" "<<f<<endl;
    }

    return freq;
}


string std_binary_file_read_with_freq(string fileName, unordered_map<char,int>& freq)
{
    ifstream inFile(fileName, ios::binary);
    if (!inFile)
    {
        cout << "Cannot open file for reading.\n";
        return "";
    }

    string header(7, '\0');
    inFile.read(&header[0], 7);
    if (header != "huffman") {
        cout << "Invalid file format. Expected 'huffman' header.\n";
        return "";
    }


    // Read frequency table first
    freq = readFrequencyTable(inFile);

    // Then read number of valid bits in last byte
    char validBitsChar;
    inFile.get(validBitsChar);
    int validBitsInLastByte = static_cast<unsigned char>(validBitsChar);

    string bits = "";
    char byte;

    // Read encoded bytes
    while (inFile.get(byte))
    {
        bits += bitset<8>(static_cast<unsigned char>(byte)).to_string();
    }

    inFile.close();

    // Trim padding bits
    if (!bits.empty() && validBitsInLastByte < 8)
    {
        int totalBits = bits.size();
        bits = bits.substr(0, totalBits - 8 + validBitsInLastByte);
    }

    return bits;
}













struct LZPair
{
    uint16_t index;
    char nextChar;
};

struct Node
{
    LZPair data;
    Node* next;
    Node(LZPair p)
    {
        data=p;
        next=nullptr;
    }

};

class LinkedList
{
public:
    Node* head;
    Node* tail;

    LinkedList() : head(nullptr), tail(nullptr) {}

    void append(LZPair p)
    {
        Node* newNode = new Node(p);
        if (!head)
        {
            head = tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
    }

    ~LinkedList()
    {
        Node* current = head;
        while (current)
        {
            Node* tmp = current;
            current = current->next;
            delete tmp;
        }
    }
};

// Compress input and return linked list of compressed pairs
LinkedList lz78_compress(const string& input)
{
    map<string, uint16_t> dictionary;
    string current = "";
    uint16_t dictSize = 1;
    LinkedList compressedList;

    for (char ch : input)
    {
        string next = current + ch;
        if (!dictionary[next])
        {
            uint16_t index = current == "" ? 0 : dictionary[current];
            compressedList.append({index, ch});
            dictionary[next] = dictSize++;
            current = "";
        }
        else
        {
            current = next;
        }
    }

    if (!current.empty())
    {
        compressedList.append({dictionary[current], '-'});
    }

    return compressedList;
}


// Save compressed data to a binary file
void save_compressed_to_file(const LinkedList& list, const string& filename)
{
    ofstream outFile(filename + "--lzw", ios::binary);
    if (!outFile)
    {
        cout << "Error opening file for writing\n";
        return;
    }

    // Write "lzw" header (3 bytes)
    outFile.write("lzw", 3);

    Node* current = list.head;
    while (current)
    {
        // Write uint16_t index (2 bytes)
        outFile.write(reinterpret_cast<const char*>(&current->data.index), sizeof(uint16_t));
        // Write char nextChar (1 byte)
        outFile.write(&current->data.nextChar, sizeof(char));
        current = current->next;
    }

    outFile.close();
}


// Load compressed data from a binary file and return linked list
LinkedList load_compressed_from_file(const string& filename)
{
    LinkedList list;
    ifstream inFile(filename, ios::binary);
    if (!inFile)
    {
        cout << "Error opening file for reading\n";
        return list;
    }

    // Read and check the file signature "lzw"
    char signature[4] = {};
    inFile.read(signature, 3);
    if (string(signature) != "lzw")
    {
        cout << "Invalid file format: Missing 'lzw' header\n";
        return list;
    }

    uint16_t index;
    char nextChar;
    while (inFile.read(reinterpret_cast<char*>(&index), sizeof(index)) &&
           inFile.read(&nextChar, sizeof(nextChar)))
    {
        list.append({index, nextChar});
    }

    return list;
}




// Linked list node for dictionary
struct DictNode
{
    string entry;
    DictNode* next;
    DictNode(string e)
    {
        entry=e;
        next=nullptr;
    }
};

// Helper to get the dictionary entry at a specific index
string getDictEntry(DictNode* dictHead, uint16_t index)
{
    DictNode* current = dictHead;
    for (uint16_t i = 0; i < index && current != nullptr; ++i)
    {
        current = current->next;
    }
    return current ? current->entry : "";
}

string lz78_decompress(Node* head)
{
    DictNode* dictHead = new DictNode("");
    DictNode* dictTail = dictHead;

    string result = "";
    Node* current = head;

    while (current)
    {
        uint16_t index = current->data.index;
        char nextChar = current->data.nextChar;

        string entry = getDictEntry(dictHead, index);
        if (nextChar != '-')
        {
            entry += nextChar;
        }

        result += entry;

        // Append new entry to dictionary
        DictNode* newDictNode = new DictNode(entry);
        dictTail->next = newDictNode;
        dictTail = newDictNode;

        current = current->next;
    }

    // Free dictionary memory
    DictNode* temp;
    while (dictHead)
    {
        temp = dictHead;
        dictHead = dictHead->next;
        delete temp;
    }

    return result;
}


void writeToFileFromInput(string filename,string content)
{



    ofstream outFile(filename+".txt");
    if (!outFile)
    {
        cout << "Failed to open file." << endl;
        return;
    }

    outFile << content;
    outFile.close();

    cout << "Content written to " << filename+".txt" << endl;
}

void writeToFileFromInput_atEnd(string filename,string content)
{



    ofstream outFile(filename + ".txt", ios::app);
    if (!outFile)
    {
        cout << "Failed to open file." << endl;
        return;
    }

    outFile << content << '\n';
    outFile.close();

    cout << "\nFile history saved to " << filename+".txt" << endl;
}



void readHistory(string filename)
{
    ifstream inFile(filename + ".txt");
    if (!inFile)
    {
        cout << "Failed to open file." << endl;
        return;
    }

    string line;
    int num=1;
    while (getline(inFile, line))
    {
        cout <<to_string(num) + ". "+ line << endl;
        num++;
    }

    inFile.close();
}




class historyNode
{
public:
    int id;
    string operation;
    string originalFile;
    string newFile;
    double ratios;
    historyNode* next;

    historyNode(int i, string op, string orig, string comp, double Ratio)
    {
        id = i;
        operation = op;
        originalFile = orig;
        newFile = comp;
        ratios = Ratio;
        next = nullptr;
    }
};

// Function to read file word by word and build the linked list
historyNode* readCompressionData(const string& filename)
{
    ifstream file(filename + ".txt");
    if (!file)
    {
        cerr << "Cannot open file: " << filename << endl;
        return nullptr;
    }

    string operation, word1, word2, word3;
    double ratios;
    int idCounter = 1;
    historyNode* head = nullptr;
    historyNode* tail = nullptr;

    while (file >> operation >> word1 >> word2 >> word3 >> ratios)
    {
        if (word2 != "to")
        {
            cout << "\nUnexpected format, expected 'to' but got '" << word2 << "'" << endl;
            continue;
        }

        historyNode* newNode = new historyNode(idCounter++, operation, word1, word3, ratios);

        if (!head)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
    }

    return head;
}

// Function to print the linked list
void printHistory(historyNode* head)
{
    historyNode* current = head;
    while (current)
    {
        cout << "ID: " << current->id
             << " , Operation: " << current->operation
             << " , Original File: " << current->originalFile
             << " , New File: " << current->newFile
             << " , Size Ratio: " << current->ratios << endl;
        current = current->next;
    }
}

// Function to free memory used by the linked list
void freeList(historyNode* head)
{
    while (head)
    {
        historyNode* temp = head;
        head = head->next;
        delete temp;
    }
}

// Function to create a deep copy of a linked list
historyNode* copyList(historyNode* head)
{
    if (!head) return nullptr;

    historyNode* newHead = new historyNode(head->id, head->operation, head->originalFile, head->newFile, head->ratios);
    historyNode* newTail = newHead;
    historyNode* current = head->next;

    while (current)
    {
        newTail->next = new historyNode(current->id, current->operation, current->originalFile, current->newFile, current->ratios);
        newTail = newTail->next;
        current = current->next;
    }

    return newHead;
}

// Quick sort implementation
historyNode* getTail(historyNode* cur)
{
    while (cur != nullptr && cur->next != nullptr)
        cur = cur->next;
    return cur;
}

historyNode* partition(historyNode* head, historyNode* end, historyNode** newHead, historyNode** newEnd)
{
    historyNode* pivot = end;
    historyNode *prev = nullptr, *cur = head, *tail = pivot;

    while (cur != pivot)
    {
        if (cur->ratios < pivot->ratios)
        {
            if (*newHead == nullptr)
                *newHead = cur;

            prev = cur;
            cur = cur->next;
        }
        else
        {
            if (prev) prev->next = cur->next;
            historyNode* tmp = cur->next;
            cur->next = nullptr;
            tail->next = cur;
            tail = cur;
            cur = tmp;
        }
    }

    if (*newHead == nullptr)
        *newHead = pivot;

    *newEnd = tail;
    return pivot;
}

historyNode* quickSortRecur(historyNode* head, historyNode* end)
{
    if (!head || head == end) return head;

    historyNode *newHead = nullptr, *newEnd = nullptr;
    historyNode* pivot = partition(head, end, &newHead, &newEnd);

    if (newHead != pivot)
    {
        historyNode* tmp = newHead;
        while (tmp->next != pivot)
            tmp = tmp->next;
        tmp->next = nullptr;

        newHead = quickSortRecur(newHead, tmp);
        tmp = getTail(newHead);
        tmp->next = pivot;
    }

    pivot->next = quickSortRecur(pivot->next, newEnd);
    return newHead;
}

void quickSort(historyNode** headRef)
{
    *headRef = quickSortRecur(*headRef, getTail(*headRef));
}

// Selection sort implementation
void selectionSort(historyNode* head)
{
    for (historyNode* i = head; i && i->next; i = i->next)
    {
        historyNode* minNode = i;
        for (historyNode* j = i->next; j; j = j->next)
        {
            if (j->ratios < minNode->ratios)
                minNode = j;
        }
        if (minNode != i)
            swap(i->ratios, minNode->ratios),
                 swap(i->operation, minNode->operation),
                 swap(i->originalFile, minNode->originalFile),
                 swap(i->newFile, minNode->newFile),
                 swap(i->id, minNode->id);
    }
}

// Insertion sort implementation
historyNode* insertionSort(historyNode* head)
{
    historyNode* sorted = nullptr;

    while (head)
    {
        historyNode* current = head;
        head = head->next;

        if (!sorted || current->ratios < sorted->ratios)
        {
            current->next = sorted;
            sorted = current;
        }
        else
        {
            historyNode* temp = sorted;
            while (temp->next && temp->next->ratios < current->ratios)
                temp = temp->next;

            current->next = temp->next;
            temp->next = current;
        }
    }

    return sorted;
}

// Insertion sort by ID (for binary search)
historyNode* insertionSortById(historyNode* head)
{
    historyNode* sorted = nullptr;

    while (head)
    {
        historyNode* current = head;
        head = head->next;

        if (!sorted || current->id < sorted->id)
        {
            current->next = sorted;
            sorted = current;
        }
        else
        {
            historyNode* temp = sorted;
            while (temp->next && temp->next->id < current->id)
                temp = temp->next;

            current->next = temp->next;
            temp->next = current;
        }
    }

    return sorted;
}

// Helper function to find middle node between start and end
historyNode* getMiddle(historyNode* start, historyNode* end)
{
    if (start == nullptr) return nullptr;

    historyNode* slow = start;
    historyNode* fast = start->next;

    while (fast != end)
    {
        fast = fast->next;
        if (fast != end)
        {
            slow = slow->next;
            fast = fast->next;
        }
    }
    return slow;
}

// Binary search function on a linked list sorted by ID
historyNode* binarySearchByID_LL(historyNode* head, int targetID)
{
    historyNode* start = head;
    historyNode* end = nullptr;

    while (start != end)
    {
        historyNode* mid = getMiddle(start, end);
        if (!mid) return nullptr;

        if (mid->id == targetID)
        {
            return mid;
        }
        else if (mid->id < targetID)
        {
            start = mid->next;
        }
        else
        {
            end = mid;
        }
    }
    return nullptr;
}

// Linear search by original file name
historyNode* linearSearchByOriginalFile(historyNode* head, const string& fileName)
{
    while (head)
    {
        if (head->originalFile == fileName)
            return head;
        head = head->next;
    }
    return nullptr;
}

int main()
{
    int choice;
    string fileName, input;
    historyNode* originalHistory ;

    do
    {
        originalHistory= readCompressionData("history");
       cout << "\n======================================================================\n";
    cout << "\t\tFILE COMPRESSION & DECOMPRESSION SYSTEM\n";
    cout << "======================================================================\n";
    cout << "\t\tLOSSLESS COMPRESSION MODULE - MAIN MENU\n";
    cout << "----------------------------------------------------------------------\n";
    cout << "\t1.\tView Original .TXT File Content\n";
    cout << "\t2.\tCompress File (Huffman Algorithm)\n";
    cout << "\t3.\tDecompress Huffman Compressed File\n";
    cout << "\t4.\tView Decompressed Huffman File Content\n";
    cout << "\t5.\tCompress File (LZW Algorithm)\n";
    cout << "\t6.\tDecompress LZW Compressed File\n";
    cout << "\t7.\tView Decompressed LZW File Content\n";
    cout << "\t8.\tView Compression History (Unsorted)\n";
    cout << "\t9.\tSort History by Compression Ratio\n";
    cout << "\t10.\tSearch History by ID (Binary Search)\n";
    cout << "\t11.\tPerform Linear Search on File\n";
    cout << "\t0.\tExit\n";

    cout << "======================================================================\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            cout << "\nEnter the name of the file (without .txt): ";
            cin >> fileName;

            string text = std_file_read(fileName + ".txt");
            cout << "\nFile Content:\n" << text << endl;
            break;
        }

        case 2:
        {

            unordered_map<char,string> huffmanCodes;

            cout << "\nEnter the name of the file (without .txt): ";
            cin >> fileName;

            string text = std_file_read(fileName + ".txt");

            unordered_map<char, int> freq;
            for (char c : text)
            {
                freq[c]++;
            }

            cout << "\nCharacter Frequencies:\n";
            for (auto& pair : freq)
            {
                cout << "'" << pair.first << "': " << pair.second << endl;
            }

            MinHeapTree heap;
            for (auto& pair : freq)
            {
                heap.insert(new HuffmanNode(pair.first, pair.second));
            }

            while (heap.getSize() > 1)
            {
                HuffmanNode* left = heap.extractMin();
                HuffmanNode* right = heap.extractMin();

                HuffmanNode* merged = new HuffmanNode('\0', left->freq + right->freq);
                merged->left = left;
                merged->right = right;

                heap.insert(merged);
            }

            HuffmanNode* root = heap.extractMin();
            printHuffmanTree(root);

            buildCodes(root,huffmanCodes);
            cout << "\nHuffman Codes:\n";

            string encoded = encode(text,huffmanCodes);

            cout << "\nEncoded Bitstring:\n" << encoded << "\n";


            //int originalSize = text.size(); // in bytes
            //int compressedSize=0;

            //if(freq.size()!=0){
            // frequency table: 2 bytes for size + 5 bytes for each character + 1 byte for padding info
            // int freqTableSize = 4 + freq.size() * 5 + 1;

            // encoded data size in bytes
            //   int encodedBytes = encoded.size() / 8;

            //  compressedSize = freqTableSize + encodedBytes;
            //}


            std_binary_file_write(encoded, freq, fileName);

            ifstream original(fileName + ".txt", ios::binary | ios::ate);
            int originalSize = original.tellg();
            original.close();

            ifstream compressed(fileName + "--huffman", ios::binary | ios::ate);
            int compressedSize = compressed.tellg();
            compressed.close();


            cout << "\nOriginal Size     : " << originalSize  << " Bytes";
            cout << "\nCompressed Size   : " << compressedSize  << " Bytes";
            float compressedRatio=(1.0 - (compressedSize*1.0/originalSize*1.0))*100.0;
            cout << "\nCompressed Ratio  : " << compressedRatio << " %";

            string historyData= "compress "+fileName+ ".txt to " +fileName + "--luffman.bin" + " "+ to_string(compressedRatio) ;
            writeToFileFromInput_atEnd("History",historyData);



            break;
        }


        case 3:
        {
            cout << "\nEnter the name of the compressed file (e.g., 'file--huffman'): ";
            cin >> fileName;

            unordered_map<char, int> freq;
            string encodedBits = std_binary_file_read_with_freq(fileName, freq);
            cout<<"\nEncoded Bits: "<<encodedBits;


            cout << "\nCharacter Frequencies:\n";
            MinHeapTree heap;
            for (auto& pair : freq)
            {
                if(pair.second>0)
                {
                    cout << "'" << pair.first << "': " << pair.second << endl;
                    heap.insert(new HuffmanNode(pair.first, pair.second));
                }
            }


            while (heap.getSize() > 1)
            {
                HuffmanNode* left = heap.extractMin();
                HuffmanNode* right = heap.extractMin();

                HuffmanNode* merged = new HuffmanNode('\0', left->freq + right->freq);
                merged->left = left;
                merged->right = right;

                heap.insert(merged);
            }

            HuffmanNode* root = heap.extractMin();

            printHuffmanTree(root);

            string decoded = decode(root, encodedBits);


            cout << "\nDecoded Text from File:\n" << decoded << "\n";

            writeToFileFromInput(fileName+"-to-txt",decoded);



            ifstream original(fileName, ios::binary | ios::ate);
            int originalSize = original.tellg();
            original.close();

            ifstream decompressed(fileName + "-to-txt.txt", ios::binary | ios::ate);
            int decompressedSize = decompressed.tellg();
            decompressed.close();


            cout << "\nOriginal Size     : " << originalSize  << " Bytes";
            cout << "\ndecompress Size   : " << decompressedSize  << " Bytes";
            float decompressedRatio=(1.0 - (originalSize*1.0/decompressedSize*1.0))*100.0;
            cout << "\ndecompress Ratio  : " << decompressedRatio << " %";

            writeToFileFromInput_atEnd("History","decompress "+ fileName+".bin to "+fileName+"-to-txt.txt" + " " + to_string(decompressedRatio));

            break;
        }
        case 4:
        {
            cout << "\nEnter the name of the compressed file (e.g., 'file--huffman'): ";
            cin >> fileName;

            unordered_map<char, int> freq;
            string encodedBits = std_binary_file_read_with_freq(fileName, freq);
            cout<<"\nEncoded Bits: "<<encodedBits;


            cout << "\nCharacter Frequencies:\n";
            MinHeapTree heap;
            for (auto& pair : freq)
            {
                if(pair.second>0)
                {
                    cout << "'" << pair.first << "': " << pair.second << endl;
                    heap.insert(new HuffmanNode(pair.first, pair.second));
                }
            }


            while (heap.getSize() > 1)
            {
                HuffmanNode* left = heap.extractMin();
                HuffmanNode* right = heap.extractMin();

                HuffmanNode* merged = new HuffmanNode('\0', left->freq + right->freq);
                merged->left = left;
                merged->right = right;

                heap.insert(merged);
            }

            HuffmanNode* root = heap.extractMin();

            printHuffmanTree(root);

            string decoded = decode(root, encodedBits);


            cout << "\nDecoded Text from File:\n" << decoded << "\n";



            break;
        }

        case 5:
        {
            cout << "Enter File Name to compress: ";
            cin.ignore();
            getline(cin, fileName);
            input = std_file_read(fileName + ".txt");

            // Compress
            LinkedList compressed = lz78_compress(input);
            save_compressed_to_file(compressed, fileName);
            cout << "Compressed data saved to " << fileName + "--lzw" << endl;

            ifstream original1(fileName + ".txt", ios::binary | ios::ate);
            int originalSize = original1.tellg();
            original1.close();

            ifstream compressed1(fileName + "--lzw", ios::binary | ios::ate);
            int compressedSize = compressed1.tellg();
            compressed1.close();


            cout << "\nOriginal Size     : " << originalSize  << " Bytes";
            cout << "\nCompressed Size   : " << compressedSize  << " Bytes";
            float compressedRatio=(1.0 - (compressedSize*1.0/originalSize*1.0))*100.0;
            cout << "\nCompressed Ratio  : " << compressedRatio << " %";

            writeToFileFromInput_atEnd("History","compress "+ fileName+".txt to "+fileName+"--lzw.bin" + " " + to_string(compressedRatio));

            break;
        }


        case 6:
        {
            cout << "Enter File Name to decompress: ";
            cin.ignore();
            getline(cin, fileName);

            LinkedList loadedCompressed = load_compressed_from_file(fileName);
            string decompressed = lz78_decompress(loadedCompressed.head);
            cout << "Decompressed string: " << decompressed << endl;
            writeToFileFromInput(fileName+"-to-txt",decompressed);



            ifstream original5(fileName, ios::binary | ios::ate);
            int originalSize = original5.tellg();
            original5.close();

            ifstream decompressed5(fileName + "-to-txt.txt", ios::binary | ios::ate);
            int decompressedSize = decompressed5.tellg();
            decompressed5.close();


            cout << "\nOriginal Size     : " << originalSize  << " Bytes";
            cout << "\ndecompress Size   : " << decompressedSize  << " Bytes";
            float decompressedRatio=(1.0 - (originalSize*1.0/decompressedSize*1.0))*100.0;
            cout << "\ndecompress Ratio  : " << decompressedRatio << " %";

            writeToFileFromInput_atEnd("History","decompress "+ fileName+".bin to "+fileName+"-to-txt.txt" + " " + to_string(decompressedRatio));

            break;

        }

        case 7:
        {
            cout << "Enter File Name to decompress: ";
            cin.ignore();
            getline(cin, fileName);

            LinkedList loadedCompressed = load_compressed_from_file(fileName);
            string decompressed = lz78_decompress(loadedCompressed.head);
            cout << "Decompressed string: " << decompressed << endl;
            break;

        }


        case 8:
        {
            printHistory(originalHistory);  // Always show history in the way they are added
            break;
        }

        case 9:
        {
            if (!originalHistory)
            {
                cout << "History is empty. Nothing to sort.\n";
                break;
            }

            // Created a temporary copy to sort
            historyNode* tempHistory = copyList(originalHistory);
            printHistory(tempHistory);

            cout << "\nChoose sorting algorithm:\n";
            cout << "1. Quick Sort\n";
            cout << "2. Selection Sort\n";
            cout << "3. Insertion Sort\n";
            cout << "Enter your choice: ";
            int sortChoice;
            cin >> sortChoice;

            switch (sortChoice)
            {
            case 1:
                quickSort(&tempHistory);
                cout << "Sorted with Quick Sort (temporary view):\n";
                break;
            case 2:
                selectionSort(tempHistory);
                cout << "Sorted with Selection Sort (temporary view):\n";
                break;
            case 3:
                tempHistory = insertionSort(tempHistory);
                cout << "Sorted with Insertion Sort (temporary view):\n";
                break;
            default:
                quickSort(&tempHistory);
                cout << "Sorted with Quick Sort in default case(temporary view):\n";
                break;
            }

            printHistory(tempHistory);
            freeList(tempHistory);  // Freed the temporary sorted list
            break;
        }

        case 10:
        {
            if (!originalHistory)
            {
                cout << "History is empty. Nothing to search.\n";
                break;
            }

            cout << "Enter ID to search: ";
            int targetID;
            cin >> targetID;

            // Create a sorted-by-ID copy for binary search
            historyNode* sortedById = insertionSortById(copyList(originalHistory));
            historyNode* result = binarySearchByID_LL(sortedById, targetID);

            if (result)
            {
                cout << "Found record:\n";
                cout << "ID: " << result->id
                     << " , Operation: " << result->operation
                     << " , Original File: " << result->originalFile
                     << " , New File: " << result->newFile
                     << " , Size Ratio: " << result->ratios << endl;
            }
            else
            {
                cout << "No record found with ID " << targetID << endl;
            }

            freeList(sortedById);
            break;
        }

        case 11:
        {
            if (!originalHistory)
            {
                cout << "History is empty. Nothing to search.\n";
                break;
            }

            cout << "\nChoose search option:\n";
            cout << "1. Show all compressed files\n";
            cout << "2. Show all decompressed files\n";
            cout << "3. Search by original filename\n";
            cout << "Enter your choice: ";
            int searchChoice;
            cin >> searchChoice;
            cin.ignore();

            bool found = false;
            historyNode* current = originalHistory;

            switch (searchChoice)
            {
            case 1:
                cout << "\nAll compressed files:\n";
                while (current)
                {
                    if (current->operation == "compress")
                    {
                        found = true;
                        cout << "ID: " << current->id
                            << " , Operation: " << current->operation
                             << " , Original File: " << current->originalFile
                             << " , New File: " << current->newFile
                             << " , Size Ratio: " << current->ratios << endl;
                    }
                    current = current->next;
                }
                if (!found) cout << "No compressed files found in history.\n";
                break;

            case 2:
                cout << "\nAll decompressed files:\n";
                while (current)
                {
                    if (current->operation == "decompress")
                    {
                        found = true;
                        cout << "ID: " << current->id
                             << " , Operation: " << current->operation
                             << " , Original File: " << current->originalFile
                             << " , New File: " << current->newFile
                             << " , Size Ratio: " << current->ratios << endl;
                    }
                    current = current->next;
                }
                if (!found) cout << "No decompressed files found in history.\n";
                break;

            case 3:
            {
                cout << "Enter original file name to search: ";
                getline(cin, fileName);

                bool anyFound = false;
                while (current)
                {
                    if (current->originalFile == fileName)
                    {
                        if (!anyFound)
                        {
                            cout << "\nFound records for '" << fileName << "':\n";
                            anyFound = true;
                        }
                    cout << "ID: " << current->id
                         << " , Operation: " << current->operation
                         << " , Original File: " << current->originalFile
                         << " , New File: " << current->newFile
                         << " , Size Ratio: " << current->ratios << endl;                    }
                    current = current->next;
                }
                if (!anyFound)
                {
                    cout << "No record found for original file '" << fileName << "'\n";
                }
                break;
            }

            default:
                cout << "Invalid search option.\n";
                break;
            }
            break;
        }
        case 0:
            cout << "\nExiting the program. Goodbye!\n";
            break;

        default:
            cout << "\nInvalid choice. Please try again.\n";
        }
    }
    while (choice != 0);

    // Clean up memory
    freeList(originalHistory);

    return 0;
}
