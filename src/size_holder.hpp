//
// Created by lukas on 07.11.17.
//

#ifndef YGG_SIZE_HOLDER_HPP
#define YGG_SIZE_HOLDER_HPP

template<bool enable>
class SizeHolder {};

template<>
class SizeHolder<true> {
public:
	SizeHolder() : n(0) {};

	void add(size_t i) {
		this->n += i;
	}

	void reduce(size_t i) {
		this->n -= i;
	}

	size_t get() const {
		return this->n;
	}

private:
	size_t n;
};

template<>
class SizeHolder<false> {
public:
	void add(size_t i) {
		(void)i;
	}

	void reduce(size_t i) {
		(void)i;
	}
private:
};

#endif //YGG_SIZE_HOLDER_HPP
