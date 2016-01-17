#include <string>
#include <map>
#include <cassert>
#include"PriQueue.h"

#define TABLE_SIZE 256
#define CHUNK_SIZE (4 * 1024)
typedef unsigned char uchar;
struct HuffmanNode {
	long long weight;
	int parent;
	int lchild;
	int rchild;
	int key;
	HuffmanNode(long long x = 0, int l = -1, int r = -1, int k = -1, int p = -1): weight(x), lchild(l), 
		rchild(r), key(k), parent(p){}
};

long long freq_table[TABLE_SIZE];
HuffmanNode huffman_tree[2 * TABLE_SIZE - 1];
map<uchar, string> huffman_code_table;
int root = -1;
void BuildFreqTable(const char* file_name) {
	for (int i = 0; i < TABLE_SIZE; i++)
		freq_table[i] = 0;
	FILE * fin = fopen(file_name, "rb");
	if (fin == nullptr) {
		cout << "Cannot open the file " << file_name << endl;
		return;
	}
	uchar buffer[CHUNK_SIZE];
	int buffer_size_read = fread(buffer, sizeof(uchar), CHUNK_SIZE, fin);
	while (!feof(fin)) {
		for (int i = 0; i < buffer_size_read; ++i)
			freq_table[buffer[i]]++;
		buffer_size_read = fread(buffer, sizeof(uchar), CHUNK_SIZE, fin);
	}
	for (int i = 0; i < buffer_size_read; ++i)
		freq_table[buffer[i]]++;
	fclose(fin);
}

void BuildHuffmanTree() {
	// 1st step, initalize huffman_tree
	for (int i = 0; i < 2 * TABLE_SIZE - 1; i++) {
		huffman_tree[i].lchild = -1;
		huffman_tree[i].rchild = -1;
		huffman_tree[i].weight = 0;
		huffman_tree[i].parent = i;
		huffman_tree[i].key = -1;
	}
	int num_nonzeros = 0;
	PriQueue<HeapNode> priq(2 * TABLE_SIZE- 1);
	// 2nd step, build heap
	for (int i = 0; i < TABLE_SIZE; ++i) {
		if (freq_table[i] != 0) {
			priq.insert(HeapNode(freq_table[i], i, true));
		}
	}
	// 3rd step, build huffman_tree
	int internal_id = TABLE_SIZE;
	while (!priq.empty()) {
		if (priq.size() > 1) {
			HeapNode left = priq.extract_min();
			HeapNode right = priq.extract_min();
			HeapNode t(left.weight + right.weight, internal_id, false);	
			priq.insert(t);
			// set children
			
			huffman_tree[left.key].parent = internal_id;
			huffman_tree[internal_id].lchild = left.key;
			huffman_tree[right.key].parent = internal_id;
			huffman_tree[internal_id].rchild = right.key;
			huffman_tree[internal_id].weight = left.weight + right.weight;
			if (left.leaf) {	
				huffman_tree[left.key].lchild = huffman_tree[left.key].rchild = -1;
				huffman_tree[left.key].weight = left.weight;	
				huffman_tree[left.key].key = left.key;
			}
			if(right.leaf) {
				huffman_tree[right.key].lchild = huffman_tree[right.key].rchild = -1;
				huffman_tree[right.key].weight = right.weight;
				huffman_tree[right.key].key = right.key;
			}
			internal_id++;
		}
		else{
			HeapNode l = priq.extract_min();
			huffman_tree[l.key].parent = -1;
			root = l.key;
		}
	}

	// 4th step, get encode string
	for (int i = 0; i < TABLE_SIZE; i++) {
		if (huffman_tree[i].weight != 0) {
			// has this key
			string str;
			int p, c = i;
			while (c != root) {
				p = huffman_tree[c].parent;
				if (huffman_tree[p].lchild == c)
					str += "0";
				else if (huffman_tree[p].rchild == c)
					str += "1";
				else{
					cout << "Fatal error happend during encoding!" << endl;
					return;
				}
				c = p;
			}
			reverse(str.begin(), str.end());
			huffman_code_table[i] = str;
		}
	}
}

void encode(string& left, const uchar* buffer, int buffer_size, string& encode_buffer) {
	//string left;
	encode_buffer = "";
	//Note encode buffer's length may exceed buffer_size, so here I have to use string instead.
	for (int i = 0; i < buffer_size; i++) {
		const string& str = huffman_code_table[buffer[i]];
		left += str;
		uchar byte = 0;
		while (left.length() >= 8) {

			for (int j = 0; j < 8; j++) {
				byte <<= 1;
				byte |= (left[j] - '0');
			}
			//encode_buffer[encode_len++] = byte;
			encode_buffer.push_back(byte);
			left = left.substr(8);
		}
	}
}

