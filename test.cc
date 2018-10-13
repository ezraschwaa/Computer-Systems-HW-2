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

// sets Cache[key] = val
void set_test(Cache* c, Cache::key_type key, int* val, Cache::index_type size) {
	cout << "setting Cache['" << key << "'] = ptr " << val << "->" << *val << "\n";
	c->set(key, val, size);
}


// prints value at key
void get_test(Cache* c, Cache::key_type key, Cache::index_type& val_size) {
	cout << "getting Cache['" << key << "']: ";
	Cache::val_type value = c->get(key, val_size);
	int* data_at_val = new int[1];
	if(value!=nullptr) {
		memcpy(data_at_val, value, val_size+1);
		cout << data_at_val << "->" << *data_at_val;
	}
	cout << "\n";
	free(data_at_val);
}

// deletes Cache[key]
void del_test(Cache* c, Cache::key_type key) {
	cout << "deleting Cache['" << key << "']...\n";
	c->del(key);
}

// simple/stupid custom hasher
int hasher(std::string) {
	return 0;
}




int main() {
	// initialize Cache obj 'c'
	Cache* c = new Cache(2);

	space_used_test(c);

	Cache::index_type size = sizeof(Cache::index_type);

	int x = 21;
	set_test(c, "key", &x, size);
	space_used_test(c);
	// get present element
	get_test(c, "key", size);
	cout << '\n';

	// overwrite present element
	int y = 16;
	set_test(c, "key", &y, size);
	// see if it overwrote
	get_test(c, "key", size);
	//size should still be 1
	space_used_test(c);
	cout << '\n';

	//test get/del an absent element
	get_test(c, "keyAbsent", size);
	del_test(c, "keyAbsent");
	space_used_test(c);

	//test del present element
	get_test(c, "key", size);
	del_test(c, "key");
	//see if del worked
	space_used_test(c);
	get_test(c, "key", size);
	

	//test evict (add 3 el.s, check that 1 gets evicted)
	cout << "testing evict, adding 3 el.s to a map w max_load 2...\n";
	x=17;
	set_test(c, "key", &x, size);
	y = 18;
	set_test(c, "key1", &y, size);
	x = 19;
	set_test(c, "key2", &x, size);
	// size should be 2 not 3
	cout << "map size should be 2: ";
	space_used_test(c);
	cout << '\n';

	get_test(c, "key", size);
	get_test(c, "key1", size);
	get_test(c, "key2", size);	

	free(c);


	cout << "\ncreating Cache w max load factor 2 and custom hasher...\n";
	Cache* customCache = new Cache(2, [](){ return 0; }, hasher);

	cout << "inserting 4 el.s (2 should get evicted)\n";
	x=31;
	set_test(customCache, "ckey1", &x, size);
	y = 32;
	set_test(customCache, "ckey2", &y, size);
	x = 33;
	set_test(customCache, "ckey3", &x, size);
	y = 34;
	set_test(customCache, "ckey3", &y, size);

	space_used_test(customCache);
	cout << '\n';

	get_test(customCache, "ckey1", size);
	get_test(customCache, "ckey2", size);
	get_test(customCache, "ckey3", size);

	free(customCache);

}
