#include "cache4.cc"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

Cache::index_type space_used_test(Cache* c) {
	return c->space_used();
}

void set_test(Cache* c, Cache::key_type key, Cache::val_type val, Cache::index_type size) {
	c->set(key, val, size);
}

Cache::val_type get_test(Cache* c, Cache::key_type key, Cache::index_type& val_size) {
	return c->get(key, val_size);
}

void del_test(Cache* c, Cache::key_type key) {
	c->del(key);
}

int hasher(std::string) {
	return 0;
}

class FIFOEvictor {
private:
	// eviction queue
	std::vector<std::string> eviction_queue;

public:
	void add_element(Cache::key_type key) {
		this->eviction_queue.push_back(key);
	}

	Cache::key_type operator()() {
		return this->eviction_queue.back();
	}

};




int main() {
	// initialize Cache obj 'c'
	Cache* c = new Cache(2);

	// test set
	cout << "\ninserting 'key':val int x (21)\n";
	int x = 21;
	set_test(c, "key", &x, space_used_test(c));

	// test space_used
	cout << "space used after 1 insert: " << space_used_test(c) << '\n';

	// test get present element
	Cache::index_type size = space_used_test(c);
	Cache::val_type val_addr = get_test(c, "key", size);
	cout << "address of val at key: " << val_addr << '\n';

	// test set-overwrite
	cout << "\ninserting 'key':val int x (16) [should overwrite]\n";
	x = 16;
	set_test(c, "key", &x, space_used_test(c));
	// see if it overwrote
	size = space_used_test(c);
	cout << "looking up 'key': ";
	val_addr = get_test(c, "key", size);
	cout << val_addr << '\n';
	//size should still be 1
	cout << "size should still be 1: " << space_used_test(c) << '\n';

	//test get an absent element
	cout << "\nlooking up absent element 'keyAbsent': ";
	val_addr = get_test(c, "keyAbsent", size);
	cout << val_addr << '\n';

	//test del
	cout << "\ndeleting 'key'\n";
	del_test(c, "key");
	//see if del worked
	cout << "looking up 'key': ";
	size = space_used_test(c);
	val_addr = get_test(c, "key", size);
	cout << val_addr << '\n';
	//ensure memused is updated
	cout << "space used should be 0: " << space_used_test(c) << '\n';

	//test evict (add 3 el.s, check that 1 gets evicted)
	cout << "\ntesting evict, adding 3 el.s to a map w max_load 2...\n";
	x=17;
	set_test(c, "key2", &x, space_used_test(c));
	int y = 18;
	set_test(c, "key3", &y, space_used_test(c));
	cout << "\n inserting 3rd element...\n";
	int z = 19;
	set_test(c, "key4", &z, space_used_test(c));
	// size should be 2 not 3
	cout << "map size is: ";
	cout << space_used_test(c) << '\n';


	cout << "\ncreating Cache w max load factor 2 and custom evictor/hasher...\n";
	//Cache::evictor_type FIFOEvictor_inst;
	Cache* customCache = new Cache(2, [](){ return 0; }, hasher);

	cout << "inserting 3 el.s (1 should get evicted)\n";
	x=31;
	set_test(customCache, "ckey1", &x, space_used_test(c));
	y = 32;
	set_test(customCache, "ckey2", &y, space_used_test(c));
	cout << "\n inserting 3rd element...\n";
	z = 33;
	set_test(customCache, "ckey3", &z, space_used_test(c));

	cout << "new length is: ";
	cout << space_used_test(customCache) << '\n';



}

