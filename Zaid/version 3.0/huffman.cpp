#include<iostream>
#include<unordered_map>
#include<fstream>
#include <bitset>
#include <cstdint>

using namespace std;

class HuffmanNode {
public:
    char ch;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char c, int f) {
        ch = c;
        freq = f;
        left = right = nullptr;
    }
};

class MinHeapTree{
public:
    HuffmanNode* heap[1000];
    int size;

    MinHeapTree(){
        size=0;
    }

    void insert(HuffmanNode *currentNode){
        heap[size]=currentNode;
        heapifyUp(size);
        size++;
    }

    HuffmanNode* extractMin(){
        if (size == 0) {
            return nullptr;
        }
        HuffmanNode* root = heap[0];
        heap[0] = heap[--size];
        heapifyDown(0);
        return root;

    }

    void swap(int i, int j) {
        HuffmanNode* temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
    }

    void heapifyDown(int i) {
        int smallest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        // Check left child
        if (left < size) {
            if (heap[left]->freq == heap[smallest]->freq) {
                if (heap[left]->ch < heap[smallest]->ch) {
                    smallest = left;
                }
            } else if (heap[left]->freq < heap[smallest]->freq) {
                smallest = left;
            }
        }

        // Check right child
        if (right < size) {
            if (heap[right]->freq == heap[smallest]->freq) {
                if (heap[right]->ch < heap[smallest]->ch) {
                    smallest = right;
                }
            } else if (heap[right]->freq < heap[smallest]->freq) {
                smallest = right;
            }
        }

        if (smallest != i) {
            swap(i, smallest);
            heapifyDown(smallest);
        }
}


