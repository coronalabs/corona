/*
 * $Id: DefaultJavaReflector.java 134 2012-01-23 20:35:05Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

// --------------------------------------------------------------------------------
// Android does not support JavaBeans.
// Leave this commented out for now.
// Would be better to access Beans via reflection in the future.
// --------------------------------------------------------------------------------
//import java.beans.BeanInfo;
//import java.beans.IntrospectionException;
//import java.beans.Introspector;
//import java.beans.PropertyDescriptor;
import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Default implementation of the <code>JavaReflector</code> interface.
 */
public class DefaultJavaReflector implements JavaReflector {
	// -- Static
	private static final DefaultJavaReflector INSTANCE = new DefaultJavaReflector();
	private static final Object JAVA_FUNCTION_TYPE = new Object();
	private static final Object[] EMPTY_ARGUMENTS = new Object[0];

	// -- State
	private Map<Class<?>, Map<String, Accessor>> accessors = new HashMap<Class<?>, Map<String, Accessor>>();
	private ReadWriteLock accessorLock = new ReentrantReadWriteLock();
	private Map<LuaCallSignature, Invocable> invocableDispatches = new HashMap<LuaCallSignature, Invocable>();
	private ReadWriteLock invocableDispatchLock = new ReentrantReadWriteLock();
	private JavaFunction index = new Index();
	private JavaFunction newIndex = new NewIndex();
	private JavaFunction equal = new Equal();
	private JavaFunction length = new Length();
	private JavaFunction lessThan = new LessThan();
	private JavaFunction lessThanOrEqual = new LessThanOrEqual();
	private JavaFunction toString = new ToString();
	private JavaFunction javaFields = new AccessorPairs(FieldAccessor.class);
	private JavaFunction javaMethods = new AccessorPairs(
			InvocableAccessor.class);
	private JavaFunction javaProperties = new AccessorPairs(
			PropertyAccessor.class);

	// -- Static methods
	/**
	 * Returns the instance of this class.
	 * 
	 * @return the instance
	 */
	public static DefaultJavaReflector getInstance() {
		return INSTANCE;
	}

	// -- Construction
	/**
	 * Creates a new instances;
	 */
	private DefaultJavaReflector() {
	}

	// -- JavaReflector methods
	@Override
	public JavaFunction getMetamethod(Metamethod metamethod) {
		switch (metamethod) {
		case INDEX:
			return index;
		case NEWINDEX:
			return newIndex;
		case EQ:
			return equal;
		case LEN:
			return length;
		case LT:
			return lessThan;
		case LE:
			return lessThanOrEqual;
		case TOSTRING:
			return toString;
		case JAVAFIELDS:
			return javaFields;
		case JAVAMETHODS:
			return javaMethods;
		case JAVAPROPERTIES:
			return javaProperties;
		default:
			return null;
		}
	}

	// -- Private methods
	/**
	 * Returns the accessors of an object.
	 */
	private Map<String, Accessor> getObjectAccessors(Object object) {
		// Check cache
		Class<?> clazz = getObjectClass(object);
		accessorLock.readLock().lock();
		try {
			Map<String, Accessor> result = accessors.get(clazz);
			if (result != null) {
				return result;
			}
		} finally {
			accessorLock.readLock().unlock();
		}

		// Fill in
		Map<String, Accessor> result = createClassAccessors(clazz);
		accessorLock.writeLock().lock();
		try {
			if (!accessors.containsKey(clazz)) {
				accessors.put(clazz, result);
			} else {
				result = accessors.get(clazz);
			}
		} finally {
			accessorLock.writeLock().unlock();
		}
		return result;
	}

