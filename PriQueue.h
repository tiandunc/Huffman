#ifndef PRIQUEUE 
#define PRIQUEUE 1
#include<iostream>
using namespace std;
struct HeapNode {
	long long weight;
	int key;
	bool leaf;
	HeapNode(long long x = 0, int k = 0, bool flag_leaf = true) : weight(x), key(k), leaf(flag_leaf) {}
	inline bool operator < (const HeapNode& x) {
		return weight < x.weight;
	}
	inline bool operator <=(const HeapNode& x) {
		return weight <= x.weight;
	}
	inline bool operator >(const HeapNode& x) {
		return weight > x.weight;
	}
	inline bool operator >=(const HeapNode& x) {
		return weight >= x.weight;
	}
	inline bool operator == (const HeapNode& x) {
		return weight == x.weight;
	}
};
template <typename Dtype>
class PriQueue
{
public:
	PriQueue(int m);
	virtual ~PriQueue();
	void insert(const Dtype& t);
	inline bool empty() const {
		return cur_size_ == 0;
	}
	inline int size() const {
		return cur_size_;
	}
	Dtype extract_min();
protected:
	inline void swap(int i, int j) {
		Dtype t = data_[i];
		data_[i] = data_[j];
		data_[j] = t;
	}
private:
	int cur_size_;
	int max_size_;
	Dtype* data_;
};
#endif