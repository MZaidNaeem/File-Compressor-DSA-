#include<iostream>
#include<unordered_map>
using namespace std;

string huffmanCodes[128];


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

        if (left < size && heap[left]->freq < heap[smallest]->freq)
            smallest = left;
        if (right < size && heap[right]->freq < heap[smallest]->freq)
            smallest = right;

        if (smallest != i) {
            swap(i, smallest);
            heapifyDown(smallest);
        }
    }

    void heapifyUp(int i){
        int parent = (i - 1) / 2;
        if (i && heap[i]->freq < heap[parent]->freq) {
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

void buildCodes(HuffmanNode* root, string code = "") {
    if (!root) {
        return;
    }

    // Leaf node
    if (!root->left && !root->right) {
        int index= root->ch;
        huffmanCodes[index] = code;
        return;
    }

    buildCodes(root->left, code + "0");
    buildCodes(root->right, code + "1");
}

string encode(string data) {

    string output;
    for(int i=0;i<data.size();i++){
        int index=data[i];
        output+=huffmanCodes[index];
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

int main(){

    cout<<"Enter the string to Encode and decode: ";
    string text;
    getline(cin , text);
    cout<<text;

// counting frequecies
    unordered_map<char,int> freq;

    for(int i=0;i<text.size();i++){
        freq[text[i]]=freq[text[i]] + 1;
    }
// printing the frequencies
    cout<<endl<<"Printing the frequencies of the data: "<<endl;
    for(auto data:freq){
        cout<<data.first<<" ";
        cout<<data.second<<" ";
        cout<<endl;
    }

// building a min heap tree based on the frequencies
    MinHeapTree heap;
    for(auto data:freq){
        HuffmanNode* currentNode= new HuffmanNode(data.first,data.second);
        heap.insert(currentNode);
    }

// printing minHeap Tree
    heap.printHeap();

// making huffman Tree

    while(heap.getSize()>1){
        HuffmanNode* left= heap.extractMin();
        HuffmanNode* right= heap.extractMin();

        HuffmanNode* merged = new HuffmanNode('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;

        heap.insert(merged);
    }


    HuffmanNode* root = heap.extractMin();
    cout<<"\nPrinting the Huffman Tree"<<endl;
    printHuffmanTree(root);

//  Generate codes
    buildCodes(root);

// Display codes
    cout << "Huffman Codes:\n";
    for (int i = 0; i < 128; i++) {
        char ch = char(i);
        if (freq[ch] > 0) {
            cout << (char)i << ": " << huffmanCodes[i] << "\n";
        }
    }
//  Encode
    string encoded= encode(text);
    cout << "\nEncoded string: " << encoded << "\n";

// decoded
    string decoded = decode(root, encoded);
    cout << "\nDecoded string: " << decoded << "\n";

// calculating size of different file parts

    int originalSize = text.size()*8;
    int compressedSize=encoded.size();


// Printing Different Statistics

    cout<<"\nOriginal Size: "<<originalSize<<" KB"<<endl;
    cout<<"Compressed File Size: "<<compressedSize<<endl;

    return 0 ;

}