    void heapifyUp(int i) {
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


    int getSize(){
        return size;
    }


    void printHeap() {
    cout << "\nHeap contents (char, freq):\n";
    for (int i = 0; i < size; i++) {
        cout << "(" << heap[i]->ch << ", " << heap[i]->freq << ") ";
    }
    cout << endl;
    }

};


void printHuffmanTree(HuffmanNode* root, string prefix = "") {
    if (root == nullptr) {
        return;
    }

    if (!root->left && !root->right) {
        cout << prefix << " : '" << root->ch << "' (" << root->freq << ")" << endl;
        return;
    }

    // traverse to left and right node separately
    printHuffmanTree(root->left, prefix + "0");
    printHuffmanTree(root->right, prefix + "1");
}

void buildCodes(HuffmanNode* root, unordered_map<char,string>& huffmanCodes, string code = "") {
    if (!root) {
        return;
    }

    // Leaf node
    if (!root->left && !root->right) {

        huffmanCodes[root->ch] = code;
        return;
    }

    buildCodes(root->left,huffmanCodes, code + "0");
    buildCodes(root->right,huffmanCodes, code + "1");
}

string encode(string data,unordered_map<char,string>& huffmanCodes) {

    string output;
    for (char c : data) {
        output += huffmanCodes[c];
    }
    return output;
}

string decode(HuffmanNode* root, string encoded) {
    string decoded = "";
    HuffmanNode* current = root;

    for (int i=0;i<encoded.size();i++) {

        char bit = encoded[i];
        if (bit == '0') {
            current = current->left;
        } else {
            current = current->right;
        }

        if (!current->left && !current->right) {
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


void std_binary_file_write(string encoded, unordered_map<char,int> freq, string fileName) {
    ofstream outFile(fileName + "--Compressed", ios::binary);
    if (!outFile) {
        cout << "\nCannot open file for writing.\n";
        return;
    }

    // Write the number of unique chars (2 bytes) i use unit16_t instead of the integer because it take only 2 bits
    int uniqueChars = freq.size();
    outFile.write(reinterpret_cast<const char*>(&uniqueChars), sizeof(uniqueChars));

    // Write frequency table entries one by one
    for (auto& pair : freq) {
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
    for (char bit : encoded) {
        int bitValue = (bit == '1') ? 1 : 0;
        byte = (byte << 1) | bitValue;
        bitCount++;
        if (bitCount == 8) {
            outFile.put(byte);
            byte = 0;
            bitCount = 0;
        }
    }
    if (bitCount > 0) {
        byte = byte << (8 - bitCount); // pad remaining bits with zeros on right
        outFile.put(byte);
    }

    outFile.close();
}


unordered_map<char,int> readFrequencyTable(ifstream& inFile) {
    unordered_map<char,int> freq;

    // Read number of unique chars
    int uniqueChars;
    inFile.read(reinterpret_cast<char*>(&uniqueChars), sizeof(uniqueChars));

    // Read frequency table entries
    for (int i = 0; i < uniqueChars; i++) {
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


string std_binary_file_read_with_freq(string fileName, unordered_map<char,int>& freq) {
    ifstream inFile(fileName, ios::binary);
    if (!inFile) {
        cout << "Cannot open file for reading.\n";
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
    while (inFile.get(byte)) {
        bits += bitset<8>(static_cast<unsigned char>(byte)).to_string();
    }

    inFile.close();

    // Trim padding bits
    if (!bits.empty() && validBitsInLastByte < 8) {
        int totalBits = bits.size();
        bits = bits.substr(0, totalBits - 8 + validBitsInLastByte);
    }

    return bits;
}



int main() {
    int choice;
    string fileName;

    string compress="";
    string decompress="d";

    do {
        cout << "\n==============================\n";
        cout << "     HUFFMAN COMPRESSION\n";
        cout << "==============================\n";
        cout << "1. Compress a file\n";
        cout << "2. Decompress a file\n";
        cout << "3. Check equality of compress data and decompress from file\n";
        cout << "0. Exit\n";
        cout << "------------------------------\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 3:
                if(compress==decompress){
                    cout<<"\nBoth the String are equal"<<endl;
                }
                else{
                    cout<<"\nNot Equal:"<<endl;
                }

            case 1: {

                unordered_map<char,string> huffmanCodes;

                cout << "\nEnter the name of the file (without .txt): ";
                cin >> fileName;

                string text = std_file_read(fileName + ".txt");
                cout << "\nFile Content:\n" << text << endl;

                unordered_map<char, int> freq;
                for (char c : text) {
                    freq[c]++;
                }

                cout << "\nCharacter Frequencies:\n";
                for (auto& pair : freq) {
                    cout << "'" << pair.first << "': " << pair.second << endl;
                }

                MinHeapTree heap;
                for (auto& pair : freq) {
                    heap.insert(new HuffmanNode(pair.first, pair.second));
                }

                while (heap.getSize() > 1) {
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

                string decoded = decode(root, encoded);

                cout << "\nEncoded Bitstring:\n" << encoded << "\n";
                cout << "\nDecoded Text:\n" << decoded << "\n";

                compress = decoded;

                //int originalSize = text.size(); // in bytes
                //int compressedSize=0;

                //if(freq.size()!=0){
                // frequency table: 2 bytes for size + 5 bytes for each character + 1 byte for padding info
                    int freqTableSize = 4 + freq.size() * 5 + 1;

                // encoded data size in bytes
                 //   int encodedBytes = encoded.size() / 8;

                  //  compressedSize = freqTableSize + encodedBytes;
                //}


                std_binary_file_write(encoded, freq, fileName);

                ifstream original(fileName + ".txt", ios::binary | ios::ate);
                int originalSize = original.tellg();
                original.close();

                ifstream compressed(fileName + "--Compressed", ios::binary | ios::ate);
                int compressedSize = compressed.tellg();
                compressed.close();


                cout << "\nOriginal Size     : " << originalSize  << " Bytes";
                cout << "\nCompressed Size   : " << compressedSize  << " Bytes";


                break;
            }

            case 2: {
                cout << "\nEnter the name of the compressed file (e.g., 'file--Compressed'): ";
                cin >> fileName;

                unordered_map<char, int> freq;
                string encodedBits = std_binary_file_read_with_freq(fileName, freq);
                cout<<"\nEncoded Bits: "<<encodedBits;


                cout << "\nCharacter Frequencies:\n";
                MinHeapTree heap;
                for (auto& pair : freq) {
                    if(pair.second>0){
                        cout << "'" << pair.first << "': " << pair.second << endl;
                        heap.insert(new HuffmanNode(pair.first, pair.second));
                    }
                }


                while (heap.getSize() > 1) {
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
                decompress=decoded;

                break;
            }

            case 0:
                cout << "\nExiting the program. Goodbye!\n";
                break;

            default:
                cout << "\nInvalid choice. Please try again.\n";
        }

        cout << "\nPress any key to continue...";
        cin.ignore();
        cin.get();

    } while (choice != 0);

    return 0;
}