void EncodeFile(const char* src_file_name, const char* tgt_file_name) {
	FILE * fin = fopen(src_file_name, "rb");
	FILE * fout = fopen(tgt_file_name, "wb");
	if (fin == nullptr || fout == nullptr) {
		cout << "Cannot open the file " << src_file_name <<" or "<< tgt_file_name << endl;
		return;
	}
	// 1st step, write file head.
	// File format is as follows: 0 ~ 3 bytes: root index, 4~root*sizeof(HuffmanTree)
	/*******************************************************************************/
	fwrite(&root, sizeof(int), 1, fout);
	int bit_padding = 0;
	fwrite(&bit_padding, sizeof(int), 1, fout);
	fwrite(huffman_tree, sizeof(HuffmanNode), root + 1, fout);

	uchar buffer[CHUNK_SIZE];
	//uchar encode_buffer[CHUNK_SIZE];
	string left;
	int encode_len = 0;
	string encode_buffer;
	int buffer_size_read = fread(buffer, sizeof(uchar), CHUNK_SIZE, fin);
	while (!feof(fin)) {
		encode(left, buffer, buffer_size_read, encode_buffer);
		fwrite(encode_buffer.c_str(), sizeof(uchar), encode_buffer.length(), fout);
		//encode_len = 0;
		buffer_size_read = fread(buffer, sizeof(uchar), CHUNK_SIZE, fin);
	}
	if (buffer_size_read > 0) {
		encode(left, buffer, buffer_size_read, encode_buffer);
		fwrite(encode_buffer.c_str(), sizeof(uchar), encode_buffer.length(), fout);
	}
	if (left.length() > 0) {
		uchar byte = 0;
		for (size_t i = 0; i < left.length(); i++) {
			byte <<= 1;
			byte |= (left[i] - '0');
		}
		for (size_t i = left.length(); i < 8; i++) {
			byte <<= 1;
			byte |= 0;
		}
		fwrite(&byte, 1, 1, fout);
	}
	fclose(fin);
	bit_padding = 8 - left.length();
	if (bit_padding > 0) {
		fseek(fout, sizeof(int), SEEK_SET);
		fwrite(&bit_padding, sizeof(int), 1, fout);
		fseek(fout, 0, SEEK_END);
	}
	fclose(fout);
}

void decode(HuffmanNode* tree, int tree_root, int& last_node, const uchar* buffer, int buf_size, 
	string& decode_buf){
	//uchar key = 0;
	decode_buf = "";
	int j, c = last_node;
	for (int i = 0; i < buf_size; i++) {
		for (j = 0; j < 8; j++) {
			if (c > 255) {
				int bit = ((buffer[i] & (1 << (7 - j))) >> (7 - j));
				//p = c;
				if (bit == 0)
					c = tree[c].lchild;
				else if (bit == 1)
					c = tree[c].rchild;
				if (c <= 255) {
					//assert(decode_buf_siz < CHUNK_SIZE - 1);
					decode_buf.push_back(tree[c].key);
					//decode_buf[decode_buf_size++] = tree[c].key;
					c = tree_root;
				}
			}
		}
	}
	last_node = c;
}

void DecodeFile(const char* src_file_name, const char* tgt_file_name) {
	FILE *fin = fopen(src_file_name, "rb");
	if (fin == nullptr) {
		cout << "Cannot open the file " << src_file_name << endl;
		return;
	}
	FILE *fout = fopen(tgt_file_name, "wb");
	if (fout == nullptr) {
		cout << "Cannot open the file " << src_file_name << endl;
		return;
	}
	uchar src_buffer[CHUNK_SIZE];
	//uchar tgt_buffer[CHUNK_SIZE];
	string tgt_buffer;
	//1st step, read file head.
	int tree_root;
	fread(&tree_root, sizeof(int), 1, fin);
	int bit_padding;
	fread(&bit_padding, sizeof(int), 1, fin);
	if (tree_root < 0 || tree_root > 2 * TABLE_SIZE - 1) {
		cout << "Unkown file!" << endl;
		fclose(fout);
		fclose(fin);
		return;
	}
	HuffmanNode* tree = new HuffmanNode[tree_root + 1];
	fread(tree, sizeof(HuffmanNode), tree_root + 1, fin);
	int buf_size_read = fread(src_buffer, sizeof(uchar), CHUNK_SIZE, fin);
	int last_root = tree_root, decode_buf_size = 0;
	while (!feof(fin)) {
		decode(tree, tree_root, last_root, src_buffer, buf_size_read, tgt_buffer);
		fwrite(tgt_buffer.c_str(), sizeof(uchar), tgt_buffer.length(), fout);
		//decode_buf_size = 0;
		//break;
		buf_size_read = fread(src_buffer, sizeof(uchar), CHUNK_SIZE, fin);
	}
	if (buf_size_read > 1) {
		if (bit_padding > 0) {
			decode(tree, tree_root, last_root, src_buffer, buf_size_read - 1, tgt_buffer);
			fwrite(tgt_buffer.c_str(), sizeof(uchar), tgt_buffer.length(), fout);
			uchar byte = src_buffer[buf_size_read - 1];
			int c = last_root;
			for (int i = 0; i < 8 - bit_padding; i++) {
				int bit = ((byte & (1 << (7 - i))) >> (7 - i));
				if (bit == 0)
					c = tree[c].lchild;
				else if (bit == 1)
					c = tree[c].rchild;
			}
			byte = tree[c].key;
			fwrite(&byte, sizeof(uchar), 1, fout);
		}
	}
	delete[] tree;
	fclose(fout);
	fclose(fin);
}

int main()
{	
	const char* src_file_name = "E:/Download/change/1.rmvb";
	const char* tgt_file_name = "E:/Download/change/2.data";
	BuildFreqTable(src_file_name);
	BuildHuffmanTree();
	
	EncodeFile(src_file_name, tgt_file_name);
	DecodeFile(tgt_file_name, "E:/Download/change/2.rmvb");
	return 0;
}