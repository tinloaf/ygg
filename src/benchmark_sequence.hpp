#ifndef YGG_BENCHMARK_SEQUENCE_HPP
#define YGG_BENCHMARK_SEQUENCE_HPP

#include <cassert>
#include <fstream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ygg {
namespace utilities {

struct NoValue
{
};

template <class KeyT, class SearchKeyT = KeyT, class ValueT = NoValue>
class BenchmarkSequenceStorage {
private:
	constexpr static bool has_value = !std::is_same_v<ValueT, NoValue>;

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
		std::variant<KeyT, SearchKeyT> key;
		ValueT value;
		const void * id;

		//		template <class Dummy = KeyT>
		//		Entry(Type type_in, typename std::enable_if_t<!has_value, Dummy>
		// key_in,
		Entry(Type type_in, KeyT key_in, const void * id_in)
		    : type(type_in), id(id_in)
		{
			switch (type) {
			case Type::INSERT:
			case Type::DELETE:
			case Type::ERASE:
				this->key =
				    std::variant<KeyT, SearchKeyT>(std::in_place_index_t<0>{}, key_in);
				break;
			case Type::LBOUND:
			case Type::UBOUND:
			case Type::SEARCH:
				this->key =
				    std::variant<KeyT, SearchKeyT>(std::in_place_index_t<1>{}, key_in);
				break;
			default:
				assert(false);
			}
		}

		template <class Dummy = ValueT>
		Entry(Type type_in, KeyT key_in,
		      std::enable_if_t<has_value, Dummy> value_in, const void * id_in)
		    : type(type_in), key(key_in), value(value_in), id(id_in)
		{}

		template <class Dummy = KeyT>
		Entry(
		    Type type_in,
		    std::enable_if_t<!std::is_same_v<Dummy, SearchKeyT>, SearchKeyT> key_in,
		    const void * id_in)
		    : type(type_in), id(id_in)
		{
			switch (type) {
			case Type::INSERT:
			case Type::DELETE:
			case Type::ERASE:
				// Called with wrong type of key!
				throw "Called with wrong type of key";
				break;
			case Type::LBOUND:
			case Type::UBOUND:
			case Type::SEARCH:
				this->key =
				    std::variant<KeyT, SearchKeyT>(std::in_place_index_t<1>{}, key_in);
				break;
			default:
				assert(false);
			}
		}

		template <class Dummy = ValueT, class KeyDummy = KeyT>
		Entry(Type type_in,
		      std::enable_if_t<!std::is_same_v<KeyDummy, SearchKeyT>, SearchKeyT>
		          key_in,
		      std::enable_if_t<has_value, Dummy> value_in, const void * id_in)
		    : type(type_in), key(key_in), value(value_in), id(id_in)
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
		bool present;

		bool
		operator!=(const TypeInfo & other)
		{
			return !((this->version == other.version) &&
			         (this->key_size == other.key_size) &&
			         (this->is_integral == other.is_integral) &&
			         (this->is_float == other.is_float) &&
			         (this->is_signed == other.is_signed) &&
			         (this->present = other.present));
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

	template <class Dummy = KeyT>
	void register_insert(const void * id,
	                     const std::enable_if_t<!has_value, Dummy> & key);
	template <class Dummy = ValueT>
	void register_insert(const void * id, const KeyT & key,
	                     const std::enable_if_t<has_value, Dummy> & value);

	void register_delete(const void * id, const KeyT & key);
	void register_erase(const void * id, const KeyT & key);
	void register_search(const void * id, const SearchKeyT & key);
	void register_lbound(const void * id, const SearchKeyT & key);
	void register_ubound(const void * id, const SearchKeyT & key);

	BenchmarkSequenceStorage();
	BenchmarkSequenceStorage(std::string filename_in);
	~BenchmarkSequenceStorage();

private:
	static constexpr size_t BUFSIZE = 1000000;

	void write_typeinfo();
	void sync();

	std::string filename;
	bool file_exists(const std::string & test_file) const;
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
