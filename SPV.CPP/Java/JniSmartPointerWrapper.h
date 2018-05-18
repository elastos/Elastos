#ifndef _JNISMARTPOINTERWRAPPER_H
#define _JNISMARTPOINTERWRAPPER_H

#include <jni.h>
#include <android/log.h>
#include <boost/shared_ptr.hpp>
#include "JniHelpers.h"

/** @brief a Wrapper for smart pointers to be used in JNI code
 *
 * **Usage**
 * Instantiation:
 * SmartPointerWrapper<Object> obj = new SmartPointerWrapper<Object>(arguments);
 * obj->instantiate(env,instance);
 *
 * Recovery:
 * boost::shared_ptr<Object> obj = SmartPointerWrapper<Object>::object(env,instance);
 *
 * or
 *
 * SmartPointerWrapper<Object> wrapper = SmartPointerWrapper<Object>::get(env,instance);
 * boost::shared_ptr<Object> obj = wrapper->get();
 *
 * Dispose:
 * SmartPointerWrapper<Object> wrapper = SmartPointerWrapper<Object>::get(env,instance);
 * delete wrapper;
 *
 * or simpler
 *
 * SmartPointerWrapper<Object>::dispose(env,instance);
 */
template <typename T>
class SmartPointerWrapper {
	boost::shared_ptr<T> mObject;
	public:
	//template <typename ...ARGS>
	//	explicit SmartPointerWrapper(ARGS... a) {
	//		mObject = boost::make_shared<T>(a...);
	//	}

	explicit SmartPointerWrapper (boost::shared_ptr<T> obj) {
		mObject = obj;
	}

	virtual ~SmartPointerWrapper() noexcept = default;

	void instantiate (JNIEnv *env, jobject instance) {
		setHandle<SmartPointerWrapper>(env, instance, this);
	}

	jlong instance() const {
		return reinterpret_cast<jlong>(this);
	}

	boost::shared_ptr<T> get() const {
		return mObject;
	}

	static boost::shared_ptr<T> object(JNIEnv *env, jobject instance) {
		return get(env, instance)->get();
	}

	static SmartPointerWrapper<T> *get(JNIEnv *env, jobject instance) {
		return getHandle<SmartPointerWrapper<T>>(env, instance);
	}

	static void dispose(JNIEnv *env, jobject instance) {
		auto obj = get(env, instance);
		__android_log_print(ANDROID_LOG_DEBUG, "JNI", "depose smartptr: 0x%lx", (long)obj);
		if (nullptr != obj) delete obj;
		setHandle<SmartPointerWrapper>(env, instance, nullptr);
	}
};

#endif
