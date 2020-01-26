#ifndef YGG_BENCHMARK_SEQUENCE_CPP
#define YGG_BENCHMARK_SEQUENCE_CPP

#include "benchmark_sequence.hpp"

#include <iostream>
#include <sstream>
#include <type_traits>

namespace ygg {
namespace utilities {

template <class KeyT, class SearchKeyT, class ValueT>
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::Reader::Reader(
    std::string filename)
    : infile(filename, std::ios::binary), remaining_in_chunk(0)
{
	if (!this->infile.good()) {
		std::cout << "==== Could not open " << filename << "!\n";
		exit(-1);
	}

	this->verify_type();
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::Reader::verify_type()
{
	TypeInfo key_ti{2,
	                sizeof(KeyT),
	                std::is_integral_v<KeyT>,
	                std::is_floating_point_v<KeyT>,
	                std::is_signed_v<KeyT>,
	                true};

	if (!this->infile.good()) {
		std::cout << "==== I/O error!\n";
		exit(-1);
	}

	TypeInfo read_key_ti;
	this->infile.read(reinterpret_cast<char *>(&read_key_ti),
	                  sizeof(read_key_ti));

	if (key_ti != read_key_ti) {
		throw WrongTypeException{};
	}

	TypeInfo search_key_ti{2,
	                       sizeof(SearchKeyT),
	                       std::is_integral_v<SearchKeyT>,
	                       std::is_floating_point_v<SearchKeyT>,
	                       std::is_signed_v<SearchKeyT>,
	                       true};

	if (!this->infile.good()) {
		std::cout << "==== I/O error!\n";
		exit(-1);
	}

	TypeInfo read_search_key_ti;
	this->infile.read(reinterpret_cast<char *>(&read_search_key_ti),
	                  sizeof(read_search_key_ti));

	if (search_key_ti != read_search_key_ti) {
		throw WrongTypeException{};
	}

	TypeInfo value_ti{2,
	                  sizeof(ValueT),
	                  std::is_integral_v<ValueT>,
	                  std::is_floating_point_v<ValueT>,
	                  std::is_signed_v<ValueT>,
	                  !std::is_same_v<ValueT, void>};

	TypeInfo read_value_ti;
	this->infile.read(reinterpret_cast<char *>(&read_value_ti),
	                  sizeof(read_value_ti));

	if (value_ti != read_value_ti) {
		throw WrongTypeException{};
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::Reader::reset()
{

	this->infile.clear();
	this->remaining_in_chunk = 0;
	this->buf.clear();
	this->infile.seekg(3 * sizeof(TypeInfo), std::ios::beg);
}

template <class KeyT, class SearchKeyT, class ValueT>
const std::vector<
    typename BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::Entry> &
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::Reader::get(size_t count)
{
	size_t assembled = 0;
	this->buf.resize(count);

	while (assembled < count) {
		if (this->remaining_in_chunk > 0) {
			size_t read_count =
			    std::min(this->remaining_in_chunk, (count - assembled));
			this->infile.read(
			    reinterpret_cast<char *>(this->buf.data() + assembled),
			    static_cast<std::streamsize>(read_count * sizeof(Entry)));
			assembled += read_count;
			this->remaining_in_chunk -= read_count;
		} else {
			// Open next chunk
			size_t chunksize;
			this->infile.read(reinterpret_cast<char *>(&chunksize), sizeof(size_t));
			if (this->infile.eof()) {
				this->buf.resize(assembled);
				return this->buf;
			}
			this->remaining_in_chunk = chunksize;
		}
	}

	return this->buf;
}

template <class KeyT, class SearchKeyT, class ValueT>
bool
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::file_exists(
    const std::string & test_file) const
{
	if (auto * file = fopen(test_file.c_str(), "r")) {
		return true;
		fclose(file);
	} else {
		return false;
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
std::string
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::get_filename() const
{
	const char * hint = std::getenv("YGG_SEQUENCE_PREFIX");
	if (hint != nullptr) {
		bool found = false;
		size_t i = 0;

		std::ostringstream filename_buf;
		while (!found) {
			filename_buf.str("");
			filename_buf.clear();
			filename_buf << hint << std::hex << (this) << "_" << std::dec << i
			             << ".bin";
			if (!this->file_exists(filename_buf.str())) {
				found = true;
			}
		}

		return filename_buf.str();
	} else {
		// This is the way to get a temp directory in POSIX
		const char * tmpdir = std::getenv("TMPDIR");
		if (tmpdir == nullptr) {
			// The system seems to ignore POSIX? Use '/tmp' and hope for the best.
			tmpdir = "/tmp";
		}

		std::ostringstream filename_buf;

		bool found = false;
		size_t i = 0;
		while (!found) {
			filename_buf.str("");
			filename_buf.clear();
			filename_buf << tmpdir;
#ifdef _WIN32
			filename_buf << '\\';
#else
			filename_buf << '/';
#endif

			filename_buf << "tree_" << std::hex << (this) << "_" << std::dec << i
			             << ".bin";
			if (!this->file_exists(filename_buf.str())) {
				found = true;
			}
		}

		return filename_buf.str();
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::BenchmarkSequenceStorage()
    : filename(get_filename()), outfile(filename)
{
	this->buf.reserve(
	    BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::BUFSIZE);

	std::cout << "Writing tree operations to " << this->filename << std::endl;

	this->write_typeinfo();
}

template <class KeyT, class SearchKeyT, class ValueT>
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::BenchmarkSequenceStorage(
    std::string filename_in)
    : filename(filename_in), outfile(filename)
{
	this->buf.reserve(
	    BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::BUFSIZE);

	std::cout << "Writing tree operations to " << this->filename << std::endl;

	this->write_typeinfo();
}

template <class KeyT, class SearchKeyT, class ValueT>
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::~BenchmarkSequenceStorage()
{
	this->sync();
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::write_typeinfo()
{
	TypeInfo key_ti{2,
	                sizeof(KeyT),
	                std::is_integral_v<KeyT>,
	                std::is_floating_point_v<KeyT>,
	                std::is_signed_v<KeyT>,
	                true};

	this->outfile.write(reinterpret_cast<char const *>(&key_ti), sizeof(key_ti));

	TypeInfo search_key_ti{2,
	                       sizeof(SearchKeyT),
	                       std::is_integral_v<SearchKeyT>,
	                       std::is_floating_point_v<SearchKeyT>,
	                       std::is_signed_v<SearchKeyT>,
	                       true};

	this->outfile.write(reinterpret_cast<char const *>(&search_key_ti),
	                    sizeof(search_key_ti));

	TypeInfo val_ti{2,
	                sizeof(ValueT),
	                std::is_integral_v<ValueT>,
	                std::is_floating_point_v<ValueT>,
	                std::is_signed_v<ValueT>,
	                has_value};

	this->outfile.write(reinterpret_cast<char const *>(&val_ti), sizeof(val_ti));
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::sync()
{
	// write number of records
	size_t count = this->buf.size();
	this->outfile.write(reinterpret_cast<char const *>(&count), sizeof(size_t));

	// Write the data
	this->outfile.write(reinterpret_cast<char const *>(this->buf.data()),
	                    static_cast<std::streamsize>(sizeof(Entry) * count));

	this->buf.clear();
}

template <class KeyT, class SearchKeyT, class ValueT>
template <class Dummy>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::register_insert(
    const void * id, const std::enable_if_t<!has_value, Dummy> & key)
{
	this->buf.emplace_back(Type::INSERT, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
template <class Dummy>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::register_insert(
    const void * id, const KeyT & key,
    const std::enable_if_t<has_value, Dummy> & value)
{
	this->buf.emplace_back(Type::INSERT, key, value, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::register_erase(
    const void * id, const KeyT & key)
{
	this->buf.emplace_back(Type::ERASE, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::register_search(
    const void * id, const SearchKeyT & key)
{
	this->buf.emplace_back(Type::SEARCH, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::register_delete(
    const void * id, const KeyT & key)
{
	this->buf.emplace_back(Type::DELETE, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::register_ubound(
    const void * id, const SearchKeyT & key)
{
	this->buf.emplace_back(Type::UBOUND, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT, class SearchKeyT, class ValueT>
void
BenchmarkSequenceStorage<KeyT, SearchKeyT, ValueT>::register_lbound(
    const void * id, const SearchKeyT & key)
{
	this->buf.emplace_back(Type::LBOUND, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

} // namespace utilities
} // namespace ygg
#endif