	/**
	 * Creates the accessors of a class.
	 */
	private Map<String, Accessor> createClassAccessors(Class<?> clazz) {
		Map<String, Accessor> result = new HashMap<String, Accessor>();

		// Fields
		Field[] fields = clazz.getFields();
		for (int i = 0; i < fields.length; i++) {
			result.put(fields[i].getName(), new FieldAccessor(fields[i]));
		}

		// Methods
		Map<String, Map<List<Class<?>>, Invocable>> accessibleMethods = new HashMap<String, Map<List<Class<?>>, Invocable>>();
		Method[] methods = clazz.getMethods();
		for (int i = 0; i < methods.length; i++) {
			// Do not overwrite fields
			Method method = methods[i];
			if (result.containsKey(method.getName())) {
				continue;
			}

			// Find the method in an interface if the declaring class is not
			// public
			if (!Modifier.isPublic(method.getDeclaringClass().getModifiers())) {
				method = getInterfaceMethod(clazz, method.getName(), method
						.getParameterTypes());
				if (method == null) {
					continue;
				}
			}

			// For each method name and parameter type list, keep
			// only the method declared by the most specific class
			Map<List<Class<?>>, Invocable> overloaded = accessibleMethods
					.get(method.getName());
			if (overloaded == null) {
				overloaded = new HashMap<List<Class<?>>, Invocable>();
				accessibleMethods.put(method.getName(), overloaded);
			}
			List<Class<?>> parameterTypes = Arrays.asList(method
					.getParameterTypes());
			Invocable currentInvocable = overloaded.get(parameterTypes);
			if (currentInvocable != null
					&& method.getDeclaringClass().isAssignableFrom(
							currentInvocable.getDeclaringClass())) {
				continue;
			}

			overloaded.put(parameterTypes, new InvocableMethod(method));
		}
		for (Map.Entry<String, Map<List<Class<?>>, Invocable>> entry : accessibleMethods
				.entrySet()) {
			result.put(entry.getKey(), new InvocableAccessor(clazz, entry
					.getValue().values()));
		}

		// Constructors
		Constructor<?>[] constructors = clazz.getConstructors();
		List<Invocable> accessibleConstructors = new ArrayList<Invocable>(
				constructors.length);
		for (int i = 0; i < constructors.length; i++) {
			// Ignore constructor if the declaring class is not public
			if (!Modifier.isPublic(constructors[i].getDeclaringClass()
					.getModifiers())) {
				continue;
			}
			accessibleConstructors
					.add(new InvocableConstructor(constructors[i]));
		}
		if (clazz.isInterface()) {
			accessibleConstructors.add(new InvocableProxy(clazz));
		}
		if (!accessibleConstructors.isEmpty()) {
			result.put("new", new InvocableAccessor(clazz,
					accessibleConstructors));
		}

// --------------------------------------------------------------------------------
// Android does not support JavaBeans.
// Leave this commented out for now.
// Would be better to access Beans via reflection in the future.
// --------------------------------------------------------------------------------
//
//		// Properties
//		BeanInfo beanInfo;
//		try {
//			beanInfo = Introspector.getBeanInfo(clazz);
//		} catch (IntrospectionException e) {
//			throw new RuntimeException(e);
//		}
//		PropertyDescriptor[] propertyDescriptors = beanInfo
//				.getPropertyDescriptors();
//		for (int i = 0; i < propertyDescriptors.length; i++) {
//			// Do not overwrite fields or methods
//			if (result.containsKey(propertyDescriptors[i].getName())) {
//				continue;
//			}
//
//			// Find the read/write methods in an interface if the declaring
//			// class is not public
//			Method method = propertyDescriptors[i].getReadMethod();
//			if (method != null
//					&& !Modifier.isPublic(method.getDeclaringClass()
//							.getModifiers())) {
//				method = getInterfaceMethod(clazz, method.getName(), method
//						.getParameterTypes());
//				try {
//					propertyDescriptors[i].setReadMethod(method);
//				} catch (IntrospectionException e) {
//				}
//			}
//			method = propertyDescriptors[i].getWriteMethod();
//			if (method != null
//					&& !Modifier.isPublic(method.getDeclaringClass()
//							.getModifiers())) {
//				method = getInterfaceMethod(clazz, method.getName(), method
//						.getParameterTypes());
//				try {
//					propertyDescriptors[i].setWriteMethod(method);
//				} catch (IntrospectionException e) {
//				}
//			}
//
//			// Do not process properties without a read and a write method
//			if (propertyDescriptors[i].getReadMethod() == null
//					&& propertyDescriptors[i].getWriteMethod() == null) {
//				continue;
//			}
//			result.put(propertyDescriptors[i].getName(), new PropertyAccessor(
//					clazz, propertyDescriptors[i]));
//		}
		
		return result;
	}

	/**
	 * Returns an interface method matching a method name and parameter list.
	 */
	private Method getInterfaceMethod(Class<?> clazz, String methodName,
			Class<?>[] parameterTypes) {
		do {
			// Get interfaces
			Class<?>[] interfaces = clazz.getInterfaces();
			for (int i = 0; i < interfaces.length; i++) {
				// Ignore non-public interfaces
				if (!Modifier.isPublic(interfaces[i].getModifiers())) {
					continue;
				}

				// Find method in the current interface
				try {
					return interfaces[i].getDeclaredMethod(methodName,
							parameterTypes);
				} catch (NoSuchMethodException e) {
					// Not found
				}

				// Check superinterfaces
				Method method = getInterfaceMethod(interfaces[i], methodName,
						parameterTypes);
				if (method != null) {
					return method;
				}
			}

			// Check superclass
			clazz = clazz.getSuperclass();
		} while (clazz != null);

		// Not found
		return null;
	}

	/**
	 * Returns the class of an object, or the class itself if the object is
	 * already a class.
	 */
	private Class<?> getObjectClass(Object object) {
		return object instanceof Class<?> ? (Class<?>) object : object
				.getClass();
	}

	// -- Nested types
	/**
	 * <code>__index</code> metamethod implementation.
	 */
	private class Index implements JavaFunction {
		public int invoke(LuaState luaState) {
			// Get object and class
			Object object = luaState.toJavaObject(1, Object.class);
			Class<?> objectClass = getObjectClass(object);

			// Handle arrays
			if (objectClass.isArray()) {
				if (!luaState.isNumber(2)) {
					throw new LuaRuntimeException(String.format(
							"attempt to read array with %s accessor", luaState
									.typeName(2)));
				}
				int index = luaState.toInteger(2);
				int length = Array.getLength(object);
				if (index < 1 || index > length) {
					throw new LuaRuntimeException(String.format(
							"attempt to read array of length %d at index %d",
							length, index));
				}
				luaState.pushJavaObject(Array.get(object, index - 1));
				return 1;
			}

			// Handle objects
			Map<String, Accessor> objectAccessors = getObjectAccessors(object);
			String key = luaState.toString(-1);
			if (key == null) {
				throw new LuaRuntimeException(String.format(
						"attempt to read class %s with %s accessor", object
								.getClass().getCanonicalName(), luaState
								.typeName(-1)));
			}
			Accessor accessor = objectAccessors.get(key);
			if (accessor == null) {
				throw new LuaRuntimeException(
						String
								.format(
										"attempt to read class %s with accessor '%s' (undefined)",
										objectClass.getCanonicalName(), key));
			}
			accessor.read(luaState, object);
			return 1;
		}
	}

