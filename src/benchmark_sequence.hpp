#ifndef YGG_BENCHMARK_SEQUENCE_HPP
#define YGG_BENCHMARK_SEQUENCE_HPP

#include <fstream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ygg {
namespace utilities {

template <class KeyT>
class BenchmarkSequenceStorage {
public:
	enum class Type : char
	{
		INSERT,
		DELETE,
		ERASE,
		LBOUND,
		UBOUND,
		SEARCH
	};

	struct Entry
	{
		Type type;
		KeyT key;
		const void * id;

		Entry(Type type_in, KeyT key_in, const void * id_in)
		    : type(type_in), key(key_in), id(id_in)
		{}

		Entry() = default;
	};

	class WrongTypeException {
	};

	struct TypeInfo
	{
		size_t version;
		size_t key_size;
		bool is_integral;
		bool is_float;
		bool is_signed;

		bool
		operator!=(const TypeInfo & other)
		{
			return !((this->version == other.version) &&
			         (this->key_size == other.key_size) &&
			         (this->is_integral == other.is_integral) &&
			         (this->is_float == other.is_float) &&
			         (this->is_signed == other.is_signed));
		}
	};

	class Reader {
	public:
		Reader(std::string path);

		const std::vector<Entry> & get(size_t count);
		void reset();

	private:
		void verify_type();
		std::vector<Entry> buf;
		std::ifstream infile;
		size_t remaining_in_chunk;
	};

	void register_insert(const void * id, const KeyT & key);
	void register_delete(const void * id, const KeyT & key);
	void register_erase(const void * id, const KeyT & key);
	void register_search(const void * id, const KeyT & key);
	void register_lbound(const void * id, const KeyT & key);
	void register_ubound(const void * id, const KeyT & key);

	BenchmarkSequenceStorage();
	BenchmarkSequenceStorage(std::string filename_in);
	~BenchmarkSequenceStorage();

private:
	static constexpr size_t BUFSIZE = 1000000;

	void write_typeinfo();
	void sync();

	std::string filename;
	std::string get_filename() const;

	std::vector<Entry> buf;
	std::ofstream outfile;
};

} // namespace utilities
} // namespace ygg

#ifndef YGG_BENCHMARK_SEQUENCE_CPP
#include "benchmark_sequence.cpp"
#endif

#endif
