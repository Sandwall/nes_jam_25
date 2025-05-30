#pragma once

/* tinydefs.hpp
 * =============
 * This is a header that defines shorthand names for common primitives
 * as well as declares useful math constants.
 */

#if defined(_WIN32)
#define USING_WIN32
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define USING_UNIX
#endif

#include <stdint.h>
#include <assert.h>
using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
using c8 = char;
using c32 = char32_t;

/* Tiny TyPes
 * 
 * NOTE(sand): I was trying to use this C++20 feature to implement filerp so that
 * a templated overload of it only accepts integer types, but I eventually gave up
 * and decided to just change the parameter I was trying to pass to filerp
 * (computer.mouseX/Y) to f32s.
 * Maybe this chunk will come in handy later on. Who knows :P
 */
#include <string_view>
namespace ttp {
	// mostly copied from https://stackoverflow.com/a/56600402. just de-enterprised it.
	template<typename T>
	static constexpr std::string_view type_name();

	template <>
	static constexpr std::string_view type_name<void>() {
		return "void";
	}
	
	template<typename T>
	static constexpr std::string_view wrapped_type_name() {
		return __FUNCSIG__;
	}
	
	template <typename T>
	static constexpr std::string_view type_name() {
		constexpr std::string_view wrappedName = wrapped_type_name<T>();
		constexpr u16 beforeTypeNameLength = (u16)(wrapped_type_name<void>().find(type_name<void>()));
		constexpr u16 afterTypeNameLength = (u16)(wrapped_type_name<void>().length() - beforeTypeNameLength - type_name<void>().length());
		constexpr u16 typeNameLength = (u16)(wrappedName.length() - beforeTypeNameLength - afterTypeNameLength);
		return wrappedName.substr(beforeTypeNameLength, typeNameLength);
	}

	template<typename T, typename S>
	struct same {
		static constexpr bool areThey() { return type_name<T>() == type_name<S>(); };
	};

	template<typename T, typename S>
	concept SameAs = same<T, S>::areThey();

	template<typename T>
	concept SignedIntegral =
		SameAs<T, i8> ||
		SameAs<T, i16> ||
		SameAs<T, i32> ||
		SameAs<T, i64> ||
		SameAs<T, ptrdiff_t>;

	template<typename T>
	concept UnsignedIntegral =
		SameAs<T, u8> ||
		SameAs<T, u16> ||
		SameAs<T, u32> ||
		SameAs<T, u64> ||
		SameAs<T, size_t>;

	template<typename T>
	concept Integral = SignedIntegral<T> || UnsignedIntegral<T>;

	template<typename T>
	concept Floating = SameAs<T, f32> || SameAs<T, f64>;
}


/*
template <typename T>
union T2 {
	T v[2];
	struct { T x, y; };
};

template <typename T>
union T4 {
	T v[4];
	struct { T x, y, w, h; };
	struct { T r, g, b, a; };
	struct { T x0, y0, x1, y1; };
};

using Float2 = T2<f32>;
using Vec2F = T2<f32>;
using Vec2I = T2<i32>;

using Float4 = T4<f32>;
using RectF = T4<f32>;
using ColorF = T4<f32>;
using LineF = T4<f32>;
*/

#define VEC3_AXIS_X 1.0f, 0.0f, 0.0f
#define VEC3_AXIS_Y 0.0f, 1.0f, 0.0f
#define VEC3_AXIS_Z 0.0f, 0.0f, 1.0f

#define COLOR_BLACK     0.0f, 0.0f, 0.0f
#define COLOR_RED       1.0f, 0.0f, 0.0f
#define COLOR_GREEN     0.0f, 1.0f, 0.0f
#define COLOR_BLUE      0.0f, 0.0f, 1.0f
#define COLOR_YELLOW    1.0f, 1.0f, 0.0f
#define COLOR_MAGENTA   1.0f, 0.0f, 1.0f
#define COLOR_CYAN      0.0f, 1.0f, 1.0f
#define COLOR_WHITE     1.0f, 1.0f, 1.0f

#define U8COLOR_BLACK   000u, 000u, 000u
#define U8COLOR_RED     255u, 000u, 000u
#define U8COLOR_GREEN   000u, 255u, 000u
#define U8COLOR_BLUE    000u, 000u, 255u
#define U8COLOR_YELLOW  255u, 255u, 000u
#define U8COLOR_MAGENTA 255u, 000u, 255u
#define U8COLOR_CYAN    000u, 255u, 255u
#define U8COLOR_WHITE   255u, 255u, 255u

// TIM = TIny Math
namespace tim {
	constexpr f32 pi = 3.1415926535f;
	constexpr f32 tau = pi * 2.0f;