	/**
	 * <code>__newindex</code> metamethod implementation.
	 */
	private class NewIndex implements JavaFunction {
		public int invoke(LuaState luaState) {
			// Get object and class
			Object object = luaState.toJavaObject(1, Object.class);
			Class<?> objectClass = getObjectClass(object);

			// Handle arrays
			if (objectClass.isArray()) {
				if (!luaState.isNumber(2)) {
					throw new LuaRuntimeException(String.format(
							"attempt to write array with %s accessor", luaState
									.typeName(2)));
				}
				int index = luaState.toInteger(2);
				int length = Array.getLength(object);
				if (index < 1 || index > length) {
					throw new LuaRuntimeException(String.format(
							"attempt to write array of length %d at index %d",
							length, index));
				}
				Class<?> componentType = objectClass.getComponentType();
				if (!luaState.isJavaObject(3, componentType)) {
					throw new LuaRuntimeException(
							String
									.format(
											"attempt to write array of %s at index %d with %s value",
											componentType.getCanonicalName(),
											luaState.typeName(3)));
				}
				Object value = luaState.toJavaObject(3, componentType);
				Array.set(object, index - 1, value);
				return 0;
			}

			// Handle objects
			Map<String, Accessor> objectAccessors = getObjectAccessors(object);
			String key = luaState.toString(2);
			if (key == null) {
				throw new LuaRuntimeException(String.format(
						"attempt to write class %s with %s accessor", object
								.getClass().getCanonicalName(), luaState
								.typeName(2)));
			}
			Accessor accessor = objectAccessors.get(key);
			if (accessor == null) {
				throw new LuaRuntimeException(
						String
								.format(
										"attempt to write class %s with accessor '%s' (undefined)",
										objectClass.getCanonicalName(), key));
			}
			accessor.write(luaState, object);
			return 0;
		}
	}

	/**
	 * <code>__len</code> metamethod implementation.
	 */
	private class Length implements JavaFunction {
		@Override
		public int invoke(LuaState luaState) {
			Object object = luaState.toJavaObject(1, Object.class);
			if (object.getClass().isArray()) {
				luaState.pushInteger(Array.getLength(object));
				return 1;
			}
			luaState.pushInteger(0);
			return 1;
		}
	}

	/**
	 * <code>__eq</code> metamethod implementation.
	 */
	private class Equal implements JavaFunction {
		@Override
		public int invoke(LuaState luaState) {
			Object object1 = luaState.toJavaObject(1, Object.class);
			Object object2 = luaState.toJavaObject(2, Object.class);
			luaState.pushBoolean(object1 == object2 || object1 != null
					&& object1.equals(object2));
			return 1;
		}
	}

	/**
	 * <code>__lt</code> metamethod implementation.
	 */
	private class LessThan implements JavaFunction {
		@SuppressWarnings("unchecked")
		@Override
		public int invoke(LuaState luaState) {
			if (!luaState.isJavaObject(1, Comparable.class)) {
				throw new LuaRuntimeException(String.format(
						"class %s does not implement Comparable", luaState
								.typeName(1)));
			}
			Comparable<Object> comparable = luaState.toJavaObject(1,
					Comparable.class);
			Object object = luaState.toJavaObject(2, Object.class);
			luaState.pushBoolean(comparable.compareTo(object) < 0);
			return 1;
		}
	}

	/**
	 * <code>__le</code> metamethod implementation.
	 */
	private class LessThanOrEqual implements JavaFunction {
		@SuppressWarnings("unchecked")
		@Override
		public int invoke(LuaState luaState) {
			if (!luaState.isJavaObject(1, Comparable.class)) {
				throw new LuaRuntimeException(String.format(
						"class %s does not implement Comparable", luaState
								.typeName(1)));
			}
			Comparable<Object> comparable = luaState.toJavaObject(1,
					Comparable.class);
			Object object = luaState.toJavaObject(2, Object.class);
			luaState.pushBoolean(comparable.compareTo(object) <= 0);
			return 1;
		}
	}

	/**
	 * Provides an iterator for accessors.
	 */
	private class AccessorPairs implements JavaFunction {
		// -- State
		private Class<?> accessorClass;

		// -- Construction
		/**
		 * Creates a new instance.
		 */
		public AccessorPairs(Class<?> accessorClass) {
			this.accessorClass = accessorClass;
		}

