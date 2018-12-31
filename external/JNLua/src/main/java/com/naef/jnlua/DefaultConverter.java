/*
 * $Id: DefaultConverter.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

import java.lang.reflect.Array;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.naef.jnlua.util.AbstractTableList;
import com.naef.jnlua.util.AbstractTableMap;

/**
 * Default implementation of the <code>Converter</code> interface.
 */
public class DefaultConverter implements Converter {
	// -- Static
	/**
	 * Static instance.
	 */
	private static final DefaultConverter INSTANCE = new DefaultConverter();

	/**
	 * Boolean distance map.
	 */
	private static final Map<Class<?>, Integer> BOOLEAN_DISTANCE_MAP = new HashMap<Class<?>, Integer>();
	static {
		BOOLEAN_DISTANCE_MAP.put(Boolean.class, new Integer(1));
		BOOLEAN_DISTANCE_MAP.put(Boolean.TYPE, new Integer(1));
		BOOLEAN_DISTANCE_MAP.put(Object.class, new Integer(2));
	}

	/**
	 * Number distance map.
	 */
	private static final Map<Class<?>, Integer> NUMBER_DISTANCE_MAP = new HashMap<Class<?>, Integer>();
	static {
		NUMBER_DISTANCE_MAP.put(Byte.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Byte.TYPE, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Short.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Short.TYPE, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Integer.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Integer.TYPE, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Long.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Long.TYPE, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Float.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Float.TYPE, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Double.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Double.TYPE, new Integer(1));
		NUMBER_DISTANCE_MAP.put(BigInteger.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(BigDecimal.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Character.class, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Character.TYPE, new Integer(1));
		NUMBER_DISTANCE_MAP.put(Object.class, new Integer(2));
		NUMBER_DISTANCE_MAP.put(String.class, new Integer(3));
	}

	/**
	 * String distance map.
	 */
	private static final Map<Class<?>, Integer> STRING_DISTANCE_MAP = new HashMap<Class<?>, Integer>();
	static {
		STRING_DISTANCE_MAP.put(String.class, new Integer(1));
		STRING_DISTANCE_MAP.put(Object.class, new Integer(2));
		STRING_DISTANCE_MAP.put(Byte.class, new Integer(3));
		STRING_DISTANCE_MAP.put(Byte.TYPE, new Integer(3));
		STRING_DISTANCE_MAP.put(Short.class, new Integer(3));
		STRING_DISTANCE_MAP.put(Short.TYPE, new Integer(3));
		STRING_DISTANCE_MAP.put(Integer.class, new Integer(3));
		STRING_DISTANCE_MAP.put(Integer.TYPE, new Integer(3));
		STRING_DISTANCE_MAP.put(Long.class, new Integer(3));
		STRING_DISTANCE_MAP.put(Long.TYPE, new Integer(3));
		STRING_DISTANCE_MAP.put(Float.class, new Integer(3));
		STRING_DISTANCE_MAP.put(Float.TYPE, new Integer(3));
		STRING_DISTANCE_MAP.put(Double.class, new Integer(3));
		STRING_DISTANCE_MAP.put(Double.TYPE, new Integer(3));
		STRING_DISTANCE_MAP.put(BigInteger.class, new Integer(3));
		STRING_DISTANCE_MAP.put(BigDecimal.class, new Integer(3));
		STRING_DISTANCE_MAP.put(Character.class, new Integer(3));
		STRING_DISTANCE_MAP.put(Character.TYPE, new Integer(3));
	}

	/**
	 * Function distance map.
	 */
	private static final Map<Class<?>, Integer> FUNCTION_DISTANCE_MAP = new HashMap<Class<?>, Integer>();
	static {
		FUNCTION_DISTANCE_MAP.put(JavaFunction.class, new Integer(1));
		FUNCTION_DISTANCE_MAP.put(Object.class, new Integer(2));
	}

	/**
	 * Lua value converters.
	 */
	private static final Map<Class<?>, LuaValueConverter<?>> LUA_VALUE_CONVERTERS = new HashMap<Class<?>, LuaValueConverter<?>>();
	static {
		LuaValueConverter<Boolean> booleanConverter = new LuaValueConverter<Boolean>() {
			@Override
			public Boolean convert(LuaState luaState, int index) {
				return Boolean.valueOf(luaState.toBoolean(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(Boolean.class, booleanConverter);
		LUA_VALUE_CONVERTERS.put(Boolean.TYPE, booleanConverter);

		LuaValueConverter<Byte> byteConverter = new LuaValueConverter<Byte>() {
			@Override
			public Byte convert(LuaState luaState, int index) {
				return Byte.valueOf((byte) luaState.toInteger(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(Byte.class, byteConverter);
		LUA_VALUE_CONVERTERS.put(Byte.TYPE, byteConverter);
		LuaValueConverter<Short> shortConverter = new LuaValueConverter<Short>() {
			@Override
			public Short convert(LuaState luaState, int index) {
				return Short.valueOf((short) luaState.toInteger(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(Short.class, shortConverter);
		LUA_VALUE_CONVERTERS.put(Short.TYPE, shortConverter);
		LuaValueConverter<Integer> integerConverter = new LuaValueConverter<Integer>() {
			@Override
			public Integer convert(LuaState luaState, int index) {
				return Integer.valueOf(luaState.toInteger(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(Integer.class, integerConverter);
		LUA_VALUE_CONVERTERS.put(Integer.TYPE, integerConverter);
		LuaValueConverter<Long> longConverter = new LuaValueConverter<Long>() {
			@Override
			public Long convert(LuaState luaState, int index) {
				return Long.valueOf((long) luaState.toNumber(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(Long.class, longConverter);
		LUA_VALUE_CONVERTERS.put(Long.TYPE, longConverter);
		LuaValueConverter<Float> floatConverter = new LuaValueConverter<Float>() {
			@Override
			public Float convert(LuaState luaState, int index) {
				return Float.valueOf((float) luaState.toNumber(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(Float.class, floatConverter);
		LUA_VALUE_CONVERTERS.put(Float.TYPE, floatConverter);
		LuaValueConverter<Double> doubleConverter = new LuaValueConverter<Double>() {
			@Override
			public Double convert(LuaState luaState, int index) {
				return Double.valueOf(luaState.toNumber(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(Double.class, doubleConverter);
		LUA_VALUE_CONVERTERS.put(Double.TYPE, doubleConverter);
		LuaValueConverter<BigInteger> bigIntegerConverter = new LuaValueConverter<BigInteger>() {
			@Override
			public BigInteger convert(LuaState luaState, int index) {
				return BigDecimal.valueOf(luaState.toNumber(index)).setScale(0,
						BigDecimal.ROUND_HALF_EVEN).toBigInteger();
			}
		};
		LUA_VALUE_CONVERTERS.put(BigInteger.class, bigIntegerConverter);
		LuaValueConverter<BigDecimal> bigDecimalConverter = new LuaValueConverter<BigDecimal>() {
			@Override
			public BigDecimal convert(LuaState luaState, int index) {
				return BigDecimal.valueOf(luaState.toNumber(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(BigDecimal.class, bigDecimalConverter);
		LuaValueConverter<Character> characterConverter = new LuaValueConverter<Character>() {
			@Override
			public Character convert(LuaState luaState, int index) {
				return Character.valueOf((char) luaState.toInteger(index));
			}
		};
		LUA_VALUE_CONVERTERS.put(Character.class, characterConverter);
		LUA_VALUE_CONVERTERS.put(Character.TYPE, characterConverter);
		LuaValueConverter<String> stringConverter = new LuaValueConverter<String>() {
			@Override
			public String convert(LuaState luaState, int index) {
				return luaState.toString(index);
			}
		};
		LUA_VALUE_CONVERTERS.put(String.class, stringConverter);
	}

	/**
	 * Java object converters.
	 */
	private static final Map<Class<?>, JavaObjectConverter<?>> JAVA_OBJECT_CONVERTERS = new HashMap<Class<?>, JavaObjectConverter<?>>();
	static {
		JavaObjectConverter<Boolean> booleanConverter = new JavaObjectConverter<Boolean>() {
			@Override
			public void convert(LuaState luaState, Boolean booleanValue) {
				luaState.pushBoolean(booleanValue.booleanValue());
			}
		};
		JAVA_OBJECT_CONVERTERS.put(Boolean.class, booleanConverter);
		JAVA_OBJECT_CONVERTERS.put(Boolean.TYPE, booleanConverter);
		JavaObjectConverter<Number> numberConverter = new JavaObjectConverter<Number>() {
			@Override
			public void convert(LuaState luaState, Number number) {
				luaState.pushNumber(number.doubleValue());
			}
		};
		JAVA_OBJECT_CONVERTERS.put(Byte.class, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Byte.TYPE, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Short.class, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Short.TYPE, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Integer.class, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Integer.TYPE, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Long.class, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Long.TYPE, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Float.class, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Float.TYPE, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Double.class, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(Double.TYPE, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(BigInteger.class, numberConverter);
		JAVA_OBJECT_CONVERTERS.put(BigDecimal.class, numberConverter);
		JavaObjectConverter<Character> characterConverter = new JavaObjectConverter<Character>() {
			@Override
			public void convert(LuaState luaState, Character character) {
				luaState.pushInteger(character.charValue());
			}
		};
		JAVA_OBJECT_CONVERTERS.put(Character.class, characterConverter);
		JAVA_OBJECT_CONVERTERS.put(Character.TYPE, characterConverter);
		JavaObjectConverter<String> stringConverter = new JavaObjectConverter<String>() {
			@Override
			public void convert(LuaState luaState, String string) {
				luaState.pushString(string);
			}
		};
		JAVA_OBJECT_CONVERTERS.put(String.class, stringConverter);
	}

	// -- Static methods
	/**
	 * Returns the instance of this class.
	 * 
	 * @return the instance
	 */
	public static DefaultConverter getInstance() {
		return INSTANCE;
	}

	// -- Construction
	/**
	 * Singleton.
	 */
	private DefaultConverter() {
	}

	// -- Java converter methods
	@Override
	public int getTypeDistance(LuaState luaState, int index, Class<?> formalType) {
		// Handle none
		LuaType luaType = luaState.type(index);
		if (luaType == null) {
			return Integer.MAX_VALUE;
		}
		
		// Handle void
		if (formalType == Void.TYPE) {
			return Integer.MAX_VALUE;
		}

		// Handle Lua value proxy
		if (formalType == LuaValueProxy.class) {
			return 0;
		}

		// Handle Lua types
		switch (luaType) {
		case NIL:
			return 1;
		case BOOLEAN:
			Integer distance = BOOLEAN_DISTANCE_MAP.get(formalType);
			if (distance != null) {
				return distance.intValue();
			}
			break;
		case NUMBER:
			distance = NUMBER_DISTANCE_MAP.get(formalType);
			if (distance != null) {
				return distance.intValue();
			}
			break;
		case STRING:
			distance = STRING_DISTANCE_MAP.get(formalType);
			if (distance != null) {
				return distance.intValue();
			}
			break;
		case TABLE:
			if (formalType == Map.class || formalType == List.class
					|| formalType.isArray()) {
				return 1;
			}
			if (formalType == Object.class) {
				return 2;
			}
			break;
		case FUNCTION:
			if (luaState.isJavaFunction(index)) {
				distance = FUNCTION_DISTANCE_MAP.get(formalType);
				if (distance != null) {
					return distance.intValue();
				}
			}
			break;
		case USERDATA:
			Object object = luaState.toJavaObjectRaw(index);
			if (object != null) {
				Class<?> type;
				if (object instanceof TypedJavaObject) {
					TypedJavaObject typedJavaObject = (TypedJavaObject) object;
					if (typedJavaObject.isStrong()) {
						if (formalType.isAssignableFrom(typedJavaObject
								.getClass())) {
							return 1;
						}
					}
					type = typedJavaObject.getType();
				} else {
					type = object.getClass();
				}
				if (formalType.isAssignableFrom(type)) {
					return 1;
				}
			}
			break;
		}

		// Handle object
		if (formalType == Object.class) {
			return Integer.MAX_VALUE - 1;
		}

		// Unsupported conversion
		return Integer.MAX_VALUE;
	}

	@SuppressWarnings("unchecked")
	@Override
	public <T> T convertLuaValue(LuaState luaState, int index,
			Class<T> formalType) {
		// Handle none
		LuaType luaType = luaState.type(index);
		if (luaType == null) {
			throw new IllegalArgumentException("undefined index: " + index);
		}
		
		// Handle void
		if (formalType == Void.TYPE) {
			throw new ClassCastException(String.format(
					"cannot convert %s to %s", luaState.typeName(index),
					formalType.getCanonicalName()));
		}

		// Handle Lua value proxy
		if (formalType == LuaValueProxy.class) {
			return (T) luaState.getProxy(index);
		}

		// Handle Lua types
		switch (luaType) {
		case NIL:
			return null;
		case BOOLEAN:
			LuaValueConverter<?> luaValueConverter;
			luaValueConverter = LUA_VALUE_CONVERTERS.get(formalType);
			if (luaValueConverter != null) {
				return (T) luaValueConverter.convert(luaState, index);
			}
			if (formalType == Object.class) {
				return (T) Boolean.valueOf(luaState.toBoolean(index));
			}
			break;
		case NUMBER:
			luaValueConverter = LUA_VALUE_CONVERTERS.get(formalType);
			if (luaValueConverter != null) {
				return (T) luaValueConverter.convert(luaState, index);
			}
			if (formalType == Object.class) {
				return (T) Double.valueOf(luaState.toNumber(index));
			}
			break;
		case STRING:
			luaValueConverter = LUA_VALUE_CONVERTERS.get(formalType);
			if (luaValueConverter != null) {
				return (T) luaValueConverter.convert(luaState, index);
			}
			if (formalType == Object.class) {
				return (T) luaState.toString(index);
			}
			break;
		case TABLE:
			if (formalType == Map.class || formalType == Object.class) {
				final LuaValueProxy luaValueProxy = luaState.getProxy(index);
				return (T) new AbstractTableMap<Object>() {
					@Override
					protected Object convertKey(int index) {
						return getLuaState().toJavaObject(index, Object.class);
					}

					@Override
					public LuaState getLuaState() {
						return luaValueProxy.getLuaState();
					}

					@Override
					public void pushValue() {
						luaValueProxy.pushValue();
					}
				};
			}
			if (formalType == List.class) {
				final LuaValueProxy luaValueProxy = luaState.getProxy(index);
				return (T) new AbstractTableList() {
					@Override
					public LuaState getLuaState() {
						return luaValueProxy.getLuaState();
					}

					@Override
					public void pushValue() {
						luaValueProxy.pushValue();
					}
				};
			}
			if (formalType.isArray()) {
				int length = luaState.length(index);
				Class<?> componentType = formalType.getComponentType();
				Object array = Array.newInstance(formalType.getComponentType(),
						length);
				for (int i = 0; i < length; i++) {
					luaState.rawGet(index, i + 1);
					try {
						Array.set(array, i, convertLuaValue(luaState, -1,
								componentType));
					} finally {
						luaState.pop(1);
					}
				}
				return (T) array;
			}
			break;
		case FUNCTION:
			if (luaState.isJavaFunction(index)) {
				if (formalType == JavaFunction.class
						|| formalType == Object.class) {
					return (T) luaState.toJavaFunction(index);
				}
			}
			break;
		case USERDATA:
			Object object = luaState.toJavaObjectRaw(index);
			if (object != null) {
				if (object instanceof TypedJavaObject) {
					TypedJavaObject typedJavaObject = (TypedJavaObject) object;
					if (typedJavaObject.isStrong()) {
						if (formalType.isAssignableFrom(typedJavaObject
								.getClass())) {
							return (T) typedJavaObject;
						}
					}
					return (T) ((TypedJavaObject) object).getObject();
				} else {
					return (T) object;
				}
			}
			break;
		}

		// Handle object
		if (formalType == Object.class) {
			return (T) luaState.getProxy(index);
		}

		// Unsupported conversion
		throw new ClassCastException(String.format("cannot convert %s to %s",
				luaState.typeName(index), formalType.getCanonicalName()));
	}

	@SuppressWarnings("unchecked")
	@Override
	public void convertJavaObject(LuaState luaState, Object object) {
		// Handle null
		if (object == null) {
			luaState.pushNil();
			return;
		}

		// Handle known Java types
		JavaObjectConverter<Object> javaObjectConverter = (JavaObjectConverter<Object>) JAVA_OBJECT_CONVERTERS
				.get(object.getClass());
		if (javaObjectConverter != null) {
			javaObjectConverter.convert(luaState, object);
			return;
		}
		if (object instanceof JavaFunction) {
			luaState.pushJavaFunction((JavaFunction) object);
			return;
		}
		if (object instanceof LuaValueProxy) {
			LuaValueProxy luaValueProxy = (LuaValueProxy) object;
			if (!luaValueProxy.getLuaState().equals(luaState)) {
				throw new IllegalArgumentException(
						"Lua value proxy is from a different Lua state");
			}
			luaValueProxy.pushValue();
			return;
		}

		// Push as is
		luaState.pushJavaObjectRaw(object);
	}

	// -- Nested types
	/**
	 * Converts Lua values.
	 */
	private interface LuaValueConverter<T> {
		/**
		 * Converts a Lua value to a Java object.
		 */
		public T convert(LuaState luaState, int index);
	}

	/**
	 * Converts Java object.
	 */
	private interface JavaObjectConverter<T> {
		/**
		 * Converts a Java object to a Lua value.
		 */
		public void convert(LuaState luaState, T object);
	}
}
