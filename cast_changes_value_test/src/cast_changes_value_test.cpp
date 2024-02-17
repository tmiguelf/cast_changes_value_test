//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		
//======== ======== ======== ======== ======== ======== ======== ========

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tuple>
#include <cstdint>
#include <string_view>


template<typename T>
auto safe_print_cast(T const val)
{
	if constexpr(std::is_same_v<T, int8_t>)
	{
		return static_cast<int16_t>(val);
	}
	else if constexpr(std::is_same_v<T, uint8_t>)
	{
		return static_cast<uint16_t>(val);
	}
	else
	{
		return val;
	}
}

template<typename T>
std::string_view get_fancy_name()
{
#define AUTO_CODE_GEN(X) if constexpr (std::is_same_v<T, X>) { return #X; }
	AUTO_CODE_GEN(int8_t);
	AUTO_CODE_GEN(int16_t);
	AUTO_CODE_GEN(int32_t);
	AUTO_CODE_GEN(int64_t);
	AUTO_CODE_GEN(uint8_t);
	AUTO_CODE_GEN(uint16_t);
	AUTO_CODE_GEN(uint32_t);
	AUTO_CODE_GEN(uint64_t);
#undef AUTO_CODE_GEN
}



template<class From_t, class To_t, From_t Value, bool Result>
struct test_case_t
{
	using from_t = From_t;
	using to_t = To_t;
	static constexpr from_t value = Value;
	static constexpr bool can_convert = Result;
};

using test_types_t = ::testing::Types<
	test_case_t<uint8_t, int8_t, 0, true>,
	test_case_t<uint8_t, int8_t, 127, true>,
	test_case_t<uint8_t, int8_t, 128, false>,
	test_case_t<uint8_t, uint64_t, 255, true>,

	test_case_t<int8_t, uint8_t, 0, true>,
	test_case_t<int8_t, uint8_t, 127, true>,
	test_case_t<int8_t, uint8_t, -1, false>,

	test_case_t<int8_t, int64_t, 0, true>,
	test_case_t<int8_t, int64_t, -128, true>,
	test_case_t<int8_t, int64_t, 127, true>,

	test_case_t<int8_t, uint64_t, 0, true>,
	test_case_t<int8_t, uint64_t, -1, false>,
	test_case_t<int8_t, uint64_t, 127, true>,

	test_case_t<uint64_t, int8_t, 0, true>,
	test_case_t<uint64_t, int8_t, 127, true>,
	test_case_t<uint64_t, int8_t, 128, false>,

	test_case_t<uint64_t, uint8_t, 0, true>,
	test_case_t<uint64_t, uint8_t, 255, true>,
	test_case_t<uint64_t, uint8_t, 256, false>,

	test_case_t<int64_t, int8_t, 0, true>,
	test_case_t<int64_t, int8_t, 127, true>,
	test_case_t<int64_t, int8_t, 128, false>,
	test_case_t<int64_t, int8_t, -128, true>,
	test_case_t<int64_t, int8_t, -129, false>,

	test_case_t<int64_t, uint8_t, 0, true>,
	test_case_t<int64_t, uint8_t, 255, true>,
	test_case_t<int64_t, uint8_t, 256, false>,
	test_case_t<int64_t, uint8_t, -1, false>,

	test_case_t<int64_t, uint64_t, 0, true>,
	test_case_t<int64_t, uint64_t, std::numeric_limits<int64_t>::max(), true>,
	test_case_t<int64_t, uint64_t, -1, false>
	>;



template<typename T>
class cast_test_suit_t : public testing::Test {
protected:
	cast_test_suit_t() {}
};

TYPED_TEST_SUITE(cast_test_suit_t, test_types_t);


template<class T, class U>
[[nodiscard]] constexpr bool would_cast_modify([[maybe_unused]] U const x)
{
	if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
	{
		if constexpr (sizeof(T) >= sizeof(U))
		{
			return false;
		}
		else
		{
			//return !(static_cast<U>(static_cast<T>(x)) == x);
			if constexpr(std::is_signed_v<T>)
			{
				
				return (x < std::numeric_limits<T>::min()) || (x > std::numeric_limits<T>::max());
			}
			else
			{
				return (x > std::numeric_limits<T>::max());
			}
		}

	}
	else
	{
		if constexpr(std::is_signed_v<T>)
		{
			return x > std::numeric_limits<T>::max();
		}
		else
		{
			if constexpr(sizeof(T) >= sizeof(U))
			{
				return (x < 0);
			}
			else
			{
				return (x < 0) || x > std::numeric_limits<T>::max();
			}
		}
	}
}

#pragma warning( push )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4389 )
template<class T, class U>
[[nodiscard]] constexpr bool check_by_limit_comparison([[maybe_unused]] U const x)
{
	return x < std::numeric_limits<T>::min() || x > std::numeric_limits<T>::max();
}
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4389 )
template<class T, class U>
[[nodiscard]] constexpr bool check_by_cast([[maybe_unused]] U const x)
{
	return !(static_cast<T>(x) == x);
}
#pragma warning( pop )

#define PRE_BOILER_PLATE \
using from_t                = typename TypeParam::from_t; \
using to_t                  = typename TypeParam::to_t; \
constexpr from_t value      = TypeParam::value; \
constexpr bool can_convert  = TypeParam::can_convert;

#define POST_BOILER_PLATE \
ASSERT_EQ(result, can_convert) \
<< "Value " << safe_print_cast(value) \
<< " of type " << get_fancy_name<from_t>() << " is " << (can_convert ? "" : "not ") \
<< "convertible to type " << get_fancy_name<to_t>() \
<< " but function reported as " << (result ? "" : "not ") << "convertible";


TYPED_TEST(cast_test_suit_t, would_cast_modify_test)
{
	PRE_BOILER_PLATE

	bool const result = !would_cast_modify<to_t>(value);

	POST_BOILER_PLATE
}

TYPED_TEST(cast_test_suit_t, check_by_limit_comparison_test)
{
	PRE_BOILER_PLATE

	bool const result = !check_by_limit_comparison<to_t>(value);

	POST_BOILER_PLATE
}

TYPED_TEST(cast_test_suit_t, check_by_cast_test)
{
	PRE_BOILER_PLATE

		bool const result = !check_by_cast<to_t>(value);

	POST_BOILER_PLATE
}

#undef PRE_BOILER_PLATE
#undef POST_BOILER_PLATE