		// -- JavaFunction methods
		@Override
		public int invoke(LuaState luaState) {
			// Get object
			Object object = luaState.toJavaObject(1, Object.class);
			Class<?> objectClass = getObjectClass(object);

			// Create iterator
			Map<String, Accessor> objectAccessors = getObjectAccessors(object);
			Iterator<Entry<String, Accessor>> iterator = objectAccessors
					.entrySet().iterator();
			luaState.pushJavaObject(new AccessorNext(iterator,
					objectClass == object));
			luaState.pushJavaObject(object);
			luaState.pushNil();
			return 3;
		}

		// -- Member types
		/**
		 * Provides the next function for iterating accessors.
		 */
		private class AccessorNext implements JavaFunction {
			// -- State
			private Iterator<Entry<String, Accessor>> iterator;
			private boolean isStatic;

			// -- Construction
			/**
			 * Creates a new instance.
			 */
			public AccessorNext(Iterator<Entry<String, Accessor>> iterator,
					boolean isStatic) {
				this.iterator = iterator;
				this.isStatic = isStatic;
			}

			// -- JavaFunction methods
			@Override
			public int invoke(LuaState luaState) {
				while (iterator.hasNext()) {
					Entry<String, Accessor> entry = iterator.next();
					Accessor accessor = entry.getValue();

					// Filter by accessor class
					if (accessor.getClass() != accessorClass) {
						continue;
					}

					// Filter by non-static, static
					if (isStatic) {
						if (!accessor.isStatic()) {
							continue;
						}
					} else {
						if (!accessor.isNotStatic()) {
							continue;
						}
					}

					// Push match
					luaState.pushString(entry.getKey());
					Object object = luaState.toJavaObject(1, Object.class);
					accessor.read(luaState, object);
					return 2;
				}

				// End iteration
				return 0;
			}
		}
	}

	/**
	 * <code>__tostring</code> metamethod implementation.
	 */
	private class ToString implements JavaFunction {
		@Override
		public int invoke(LuaState luaState) {
			Object object = luaState.toJavaObject(1, Object.class);
			luaState.pushString(object != null ? object.toString() : "null");
			return 1;
		}
	}

	/**
	 * Provides access to class or object members.
	 */
	private interface Accessor {
		/**
		 * Reads the object member.
		 */
		void read(LuaState luaState, Object object);

		/**
		 * Writes the object member.
		 */
		void write(LuaState luaState, Object object);

		/**
		 * Returns whether this accessor is applicable in a non-static context.
		 */
		boolean isNotStatic();

		/**
		 * Returns whether this accessor is applicable in a static context.
		 */
		boolean isStatic();
	}

	/**
	 * Provides field access.
	 */
	private class FieldAccessor implements Accessor {
		// -- State
		private Field field;

		// -- Construction
		/**
		 * Creates a new instance.
		 */
		public FieldAccessor(Field field) {
			this.field = field;
		}

		// -- Accessor methods
		@Override
		public void read(LuaState luaState, Object object) {
			try {
				Class<?> objectClass = getObjectClass(object);
				if (objectClass == object) {
					object = null;
				}
				luaState.pushJavaObject(field.get(object));
			} catch (IllegalArgumentException e) {
				throw new RuntimeException(e);
			} catch (IllegalAccessException e) {
				throw new RuntimeException(e);
			}
		}

		@Override
		public void write(LuaState luaState, Object object) {
			try {
				Class<?> objectClass = getObjectClass(object);
				if (objectClass == object) {
					object = null;
				}
				Object value = luaState.checkJavaObject(-1, field.getType());
				field.set(object, value);
			} catch (IllegalArgumentException e) {
				throw new RuntimeException(e);
			} catch (IllegalAccessException e) {
				throw new RuntimeException(e);
			}
		}

		@Override
		public boolean isNotStatic() {
			return !Modifier.isStatic(field.getModifiers());
		}

		@Override
		public boolean isStatic() {
			return Modifier.isStatic(field.getModifiers());
		}
	}

	/**
	 * Provides invocable access.
	 */
	private class InvocableAccessor implements Accessor, JavaFunction {
		// -- State
		private Class<?> clazz;
		private List<Invocable> invocables;

		// -- Construction
		/**
		 * Creates a new instance.
		 */
		public InvocableAccessor(Class<?> clazz,
				Collection<Invocable> invocables) {
			this.clazz = clazz;
			this.invocables = new ArrayList<Invocable>(invocables);
		}

		// -- Properties
		/**
		 * Returns the name of the invocable.
		 */
		public String getName() {
			return invocables.get(0).getName();
		}

		/**
		 * Returns what this invocable accessor is for.
		 */
		public String getWhat() {
			return invocables.get(0).getWhat();
		}

		// -- Accessor methods
		@Override
		public void read(LuaState luaState, Object object) {
			Class<?> objectClass = getObjectClass(object);
			if (objectClass == object) {
				object = null;
			}
			luaState.pushJavaFunction(this);
		}

		@Override
		public void write(LuaState luaState, Object object) {
			Class<?> objectClass = getObjectClass(object);
			throw new LuaRuntimeException(String.format(
					"attempt to write class %s with accessor '%s' (a %s)",
					objectClass.getCanonicalName(), getName(), getWhat()));
		}

