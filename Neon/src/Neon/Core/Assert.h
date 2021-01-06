#pragma once

#ifdef NEO_DEBUG
	#define NEO_ASSERT_NO_MESSAGE(condition)                                                                                       \
		{                                                                                                                          \
			if (!(condition))                                                                                                      \
			{                                                                                                                      \
				NEO_ERROR("Assertion Failed");                                                                                     \
				__debugbreak();                                                                                                    \
			}                                                                                                                      \
		}
	#define NEO_ASSERT_MESSAGE(condition, ...)                                                                                     \
		{                                                                                                                          \
			if (!(condition))                                                                                                      \
			{                                                                                                                      \
				NEO_ERROR("Assertion Failed: {0}", __VA_ARGS__);                                                                   \
				__debugbreak();                                                                                                    \
			}                                                                                                                      \
		}

	#define NEO_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro
	#define NEO_GET_ASSERT_MACRO(...) NEO_EXPAND_VARGS(NEO_ASSERT_RESOLVE(__VA_ARGS__, NEO_ASSERT_MESSAGE, NEO_ASSERT_NO_MESSAGE))

	#define NEO_ASSERT(...) NEO_EXPAND_VARGS(NEO_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__))
	#define NEO_CORE_ASSERT(...) NEO_EXPAND_VARGS(NEO_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__))
#else
	#define NEO_ASSERT(...)
	#define NEO_CORE_ASSERT(...)
#endif
