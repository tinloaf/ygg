#ifndef YGG_BENCHMARK_SEQUENCE_CPP
#define YGG_BENCHMARK_SEQUENCE_CPP

#include "benchmark_sequence.hpp"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace ygg {
namespace utilities {

template <class KeyT>
BenchmarkSequenceStorage<KeyT>::Reader::Reader(std::string filename)
    : infile(filename, std::ios::binary), remaining_in_chunk(0)
{
	if (!this->infile.good()) {
		std::cout << "==== Could not open " << filename << "!\n";
		exit(-1);
	}

	this->verify_type();
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::Reader::verify_type()
{
	TypeInfo ti{
	    1,
	    sizeof(KeyT),
	    std::is_integral_v<KeyT>,
	    std::is_floating_point_v<KeyT>,
	    std::is_signed_v<KeyT>,
	};

	if (!this->infile.good()) {
		std::cout << "==== I/O error!\n";
		exit(-1);
	}

	TypeInfo read_ti;
	this->infile.read(reinterpret_cast<char *>(&read_ti), sizeof(read_ti));

	if (ti != read_ti) {
		throw WrongTypeException{};
	}
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::Reader::reset()
{

	this->infile.clear();
	this->remaining_in_chunk = 0;
	this->buf.clear();
	this->infile.seekg(sizeof(TypeInfo), std::ios::beg);
}

template <class KeyT>
const std::vector<typename BenchmarkSequenceStorage<KeyT>::Entry> &
BenchmarkSequenceStorage<KeyT>::Reader::get(size_t count)
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

template <class KeyT>
std::string
BenchmarkSequenceStorage<KeyT>::get_filename() const
{
	const char * hint = std::getenv("YGG_SEQUENCE_PREFIX");
	if (hint != nullptr) {
		bool found = false;
		size_t i = 0;

		std::ostringstream filename_buf;
		std::filesystem::path path;
		while (!found) {
			filename_buf.clear();
			filename_buf << hint << std::hex << (this) << "_" << std::dec << i
			             << ".bin";
			path = std::filesystem::path(filename_buf.str());
			if (!std::filesystem::exists(path)) {
				found = true;
			}
		}

		return path.string();
	} else {
		auto tmp_path = std::filesystem::temp_directory_path();
		std::filesystem::path full_path;

		std::ostringstream filename_buf;

		bool found = false;
		size_t i = 0;
		while (!found) {
			filename_buf.clear();
			filename_buf << "tree_" << std::hex << (this) << "_" << std::dec << i
			             << ".bin";
			full_path = tmp_path / filename_buf.str();

			if (!std::filesystem::exists(full_path)) {
				found = true;
			}
		}

		return full_path.string();
	}
}

template <class KeyT>
BenchmarkSequenceStorage<KeyT>::BenchmarkSequenceStorage()
    : filename(get_filename()), outfile(filename)
{
	this->buf.reserve(BenchmarkSequenceStorage<KeyT>::BUFSIZE);

	std::cout << "Writing tree operations to " << this->filename << std::endl;

	this->write_typeinfo();
}

template <class KeyT>
BenchmarkSequenceStorage<KeyT>::BenchmarkSequenceStorage(
    std::string filename_in)
    : filename(filename_in), outfile(filename)
{
	this->buf.reserve(BenchmarkSequenceStorage<KeyT>::BUFSIZE);

	std::cout << "Writing tree operations to " << this->filename << std::endl;

	this->write_typeinfo();
}

template <class KeyT>
BenchmarkSequenceStorage<KeyT>::~BenchmarkSequenceStorage()
{
	this->sync();
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::write_typeinfo()
{
	TypeInfo ti{
	    1,
	    sizeof(KeyT),
	    std::is_integral_v<KeyT>,
	    std::is_floating_point_v<KeyT>,
	    std::is_signed_v<KeyT>,
	};

	this->outfile.write(reinterpret_cast<char const *>(&ti), sizeof(ti));
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::sync()
{
	// write number of records
	size_t count = this->buf.size();
	this->outfile.write(reinterpret_cast<char const *>(&count), sizeof(size_t));

	// Write the data
	this->outfile.write(reinterpret_cast<char const *>(this->buf.data()),
	                    static_cast<std::streamsize>(sizeof(Entry) * count));

	this->buf.clear();
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::register_insert(const void * id,
                                                const KeyT & key)
{
	this->buf.emplace_back(Type::INSERT, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::register_erase(const void * id,
                                               const KeyT & key)
{
	this->buf.emplace_back(Type::ERASE, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::register_search(const void * id,
                                                const KeyT & key)
{
	this->buf.emplace_back(Type::SEARCH, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::register_delete(const void * id,
                                                const KeyT & key)
{
	this->buf.emplace_back(Type::DELETE, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::register_ubound(const void * id,
                                                const KeyT & key)
{
	this->buf.emplace_back(Type::UBOUND, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

template <class KeyT>
void
BenchmarkSequenceStorage<KeyT>::register_lbound(const void * id,
                                                const KeyT & key)
{
	this->buf.emplace_back(Type::LBOUND, key, id);
	if (this->buf.size() > BUFSIZE) {
		this->sync();
	}
}

} // namespace utilities
} // namespace ygg
#endif
