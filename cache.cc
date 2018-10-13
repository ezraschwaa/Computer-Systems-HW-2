#include <stdio.h>
#include <string.h>
#include "cache.hh"
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

// this is a functor
class FifoEvictor {
private:
	std::vector<std::string> eviction_queue_;
public:
	string operator()() {
		string next_evict;
		if(this->eviction_queue_.size()>0) {
			next_evict = this->eviction_queue_[0];
			this->remove(next_evict);
		} else {
			cout << this->eviction_queue_.size() << "nothing to evict\n";
		}
		return next_evict;
	}

	void add(string key) {
		this->eviction_queue_.push_back(key);
	}

	void remove(string key) {
		// erase-remove - v.erase( std::remove( v.begin(), v.end(), 5 ), v.end() );
		this->eviction_queue_.erase(std::remove(this->eviction_queue_.begin(), this->eviction_queue_.end(), key), this->eviction_queue_.end());
	}
};



struct Cache::Impl {



	index_type maxmem_;
	evictor_type evictor_;
	hash_func hasher_;
	index_type memused_;
	FifoEvictor Fifo_;

	std::unordered_map<std::string, void*, hash_func> hashtable_;


	Impl(index_type maxmem, evictor_type evictor, hash_func hasher)
	: 
	maxmem_(maxmem), evictor_(evictor_), hasher_(hasher), memused_(0), hashtable_(0 , hasher_), Fifo_()

	{
		hashtable_.max_load_factor(0.5);
    }


    ~Impl() = default;

	void set(key_type key, val_type val, index_type size) {
		// if the key is already in the table...
		if(hashtable_.find(key)!=hashtable_.end()) {
			// remove it from queue (will overwrite it in cache/re-add it to queue later)
			Fifo_.remove(key);
			memused_ -= 1;
		} else if(memused_ >= maxmem_) {
			// pop off next_evict (also del.s it from FIFO queue)
			key_type next_evict = Fifo_();
			// erase it from cache
			hashtable_.erase(next_evict);
			memused_ -= 1;
		}
		void* newval = new char[size];
		memcpy(newval, val, size);
		hashtable_[key] = newval;
		memused_ += 1;
		Fifo_.add(key);
	}


	val_type get(key_type key, index_type& val_size) const {
		if(hashtable_.find(key)!=hashtable_.end()) {
			return hashtable_.find(key)->second;

		} else {
			return nullptr;
		}
	}

	void del(key_type key) {
		hashtable_.erase(key);
		Fifo_.remove(key);
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
