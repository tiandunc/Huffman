#include "PriQueue.h"

template <typename Dtype>
PriQueue<Dtype>::PriQueue(int m)
{
	if (m <= 0)
	{
		cout << "PriQueue initialization failed!" << endl;
		exit(1);
	}
	max_size_ = m;
	data_ = new Dtype[max_size_ + 1];
	cur_size_ = 0;
}

template <typename Dtype>
PriQueue<Dtype>::~PriQueue()
{
	delete[] data_;
}

template <typename Dtype>
void PriQueue<Dtype>::insert(const Dtype& t) {
	int i, p;
	cur_size_++;
	if (cur_size_ >= max_size_) { // reallocate memory
		int tmp_size = max_size_ * 2 - 1;
		Dtype* tmp = new Dtype[tmp_size];
		for (i = 0; i < max_size_; i++) {
			tmp[i] = data_[i];
		}
		delete[] data_;
		data_ = tmp;
		max_size_ = tmp_size;
	}
	data_[cur_size_] = t;
	for (i = cur_size_; i > 1 && data_[p = i / 2] > data_[i]; i = p)
		swap(p, i);
}

template <typename Dtype>
Dtype PriQueue<Dtype>::extract_min() {
	int i, c;
	Dtype t = data_[1];
	data_[1] = data_[cur_size_--];
	for (i = 1; (c = 2 * i) <= cur_size_; i = c) {
		if (c + 1 <= cur_size_ && data_[c + 1] < data_[c])
			c++;
		if (data_[i] <= data_[c])
			break;
		swap(c, i);
	}
	return t;
}
template class PriQueue<int>;
template class PriQueue<float>;
template class PriQueue<long>;
template class PriQueue<double>;
template class PriQueue<long long>;
template class PriQueue<unsigned int>;
template class PriQueue<HeapNode>;