		@Override
		public boolean isNotStatic() {
			for (Invocable invocable : invocables) {
				if (!Modifier.isStatic(invocable.getModifiers())) {
					return true;
				}
			}
			return false;
		}

		@Override
		public boolean isStatic() {
			for (Invocable invocable : invocables) {
				if (Modifier.isStatic(invocable.getModifiers())) {
					return true;
				}
			}
			return false;
		}

		// -- JavaFunction methods
		@Override
		public int invoke(LuaState luaState) {
			// Argument sanity checks
			Object object = luaState.checkJavaObject(1, Object.class);
			Class<?> objectClass = getObjectClass(object);
			luaState.checkArg(1, clazz.isAssignableFrom(objectClass), String
					.format("class %s is not a subclass of %s", objectClass
							.getCanonicalName(), clazz.getCanonicalName()));
			if (objectClass == object) {
				object = null;
			}

			// Invocable dispatch
			LuaCallSignature luaCallSignature = getLuaCallSignature(luaState);
			Invocable invocable;
			invocableDispatchLock.readLock().lock();
			try {
				invocable = invocableDispatches.get(luaCallSignature);
			} finally {
				invocableDispatchLock.readLock().unlock();
			}
			if (invocable == null) {
				invocable = dispatchInvocable(luaState, object == null);
				invocableDispatchLock.writeLock().lock();
				try {
					if (!invocableDispatches.containsKey(luaCallSignature)) {
						invocableDispatches.put(luaCallSignature, invocable);
					} else {
						invocable = invocableDispatches.get(luaCallSignature);
					}
				} finally {
					invocableDispatchLock.writeLock().unlock();
				}
			}

			// Prepare arguments
			int argCount = luaState.getTop() - 1;
			int parameterCount = invocable.getParameterCount();
			Object[] arguments = new Object[parameterCount];
			if (invocable.isVarArgs()) {
				for (int i = 0; i < parameterCount - 1; i++) {
					arguments[i] = luaState.toJavaObject(i + 2, invocable
							.getParameterType(i));
				}
				arguments[parameterCount - 1] = Array.newInstance(invocable
						.getParameterType(parameterCount - 1), argCount
						- (parameterCount - 1));
				for (int i = parameterCount - 1; i < argCount; i++) {
					Array.set(arguments[parameterCount - 1], i
							- (parameterCount - 1), luaState.toJavaObject(
							i + 2, invocable.getParameterType(i)));
				}
			} else {
				for (int i = 0; i < parameterCount; i++) {
					arguments[i] = luaState.toJavaObject(i + 2, invocable
							.getParameterType(i));
				}
			}

			// Invoke
			Object result;
			try {
				result = invocable.invoke(object, arguments);
			} catch (InstantiationException e) {
				throw new RuntimeException(e);
			} catch (IllegalArgumentException e) {
				throw new RuntimeException(e);
			} catch (IllegalAccessException e) {
				throw new RuntimeException(e);
			} catch (InvocationTargetException e) {
				throw new RuntimeException(e.getTargetException());
			}

			// Return
			if (invocable.getReturnType() != Void.TYPE) {
				if (invocable.isRawReturn()) {
					luaState.pushJavaObjectRaw(result);
				} else {
					luaState.pushJavaObject(result);
				}
				return 1;
			} else {
				return 0;
			}
		}

		// -- Private methods
		/**
		 * Creates a Lua call signature.
		 */
		private LuaCallSignature getLuaCallSignature(LuaState luaState) {
			int argCount = luaState.getTop() - 1;
			Object[] types = new Object[argCount];
			for (int i = 0; i < argCount; i++) {
				LuaType type = luaState.type(i + 2);
				switch (type) {
				case FUNCTION:
					types[i] = luaState.isJavaFunction(i + 2) ? JAVA_FUNCTION_TYPE
							: LuaType.FUNCTION;
					break;
				case USERDATA:
					if (luaState.isJavaObjectRaw(i + 2)) {
						Object object = luaState.toJavaObjectRaw(i + 2);
						if (object instanceof TypedJavaObject) {
							types[i] = ((TypedJavaObject) object).getType();
						} else {
							types[i] = object.getClass();
						}
					} else {
						types[i] = LuaType.USERDATA;
					}
					break;
				default:
					types[i] = type;
				}
			}
			return new LuaCallSignature(clazz, getName(), types);
		}