	constexpr i32 wrap_around(i32 x, i32 max) {
		if (x >= max) return x % max;
		if (x < 0) return max + x - 1;
		return x;
	}

	template <typename T>
	constexpr T clamp(T x, T min, T max) {
		if (x >= max) return max;
		if (x <= min) return min;
		return x;
	}

	// This function extends clamp to strictly keep a value in between a range
	// doesn't matter the order of the range arguments
	template <typename T>
	constexpr T between(T x, T side1, T side2) {
		if (side1 == side2) return side1;
		return (side2 < side1) ?
			clamp(x, side2, side1) :
			clamp(x, side1, side2);
	}

	// only works for signed integrals because... well...
	template<typename T>
	constexpr T abs(T x) requires ttp::SignedIntegral<T> {
		return x > 0 ? x : -x;
	}

	// takes a position and a length, and just returns an index into a circular buffer
	template<typename T>
	constexpr T circ_idx(T i, T len) requires ttp::Integral<T> {
		T result = i % len;
		return result < 0 ? result + len : result;
	}

	// Frame-independent lerp smoothing
	// thx freya holmer https://youtu.be/LSNQuFEDOyQ?si=mk9QMjab57lxyNkM&t=2978
	// Decay is recommended to be [1,25] from "slow to fast"
#ifdef TINYDEF_EXPF
	f32 filerp32(f32 current, f32 target, f32 decay, f32 dt) {
		return target + (current - target) * TINYDEF_EXPF(-decay * dt);
	}
#endif

#ifdef TINYDEF_EXP
	f64 filerp64(f64 current, f64 target, f64 decay, f64 dt) {
		return target + (current - target) * TINYDEF_EXP(-decay * dt);
	}
#endif

	template<ttp::UnsignedIntegral _UnsignedT>
	static constexpr _UnsignedT b_to_max(bool value) {
		return ((_UnsignedT)!value) - 1;
	}

	template<ttp::UnsignedIntegral _UnsignedT>
	static constexpr _UnsignedT set_flag(_UnsignedT flagPole, _UnsignedT flag, bool value) {
		return (b_to_max<_UnsignedT>(value) & flag) | (flagPole & ~flag);
	}

}

// TDS = Tiny Data Structures
namespace tds {
	// this ministruct allows treating data as a circular buffer
	template<typename T>
	struct RingSlice {
		T* data;
		size_t len;

		T& operator[](i64 i) {
			return data[tim::circ_idx(i, len)];
		}
	};

	template<typename T, u32 Capacity>
	struct Stack {
		static constexpr u32 capacity = Capacity;
		u32 size;
		T data[Capacity];

		void reset() {
			memset(data, 0, sizeof(T) * Capacity);
			size = 0;
		}

		bool push(T t) {
			assert(size < capacity);
			data[size++] = t;
		}

		T pop() {
			assert(size > 0);
			return data[size--];
		}

		T peek(u32 pos) {
			if (size <= 0) return 0;
			return data[size - 1];
		}
	};

	template<u32 NumStates>
	struct StateMachine {
		static constexpr u32 maxStates = NumStates;
		using StateFunction = void(*)(StateMachine<NumStates>&);

		u32 prevState = 0;
		u32 state = 0;
		u32 nextState = 0;

		// These can be set to true in case we want to tell the state machine
		// to call the enter_state or exit_state functions again for some reason
		bool signalEnter = false;
		bool signalExit = false;

		// [i][0] = enter_state
		// [i][1] = update_state
		// [i][2] = exit_state
		StateFunction stateTable[NumStates][3];
		StateFunction onEnter = nullptr;  // global state enter function
		StateFunction onExit = nullptr;   // global state exit function

#define ASSERT_STATE_VALIDITY \
	assert(state < maxStates); \
	assert(nextState < maxStates);

		void update() {
			// enter_state is called
			ASSERT_STATE_VALIDITY
			if (nextState != state || signalEnter) {
				StateFunction& sf = stateTable[nextState][0];
				if (sf) sf(*this);
				if (onEnter) onEnter(*this);

				signalEnter = false;
				prevState = state;
				state = nextState;
			}

			// update_state (this is expected to change states by setting nextState)
			ASSERT_STATE_VALIDITY
			{
				StateFunction &sf = stateTable[state][1];
				if (sf) sf(*this);
			}

			// exit_state
			ASSERT_STATE_VALIDITY
			if (nextState != state || signalExit) {
				StateFunction& sf = stateTable[state][2];
				if (sf) sf(*this);
				if (onExit) onExit(*this);

				signalExit = false;
			}
		}
	};
}
