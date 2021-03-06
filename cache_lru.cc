// by Joe Meyer

#include <stdio.h>
#include <string.h>
#include "cache.hh"
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <assert.h>

using namespace std;

// this is a functor (implements LRU)
// if all val.s are same size it acts as Fifo
class LruEvictor {
private:
	vector<tuple<uint32_t, string> > eviction_queue_;
	// eviction_queue_ holds nodes of form (val-size, key)
public:
	// returns next key to evict, also removes it from ev. q
	tuple<uint32_t,string> operator()() {
		tuple<uint32_t,string> next_evict;
		// LruEvictor() is never called on an empty eviction_queue
		assert(this->eviction_queue_.size()>0 && "nothing to evict\n");
		next_evict = this->eviction_queue_[0];
		string next_evict_key = get<1>(next_evict);
		cout << "evicting '" << next_evict_key << "'\n";
		this->remove(next_evict_key);
		return next_evict;
	}

	// add an element to eviction queue
	void add(uint32_t elt_size, string key) {
		uint32_t evq_size = this->eviction_queue_.size();
		tuple<uint32_t, string> node = make_tuple(elt_size, key);
		uint32_t i = 0;
		for(;i<evq_size; i++) {
			uint32_t i_size = get<0>(this->eviction_queue_[i]);
			if(elt_size>i_size) {
				this->eviction_queue_.insert(this->eviction_queue_.begin()+i, node);
			}
		}
		if(i>=evq_size) {
			this->eviction_queue_.push_back(node);
		}
	}

	// remove an item from ev. q.
	void remove(string key) {
		// erase-remove_if idiom
		this->eviction_queue_.erase(std::remove_if(this->eviction_queue_.begin(), this->eviction_queue_.end(), [key](tuple<uint32_t, string> node){return get<1>(node)==key;}), this->eviction_queue_.end());
	}

	// get size of key's val
	uint32_t getsize(string key) {
		uint32_t i = 0;
		for(;i<this->eviction_queue_.size(); i++) {
			tuple<uint32_t, string> node = this->eviction_queue_[i];
			if(get<1>(node) == key) {
				return get<0>(node);
			}
		}
		assert(false && "key not found\n");
		return 0;
	}
};



// These funcs are necessary bc mandatory interface overwrites
//		``get" func required for tuple element access in Cache class
string get_tuple_key(tuple<uint32_t, string> node) {
	return get<1>(node);
}

uint32_t get_tuple_size(tuple<uint32_t, string> node) {
	return get<0>(node);
}


struct Cache::Impl {



	index_type maxmem_;
	evictor_type evictor_;
	hash_func hasher_;
	index_type memused_;
	LruEvictor Lru_;

	std::unordered_map<std::string, void*, hash_func> hashtable_;


	Impl(index_type maxmem, evictor_type evictor, hash_func hasher)
	: 
	maxmem_(maxmem), evictor_(evictor_), hasher_(hasher), memused_(0), hashtable_(0 , hasher_), Lru_()

	{
		assert(maxmem_>0 && "Cache size must be positive");
		hashtable_.max_load_factor(0.5);
    }


    ~Impl() = default;

	void set(key_type key, val_type val, index_type size) {
		// if the key is already in the table...
		if(hashtable_.find(key)!=hashtable_.end()) {
			// remove it from queue (will overwrite it in cache/re-add it to queue later)
			cout << "overwriting key: '" << key << "'\n";
			free(hashtable_[key]);
			memused_ -= Lru_.getsize(key);
			Lru_.remove(key);
		} else if(memused_ >= maxmem_) {
			// get next_evict (also del.s it from ev. q.)
			tuple<uint32_t, string> next_evict = Lru_();
			string next_evict_key = get_tuple_key(next_evict);
			uint32_t next_evict_size = get_tuple_size(next_evict);
			memused_ -= next_evict_size;
			hashtable_.erase(next_evict_key);
		}
		void* newval = new char[size];
		memcpy(newval, val, size);
		hashtable_[key] = newval;
		memused_ += size;
		Lru_.add(size, key);
	}

	// returns cache[key]
	val_type get(key_type key, index_type& val_size) const {
		if(hashtable_.find(key)!=hashtable_.end()) {
			return hashtable_.find(key)->second;

		} else {
			cout << "key '" << key << "' is absent\n";
			return nullptr;
		}
	}

	// removes key:val from cache
	void del(key_type key) {
		if(hashtable_.find(key)!=hashtable_.end()) {
			free(hashtable_[key]);
			hashtable_.erase(key);
			memused_ -= Lru_.getsize(key);
			Lru_.remove(key);
		} else {
			cout << "key '" << key << "' is already absent\n";
		}
	}

	// returns num of bytes used by cached values
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