		/**
		 * Dispatches an invocable.
		 */
		private Invocable dispatchInvocable(LuaState luaState,
				boolean staticDispatch) {
			// Begin with all candidates
			Set<Invocable> candidates = new HashSet<Invocable>(invocables);

			// Eliminate methods with an invalid static modifier
			for (Iterator<Invocable> i = candidates.iterator(); i.hasNext();) {
				Invocable invocable = i.next();
				if (Modifier.isStatic(invocable.getModifiers()) != staticDispatch) {
					i.remove();
				}
			}

			// Eliminate methods with an invalid parameter count
			int argCount = luaState.getTop() - 1;
			for (Iterator<Invocable> i = candidates.iterator(); i.hasNext();) {
				Invocable invocable = i.next();
				if (invocable.isVarArgs()) {
					if (argCount < invocable.getParameterCount() - 1) {
						i.remove();
					}
				} else {
					if (argCount != invocable.getParameterCount()) {
						i.remove();
					}
				}
			}

			// Eliminate methods that are not applicable
			Converter converter = luaState.getConverter();
			outer: for (Iterator<Invocable> i = candidates.iterator(); i
					.hasNext();) {
				Invocable invocable = i.next();
				for (int j = 0; j < argCount; j++) {
					int distance = converter.getTypeDistance(luaState, j + 2,
							invocable.getParameterType(j));
					if (distance == Integer.MAX_VALUE) {
						i.remove();
						continue outer;
					}
				}
			}

			// Eliminate variable arguments methods in the presence of fix
			// arguments methods
			boolean haveFixArgs = false;
			boolean haveVarArgs = false;
			for (Invocable invocable : candidates) {
				haveFixArgs = haveFixArgs || !invocable.isVarArgs();
				haveVarArgs = haveVarArgs || invocable.isVarArgs();
			}
			if (haveVarArgs && haveFixArgs) {
				for (Iterator<Invocable> i = candidates.iterator(); i.hasNext();) {
					Invocable invocable = i.next();
					if (invocable.isVarArgs()) {
						i.remove();
					}
				}
			}

			// Eliminate methods that are not closest
			outer: for (Iterator<Invocable> i = candidates.iterator(); i
					.hasNext();) {
				Invocable invocable = i.next();
				inner: for (Invocable other : candidates) {
					if (other == invocable) {
						continue;
					}
					int parameterCount = Math.min(argCount, Math.max(invocable
							.getParameterCount(), other.getParameterCount()));
					boolean delta = false;
					for (int j = 0; j < parameterCount; j++) {
						int distance = converter.getTypeDistance(luaState,
								j + 2, invocable.getParameterType(j));
						int otherDistance = converter.getTypeDistance(luaState,
								j + 2, other.getParameterType(j));
						if (otherDistance > distance) {
							// Other is not closer
							continue inner;
						}
						delta = delta || distance != otherDistance;
					}

					// If there is no delta, other is not closer
					if (!delta) {
						continue;
					}

					// Other is closer
					i.remove();
					continue outer;
				}
			}

			// Eliminate methods that are not most precise
			outer: for (Iterator<Invocable> i = candidates.iterator(); i
					.hasNext();) {
				Invocable invocable = i.next();
				inner: for (Invocable other : candidates) {
					if (other == invocable) {
						continue;
					}
					int parameterCount = Math.min(argCount, Math.max(invocable
							.getParameterCount(), other.getParameterCount()));
					boolean delta = false;
					for (int j = 0; j < parameterCount; j++) {
						Class<?> type = invocable.getParameterType(j);
						Class<?> otherType = other.getParameterType(j);
						if (!type.isAssignableFrom(otherType)) {
							// Other is not more specific
							continue inner;
						}
						delta = delta || type != otherType;
					}

					// If there is no delta, other is not more specific
					if (!delta) {
						continue;
					}

					// Other is more specific
					i.remove();
					continue outer;
				}
			}

			// Handle outcomes
			if (candidates.isEmpty()) {
				throw getSignatureMismatchException(luaState);
			}
			if (candidates.size() > 1) {
				throw getSignatureAmbivalenceException(luaState, candidates);
			}

			// Return
			return candidates.iterator().next();
		}

		/**
		 * Returns a Lua runtime exception indicating that no matching invocable
		 * has been found.
		 */
		private LuaRuntimeException getSignatureMismatchException(
				LuaState luaState) {
			return new LuaRuntimeException(String.format(
					"no %s of class %s matches '%s(%s)'", getWhat(), clazz
							.getCanonicalName(), getName(),
					getLuaSignatureString(luaState)));
		}

		/**
		 * Returns a Lua runtime exception indicating that an invocable is
		 * ambivalent.
		 */
		private LuaRuntimeException getSignatureAmbivalenceException(
				LuaState luaState, Set<Invocable> candidates) {
			StringBuffer sb = new StringBuffer();
			sb.append(String.format(
					"%s '%s(%s)' on class %s is ambivalent among ", getWhat(),
					getName(), getLuaSignatureString(luaState), clazz
							.getCanonicalName()));
			boolean first = true;
			for (Invocable invocable : candidates) {
				if (first) {
					first = false;
				} else {
					sb.append(", ");
				}
				sb.append(String.format("'%s(%s)'", getName(),
						getJavaSignatureString(invocable.getParameterTypes())));
			}
			return new LuaRuntimeException(sb.toString());
		}

		/**
		 * Returns a Lua value signature string for diagnostic messages.
		 */
		private String getLuaSignatureString(LuaState luaState) {
			int argCount = luaState.getTop() - 1;
			StringBuffer sb = new StringBuffer();
			for (int i = 0; i < argCount; i++) {
				if (i > 0) {
					sb.append(", ");
				}
				sb.append(luaState.typeName(i + 2));
			}
			return sb.toString();
		}

		/**
		 * Returns a Java type signature string for diagnostic messages.
		 */
		private String getJavaSignatureString(Class<?>[] types) {
			StringBuffer sb = new StringBuffer();
			for (int i = 0; i < types.length; i++) {
				if (i > 0) {
					sb.append(", ");
				}
				sb.append(types[i].getCanonicalName());
			}
			return sb.toString();
		}
	}

