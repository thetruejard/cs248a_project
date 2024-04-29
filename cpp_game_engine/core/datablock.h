#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>


/*
* Datablocks are a standard way of managing the (de)allocation of various internal game
* engine types, such as GameObjects, Meshes, Textures, etc. The DatablockManager class
* manages the memory of these datablocks, including their construction/destruction, and
* all datablocks are classes that derive from the Datablock abstract class.
* 
* All datablock types must satisfy the following conditions:
*	1. They must derive from the Datablock class.
*	2. The FIRST argument of all constructors must be a DatablockID type, or the ID type
		corresponding to the given datablock type (i.e. defined by DATABLOCK_ID).
*	3. All constructors must forward the ID passed as the first argument to the Datablock
*		constructor.
*
* All datablocks are reference counted. There are 3 primary ways to reference datablocks:
*	1. Ref<Type> - A counted reference. As long as this reference exists, the datablock
*		is guaranteed to still exist.
*	2. WeakRef<Type> - An uncounted reference. The datablock may be deleted while this
*		reference still exists, in which case the WeakRef behaves similarly to a null
*		reference. These references can be temporarily elevated to counted references
*		for ease of access.
*	3. Direct pointers - Recommended for function arguments when a Ref is known to exist.
*		Note that the existence of a WeakRef is insufficient to guarantee the safety of
*		a direct pointer; only a valid Ref can guarantee the existence of the underlying
*		datablock.
* In addition to the above smart references, direct pointers to the underlying datablock
* can be used. This should only be used when a counted reference is known to exist.
* 
* Within the game engine code, the specific usages of reference-counted Refs are all
* well-defined. See the respective header files of each datablock type for how these
* Refs are used. For example, see objects/gameobject.h for a list of all internal uses
* of Ref<GameObject>.
*/


template<typename Type>
class DatablockManager;


#define DATABLOCK_ID(_class)				\
	using _class##ID = DatablockID;			\
	constexpr _class##ID _class##Null = 0;

using DatablockID = uint64_t;
constexpr DatablockID DatablockNull = 0;

template<typename Type>
class WeakRef;

class Datablock;



template<typename Type>
class Ref {
public:

	Ref() {}
	Ref(std::nullptr_t) {}
	Ref(const Ref<Type>& ref) { *this = ref; }
	Ref(Ref<Type>&& ref) { *this = ref; }
	template<typename FromType>
	Ref(const Ref<FromType>& ref) { *this = ref; }
	template<typename FromType>
	Ref(Ref<FromType>&& ref) { *this = ref; }

	Ref<Type>& operator=(const Ref<Type>& ref) {
		this->datablock = ref.datablock;
		return *this;
	}
	Ref<Type>& operator=(Ref<Type>&& ref) {
		this->datablock = std::move(ref.datablock);
		return *this;
	}
	template<typename FromType>
	Ref<Type>& operator=(const Ref<FromType>& ref) {
		return *this = ref.cast<Type>();
	}
	template<typename FromType>
	Ref<Type>& operator=(Ref<FromType>&& ref) {
		return *this = std::move(ref.cast<Type>());
	}

	Type* operator->() const {
		return this->get();
	}

	Type* get() const {
		return this->datablock.get();
	}

	operator bool() const {
		return this->datablock != nullptr;
	}

	template<typename CmpType>
	bool operator==(const Ref<CmpType>& r) const {
		return this->get() == r.get();
	}
	bool operator==(std::nullptr_t) const {
		return !bool(*this);
	}
	template<typename CmpType>
	bool operator!=(const Ref<CmpType>& r) const {
		return this->get() != r.get();
	}
	bool operator!=(std::nullptr_t) const {
		return bool(*this);
	}

	template<typename ToType>
	Ref<ToType> cast() const {
		static_assert(std::is_base_of_v<Type, ToType> || std::is_base_of_v<ToType, Type>,
			"datablock.h: Ref::cast(): One of either Type or ToType must derive from the other"
		);
		Ref<ToType> r;
		r.datablock = std::static_pointer_cast<ToType>(this->datablock);
		return r;
	}

	Ref<Type>&& move() {
		return std::move(*this);
	}

	WeakRef<Type> weak() const {
		return WeakRef<Type>::fromRef(*this);
	}


private:

	std::shared_ptr<Type> datablock = nullptr;

	template<typename... Args>
	static Ref<Type> create(DatablockID id, Args... args) {
		Ref<Type> r;
		r.datablock = std::make_shared<Type>(id, args...);
		r->datablockSelf = r.cast<Datablock>().weak();
		return r;
	}

