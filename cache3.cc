#include <stdio.h>
#include <string.h>
#include "cache.hh"
#include <unordered_map>

struct Cache::Impl {


	/*std::unordered_map<std::string, void*, hash_type> hashtable_(0, hasher_); 
	hash_type hasher; [hasher is a functor]

	hash_vale = hasher();


	std::hash<std::string> h; [is default hasher by unordered map]

	h();



	*/
	std::unordered_map<std::string, void*> hashtable_;
	index_type maxmem_;
	evictor_type evictor_;
	hash_func hasher_;
	index_type memused_;

	//
	Impl(index_type maxmem, evictor_type evictor, hash_func hasher) : 
	maxmem_(maxmem), evictor_(evictor), hasher_(hasher), memused_(0)
	{
		//hashtable_(0 [([[[bucket count])])], hasher);
    }


    ~Impl() = default;

	void set(key_type key, val_type val, index_type size) {
		if(memused_ >= maxmem_) {
			hashtable_.erase(hashtable_.begin());
			memused_ -= 1;
		}
		void* newval = new char[size];
		memcpy(newval, val, size);
		if(hashtable_.find(key)!=hashtable_.end()) {
			memused_ -= 1;
		}
		hashtable_[key] = newval;
		memused_ += 1;
	}


	val_type get(key_type key, index_type& val_size) const {
		if(hashtable_.find(key)!=hashtable_.end()) {
			//const val_type val = hashtable_[key]
			return hashtable_.find(key)->second;

		} else {
			return nullptr;
		}
	}

	void del(key_type key) {
		hashtable_.erase(key);
		memused_ -= 1;
	}

	index_type space_used() const {
		return memused_;
	}
};

Cache::Cache(index_type maxmem,
    evictor_type evictor,
    hash_func hasher)
	: pImpl_(new Impl(maxmem, evictor, hasher)) {
}

Cache::~Cache() {

}


// Add a <key, value> pair to the cache.
// If key already exists, it will overwrite the old value.
// Both the key and the value are to be deep-copied (not just pointer copied).
// If maxmem capacity is exceeded, sufficient values will be removed
// from the cache to accomodate the new value.
void Cache::set(key_type key, val_type val, index_type size) {
	pImpl_->set(key, val, size);
}

// Retrieve a pointer to the value associated with key in the cache,
// or NULL if not found.
Cache::val_type Cache::get(key_type key, index_type& val_size) const {
	return pImpl_->get(key, val_size);
}

// Delete an object from the cache, if it's still there
void Cache::del(key_type key) {
	pImpl_->del(key);
}

// Compute the total amount of memory used up by all cache values (not keys)
Cache::index_type Cache::space_used() const {
	return pImpl_->space_used();
}