	/**
	 * Provides property access.
	 */
	private class PropertyAccessor implements Accessor {
		// -- State
//		private Class<?> clazz;
//		private PropertyDescriptor propertyDescriptor;

//		// -- Construction
//		/**
//		 * Creates a new instance.
//		 */
//		public PropertyAccessor(Class<?> clazz,
//				PropertyDescriptor propertyDescriptor) {
//			this.clazz = clazz;
//			this.propertyDescriptor = propertyDescriptor;
//		}

		// -- Accessor methods
		@Override
		public void read(LuaState luaState, Object object) {
//			if (propertyDescriptor.getReadMethod() == null) {
//				throw new LuaRuntimeException(
//						String
//								.format(
//										"attempt to read class %s with accessor '%s' (a write-only property)",
//										clazz.getCanonicalName(),
//										propertyDescriptor.getName()));
//			}
//			try {
//				luaState.pushJavaObject(propertyDescriptor.getReadMethod()
//						.invoke(object, EMPTY_ARGUMENTS));
//			} catch (IllegalArgumentException e) {
//				throw new RuntimeException(e);
//			} catch (IllegalAccessException e) {
//				throw new RuntimeException(e);
//			} catch (InvocationTargetException e) {
//				throw new RuntimeException(e.getTargetException());
//			}
			throw new java.lang.UnsupportedOperationException();
		}

		@Override
		public void write(LuaState luaState, Object object) {
//			if (propertyDescriptor.getWriteMethod() == null) {
//				throw new LuaRuntimeException(
//						String
//								.format(
//										"attempt to write class %s with acessor '%s' (a read-only property)",
//										clazz.getCanonicalName(),
//										propertyDescriptor.getName()));
//			}
//			try {
//				Object value = luaState.checkJavaObject(-1, propertyDescriptor
//						.getPropertyType());
//				propertyDescriptor.getWriteMethod().invoke(object, value);
//			} catch (IllegalArgumentException e) {
//				throw new RuntimeException(e);
//			} catch (IllegalAccessException e) {
//				throw new RuntimeException(e);
//			} catch (InvocationTargetException e) {
//				throw new RuntimeException(e.getTargetException());
//			}
//			luaState.pop(1);
			throw new java.lang.UnsupportedOperationException();
		}

		@Override
		public boolean isNotStatic() {
			return true;
		}

		@Override
		public boolean isStatic() {
			return false;
		}
	}

	/**
	 * Virtual superinterface for methods and constructors.
	 */
	private interface Invocable {
		/**
		 * Returns what this invocable is, for use in diagnostic messages.
		 */
		public String getWhat();

		/**
		 * Returns the declaring class of this invocable.
		 */
		public Class<?> getDeclaringClass();

		/**
		 * Returns the modifiers of this invocable.
		 * 
		 * @return
		 */
		public int getModifiers();

		/**
		 * Returns the name of this invocable.
		 */
		public String getName();

		/**
		 * Returns the return type of this invocable.
		 */
		public Class<?> getReturnType();

		/**
		 * Returns whether this invocable has a return value that must be pushed
		 * raw.
		 */
		public boolean isRawReturn();

		/**
		 * Returns the number of parameters.
		 */
		public int getParameterCount();

		/**
		 * Returns the parameter types of this invocable.
		 */
		public Class<?>[] getParameterTypes();

		/**
		 * Returns a parameter type, flattening variable arguments.
		 */
		public Class<?> getParameterType(int index);

		/**
		 * Returns whether this invocable has a variable number of arguments.
		 * 
		 * @return
		 */
		public boolean isVarArgs();

		/**
		 * Invokes this invocable.
		 */
		public Object invoke(Object obj, Object... args)
				throws InstantiationException, IllegalAccessException,
				IllegalArgumentException, InvocationTargetException;
	}

	/**
	 * Invocable method.
	 */
	private static class InvocableMethod implements Invocable {
		private Method method;
		private Class<?>[] parameterTypes;

		/**
		 * Creates a new instance.
		 */
		public InvocableMethod(Method method) {
			this.method = method;
			this.parameterTypes = method.getParameterTypes();
		}

		@Override
		public String getWhat() {
			return "method";
		}

		@Override
		public Class<?> getDeclaringClass() {
			return method.getDeclaringClass();
		}

		@Override
		public int getModifiers() {
			return method.getModifiers();
		}

		@Override
		public String getName() {
			return method.getName();
		}

		@Override
		public Class<?> getReturnType() {
			return method.getReturnType();
		}

		@Override
		public boolean isRawReturn() {
			return false;
		}

		@Override
		public int getParameterCount() {
			return parameterTypes.length;
		}

		@Override
		public Class<?>[] getParameterTypes() {
			return parameterTypes;
		}

		@Override
		public Class<?> getParameterType(int index) {
			if (method.isVarArgs() && index >= parameterTypes.length - 1) {
				return parameterTypes[parameterTypes.length - 1]
						.getComponentType();
			} else {
				return parameterTypes[index];
			}
		}

		@Override
		public boolean isVarArgs() {
			return method.isVarArgs();
		}