	/*
	* Returns whether this is the last remaining reference for this datablock, i.e. whether
	* the datablock is only referenced by the DatablockManager itself. If this returns true,
	* it's safe to remove/destroy the datablock, and the datablock is atomically marked as
	* deleted (by setting datablockSelf to a null WeakRef).
	*/
	bool checkGarbage() const {
		/* Despite the controversy around use_count() in a multithreaded setting, we observe
		that datablocks cannot be made relevant again from the DatablockManager alone, so only
		WeakRefs are still relevant. In this case, it is safe to remove the DatablockManager's
		reference even if a WeakRef elevates between now and then, because WeakRef elevations
		cannot assume the datablock still exists in the DatablockManager. */

		// TODO: MULTITHREADING: Make this operation atomic, such that no new Refs can be
		// created between when use_count() is called and when the datablock is marked as
		// "deleted." This mostly refers to elevating WeakRefs, but also double check we can't
		// copy Refs directly out of the DatablockManager.
		if (!this->datablock) {
			return true;
		}
		bool g = (this->datablock.use_count() == 1);
		if (g) {
			this->datablock->datablockSelf = nullptr;
		}
		return g;
	}

	template<typename T>
	friend class Ref;
	template<typename T>
	friend class WeakRef;
	template<typename T>
	friend class DatablockManager;
};



template<typename Type>
class WeakRef {
public:

	WeakRef() {}
	WeakRef(std::nullptr_t) {}
	WeakRef(const WeakRef<Type>& ref) { *this = ref; }
	WeakRef(WeakRef<Type>&& ref) { *this = ref; }

	WeakRef<Type>& operator=(const WeakRef<Type>& ref) {
		this->weakptr = ref.weakptr;
		return *this;
	}

	WeakRef<Type>& operator=(WeakRef<Type>&& ref) {
		this->weakptr = std::move(ref.weakptr);
		return *this;
	}

	operator bool() const {
		return this->exists();
	}

	/*
	* Returns whether the referenced datablock still exists. Don't bother calling this
	* if you intend to access the underlying datablock; just call elevate().
	*/
	bool exists() const {
		Ref<Type> r;
		r.datablock = this->weakptr.lock();
		return r.datablock && (this == (WeakRef<Type>*)&r.datablock->datablockSelf ||
			r.datablock->datablockSelf.exists());
	}

	/*
	* Elevates the underlying reference to a counted reference, or returns a null Ref
	* if the object has already been marked as deleted.
	*/
	Ref<Type> elevate() const {
		Ref<Type> r;
		r.datablock = this->weakptr.lock();
		if (r.datablock && r.datablock->datablockSelf.exists()) {
			return r;
		}
		return nullptr;
	}

	static WeakRef<Type> fromRef(const Ref<Type>& ref) {
		WeakRef<Type> wr;
		wr.weakptr = std::weak_ptr<Type>(ref.datablock);
		return wr;
	}


private:

	std::weak_ptr<Type> weakptr;

};



class Datablock {
public:

	Datablock(DatablockID id);
	virtual ~Datablock() = default;

	virtual DatablockID getID() final;

	// Returns a null Ref if this datablock has been deleted.
	virtual Ref<Datablock> getRef() final;
	// Returns a null WeakRef is this datablock has been deleted.
	virtual WeakRef<Datablock> getWeakRef() final;


private:

	DatablockID datablockID;

	/*
	* A Datablock is marked as "deleted" when its datablockSelf is set to nullptr.
	* When this happens, it means the datablock has been removed from the DatablockManager
	* and it should not longer be considered usable.
	*/
	WeakRef<Datablock> datablockSelf;

	template<typename T>
	friend class Ref;
	template<typename T>
	friend class WeakRef;
};



template<typename BaseType>
class DatablockManager {
public:

	DatablockManager() {
		static_assert(std::is_base_of_v<Datablock, BaseType>,
			"DatablockManager can only manage classes derived from Datablock"
		);
	}

	template<typename CreateType = BaseType, typename... Args>
	Ref<CreateType> create(Args... args) {
		static_assert(std::is_base_of_v<BaseType, CreateType>,
			"DatablockManager::create: CreateType must derive from BaseType"
		);
		Ref<CreateType> r = Ref<CreateType>::create(this->genNewID(), args...);
		this->datablocks.push_back(r);
		return r;
	}

	template<typename CreateType = BaseType, typename... Args>
	Ref<BaseType> createBase(Args... args) {
		static_assert(std::is_base_of_v<BaseType, CreateType>,
			"DatablockManager::createBase: CreateType must derive from BaseType"
		);
		return this->create(args...).cast<BaseType>();
	}

	Ref<BaseType> getByID(DatablockID id) {
		auto loc = std::find_if(this->datablocks.begin(), this->datablocks.end(), [=](Ref<BaseType>& d) {
			return d->getID() == id;
		});
		if (loc == this->datablocks.end()) {
			return nullptr;
		}
		return *loc;
	}

	void garbageCollect() {
		this->datablocks.erase(
			std::remove_if(this->datablocks.begin(), this->datablocks.end(),
				[](const Ref<BaseType>& r) { return r.checkGarbage(); }),
			this->datablocks.end()
		);
	}

	const std::vector<Ref<BaseType>>& iterate() {
		return this->datablocks;
	}


private:

	// TODO: Consider alternative data structures.
	std::vector<Ref<BaseType>> datablocks;


	DatablockID genNewID() {
		// nextID is unique on a per-BaseType basis.
		static DatablockID nextID = DatablockNull + 1;
		return nextID++;
	}


};
