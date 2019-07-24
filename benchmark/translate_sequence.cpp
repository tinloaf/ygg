#include "../src/benchmark_sequence.hpp"

#include <ostream>
#include <type_traits>
#include <unordered_map>

int
main(int argc, char ** argv)
{
	if (argc != 3) {
		return -1;
	}

	using BSS = ygg::utilities::BenchmarkSequenceStorage<unsigned int>;
	auto reader = BSS::Reader(argv[1]);
	auto writer = BSS(argv[2]);

	std::unordered_map<const void *, size_t> node_map;

	for (auto * buf = &reader.get(10000); buf->size() > 0;
	     buf = &reader.get(10000)) {
		std::cout << "Got " << buf->size() << " entries.\n";
		for (auto & entry : *buf) {
			const void * id = entry.id;
			const void * translated_id;

			switch (entry.type) {
			case BSS::Type::INSERT:
				if (node_map.find(id) == node_map.end()) {
					node_map.emplace(std::make_pair(id, node_map.size() + 1));
				}
				break;
			case BSS::Type::ERASE:
			case BSS::Type::DELETE:
			case BSS::Type::SEARCH:
			case BSS::Type::LBOUND:
			case BSS::Type::UBOUND:
			default:
				break;
			}

			if (node_map.find(id) != node_map.end()) {
				translated_id = reinterpret_cast<const void *>(node_map[id]);
			} else {
				translated_id = reinterpret_cast<const void *>(0);
			}

			switch (entry.type) {
			case BSS::Type::INSERT:
				writer.register_insert(translated_id, entry.key);
				break;
			case BSS::Type::ERASE:
				writer.register_erase(translated_id, entry.key);
				break;
			case BSS::Type::DELETE:
				writer.register_delete(translated_id, entry.key);
				break;
			case BSS::Type::SEARCH:
				writer.register_search(translated_id, entry.key);
				break;
			case BSS::Type::LBOUND:
				writer.register_lbound(translated_id, entry.key);
				break;
			case BSS::Type::UBOUND:
				writer.register_ubound(translated_id, entry.key);
				break;
			default:
				break;
			}
		}
	}
}
