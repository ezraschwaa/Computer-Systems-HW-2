#include "cache.cc"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

Cache::index_type space_used_test(Cache* c) {
	Cache::index_type used_space = c->space_used();
	cout << "space used: " << used_space << "\n";
	return used_space;
}

void set_test(Cache* c, Cache::key_type key, int* val, Cache::index_type size) {
	cout << "setting Cache['" << key << "'] = ptr " << val << "->" << *val << "\n";
	c->set(key, val, size);
}

Cache::val_type get_test(Cache* c, Cache::key_type key, Cache::index_type& val_size) {
	Cache::val_type value = c->get(key, val_size);
	cout << "getting Cache['" << key << "']: ";
	int* data_at_val = new int[1];
	if(value!=nullptr) {
		memcpy(data_at_val, value, val_size+1);
		cout << data_at_val << "->" << *data_at_val;
	} else {
		cout << key << " is absent\n";
	}
	cout << "\n";
	return value;
}

void del_test(Cache* c, Cache::key_type key) {
	cout << "deleting Cache['" << key << "']...\n";
	c->del(key);
}

int hasher(std::string) {
	return 0;
}




int main() {
	// initialize Cache obj 'c'
	Cache* c = new Cache(2);

	Cache::index_type size = sizeof(Cache::index_type);

	space_used_test(c);

	// test set
	int x = 21;
	set_test(c, "key", &x, size);
	
	// test space_used
	space_used_test(c);

	// test get present element
	get_test(c, "key", size);

	// test set-overwrite
	x = 16;
	set_test(c, "key", &x, space_used_test(c));
	// see if it overwrote
	space_used_test(c);
	get_test(c, "key", size);
	//size should still be 1
	cout << "size should still be 1: " << space_used_test(c) << '\n';

	//test get an absent element
	get_test(c, "keyAbsent", size);

	//test del
	del_test(c, "key");
	//see if del worked
	space_used_test(c);
	get_test(c, "key", size);
	//ensure memused is updated
	cout << "space used should be 0: " << space_used_test(c) << '\n';

	//test evict (add 3 el.s, check that 1 gets evicted)
	cout << "\ntesting evict, adding 3 el.s to a map w max_load 2...\n";
	x=17;
	set_test(c, "key2", &x, space_used_test(c));
	int y = 18;
	set_test(c, "key3", &y, space_used_test(c));
	int z = 19;
	set_test(c, "key4", &z, space_used_test(c));
	// size should be 2 not 3
	cout << "map size should be 2: ";
	space_used_test(c);
	cout << '\n';

	get_test(c, "key", size);
	get_test(c, "key2", size);
	get_test(c, "key3", size);
	get_test(c, "key4", size);	

	free(c);


	cout << "\ncreating Cache w max load factor 2 and custom hasher...\n";
	Cache* customCache = new Cache(2, [](){ return 0; }, hasher);

	cout << "inserting 4 el.s (2 should get evicted)\n";
	x=31;
	set_test(customCache, "ckey1", &x, space_used_test(customCache));
	get_test(customCache, "ckey1", size);
	set_test(customCache, "ckey1", &x, space_used_test(customCache));
	get_test(customCache, "ckey1", size);

	y = 32;
	set_test(customCache, "ckey2", &y, space_used_test(customCache));
	z = 33;
	set_test(customCache, "ckey3", &z, space_used_test(customCache));
	int n = 34;
	set_test(customCache, "ckey3", &n, space_used_test(customCache));

	cout << "map size should be 2: ";
	size = space_used_test(customCache);
	cout << size << '\n';

	get_test(customCache, "ckey1", size);
	get_test(customCache, "ckey2", size);
	get_test(customCache, "ckey3", size);

	free(customCache);

}

