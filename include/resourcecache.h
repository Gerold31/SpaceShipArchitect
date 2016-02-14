#ifndef RESOURCECACHE_H
#define RESOURCECACHE_H

#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>


template<class T, class L, T(L::*Func)(const std::string&) const>
class ResourceCache
{
public:
	ResourceCache(const L *loader) :
		mLoader(loader)
	{}
	~ResourceCache() {
		assert(mMap.empty());
	}

	std::shared_ptr<T> get(const std::string &name) {
		std::shared_ptr<T> result;
		std::lock_guard<std::mutex> lock(mMutex);

		auto it = mMap.find(name);
		if (it == mMap.end() || (result = it->second.lock()) == nullptr) {
			result = std::shared_ptr<T>(new T((mLoader->*Func)(name)), [this,name](T*){
				std::lock_guard<std::mutex> lock(mMutex);
				mMap.erase(name);
			});
			mMap[name] = result;
		}
		return result;
	}

private:
	const L * mLoader;
	std::mutex mMutex;
	std::unordered_map<std::string,std::weak_ptr<T>> mMap;

};

#endif // RESOURCECACHE_H