		@Override
		public Object invoke(Object obj, Object... args)
				throws IllegalAccessException, IllegalArgumentException,
				InvocationTargetException {
			return method.invoke(obj, args);
		}

		@Override
		public String toString() {
			return method.toString();
		}
	}

	/**
	 * Invocable constructor.
	 */
	private static class InvocableConstructor implements Invocable {
		// -- State
		private Constructor<?> constructor;
		private Class<?>[] parameterTypes;

		/**
		 * Creates a new instance.
		 */
		public InvocableConstructor(Constructor<?> constructor) {
			this.constructor = constructor;
			this.parameterTypes = constructor.getParameterTypes();
		}

		@Override
		public String getWhat() {
			return "constructor";
		}

		@Override
		public Class<?> getDeclaringClass() {
			return constructor.getDeclaringClass();
		}

		@Override
		public int getModifiers() {
			return constructor.getModifiers() | Modifier.STATIC;
		}

		@Override
		public String getName() {
			return "new";
		}

		@Override
		public Class<?> getReturnType() {
			return constructor.getDeclaringClass();
		}

		@Override
		public boolean isRawReturn() {
			return false;
		}

		@Override
		public int getParameterCount() {
			return parameterTypes.length;
		}

		@Override
		public Class<?>[] getParameterTypes() {
			return parameterTypes;
		}

		@Override
		public Class<?> getParameterType(int index) {
			if (constructor.isVarArgs() && index >= parameterTypes.length - 1) {
				return parameterTypes[parameterTypes.length - 1]
						.getComponentType();
			} else {
				return parameterTypes[index];
			}
		}

		@Override
		public boolean isVarArgs() {
			return constructor.isVarArgs();
		}

		@Override
		public Object invoke(Object obj, Object... args)
				throws InstantiationException, IllegalAccessException,
				IllegalArgumentException, InvocationTargetException {
			return constructor.newInstance(args);
		}

		@Override
		public String toString() {
			return constructor.toString();
		}
	}

	/**
	 * Invocable proxy.
	 */
	private static class InvocableProxy implements Invocable {
		// -- Static
		private static final Class<?>[] PARAMETER_TYPES = new Class<?>[] { LuaValueProxy.class };

		// -- State
		private Class<?> interfaze;

		/**
		 * Creates a new instance.
		 */
		public InvocableProxy(Class<?> interfaze) {
			this.interfaze = interfaze;
		}

		@Override
		public String getWhat() {
			return "proxy";
		}

		@Override
		public Class<?> getDeclaringClass() {
			return interfaze;
		}

		@Override
		public int getModifiers() {
			return interfaze.getModifiers() | Modifier.STATIC;
		}

		@Override
		public String getName() {
			return "new";
		}

		@Override
		public Class<?> getReturnType() {
			return interfaze;
		}

		@Override
		public boolean isRawReturn() {
			return true;
		}

		@Override
		public int getParameterCount() {
			return 1;
		}

		@Override
		public Class<?>[] getParameterTypes() {
			return PARAMETER_TYPES;
		}

		@Override
		public Class<?> getParameterType(int index) {
			return PARAMETER_TYPES[0];
		}

		@Override
		public boolean isVarArgs() {
			return false;
		}

		@Override
		public Object invoke(Object obj, Object... args)
				throws InstantiationException, IllegalAccessException,
				IllegalArgumentException, InvocationTargetException {
			LuaValueProxy luaValueProxy = (LuaValueProxy) args[0];
			luaValueProxy.pushValue();
			Object proxy = luaValueProxy.getLuaState().getProxy(-1, interfaze);
			luaValueProxy.getLuaState().pop(1);
			return proxy;
		}

		@Override
		public String toString() {
			return interfaze.toString();
		}
	}

	/**
	 * Lua call signature.
	 */
	private static class LuaCallSignature {
		// -- State
		private Class<?> clazz;
		private String invocableName;
		private Object[] types;
		private int hashCode;

		// -- Construction
		/**
		 * Creates a new instance.
		 */
		public LuaCallSignature(Class<?> clazz, String invocableName,
				Object[] types) {
			this.clazz = clazz;
			this.invocableName = invocableName;
			this.types = types;
			hashCode = clazz.hashCode();
			hashCode = hashCode * 65599 + invocableName.hashCode();
			for (int i = 0; i < types.length; i++) {
				hashCode = hashCode * 65599 + types[i].hashCode();
			}
		}

		@Override
		public int hashCode() {
			return hashCode;
		}

		@Override
		public boolean equals(Object obj) {
			if (obj == this) {
				return true;
			}
			if (!(obj instanceof LuaCallSignature)) {
				return false;
			}
			LuaCallSignature other = (LuaCallSignature) obj;
			if (clazz != other.clazz
					|| !invocableName.equals(other.invocableName)
					|| types.length != other.types.length) {
				return false;
			}
			for (int i = 0; i < types.length; i++) {
				if (types[i] != other.types[i]) {
					return false;
				}
			}
			return true;
		}

		@Override
		public String toString() {
			return clazz.getCanonicalName() + ": " + invocableName + "("
					+ Arrays.asList(types) + ")";
		}
	}
}
