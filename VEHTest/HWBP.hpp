#pragma once

enum class Status
{
	Disabled = 0,
	Enabled = 1
};

enum class Register
{
	Dr0 = 0,
	Dr1 = 1,
	Dr2 = 2,
	Dr3 = 3
};

enum class Condition
{
	Execution = 0,
	Write = 1,
	IOReadWrite = 2,
	ReadWrite = 3
};

enum class Length
{
	Byte = 0,
	WORD = 1,
	QWORD = 2,
	DWORD = 3
};

class HWBP
{
public:
	static void SetBreakPoint(uint64_t address, Register _register, Condition condition, Length length, Status status)
	{
#ifdef _WIN64
		CONTEXT context = { 0 };
#else
		WOW64_CONTEXT context = { 0 };
#endif
		context.ContextFlags = CONTEXT_ALL;

#ifdef _WIN64
		GetThreadContext(GetCurrentThread(), &context);
#else
		Wow64GetThreadContext(GetCurrentThread(), &context);
#endif

		if (condition == Condition::Execution && length != Length::Byte)
		{
			throw std::exception("When condition is \"Execution\", length must be \"Byte\"");
		}

		switch (_register)
		{
		case Register::Dr0:
#ifdef _WIN64
			context.Dr0 = address;
#else
			context.Dr0 = static_cast<uint32_t>(address);
#endif
			break;
		case Register::Dr1:
#ifdef _WIN64
			context.Dr1 = address;
#else
			context.Dr1 = static_cast<uint32_t>(address);
#endif
			break;
		case Register::Dr2:
#ifdef _WIN64
			context.Dr2 = address;
#else
			context.Dr2 = static_cast<uint32_t>(address);
#endif
			break;
		case Register::Dr3:
#ifdef _WIN64
			context.Dr3 = address;
#else
			context.Dr3 = static_cast<uint32_t>(address);
#endif
			break;
		}

		auto index = static_cast<uint64_t>(_register);
		context.Dr7 |= static_cast<uint64_t>(status) << (2 * index);
		context.Dr7 |= static_cast<uint64_t>(condition) << (4 * index + 16);
		context.Dr7 |= static_cast<uint64_t>(length) << (4 * index + 18);

		// Recommended that LE, GE flags be set to 1(see Intel x64 document)
		context.Dr7 |= static_cast<uint64_t>(1) << 8;
		context.Dr7 |= static_cast<uint64_t>(1) << 9;

		// Reserved
		context.Dr7 |= static_cast<uint64_t>(1) << 10;
		context.Dr7 |= static_cast<uint64_t>(0) << 12;
		context.Dr7 |= static_cast<uint64_t>(0) << 14;
		context.Dr7 |= static_cast<uint64_t>(0) << 15;

		context.Dr6 = 0;
		context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

#ifdef _WIN64
		SetThreadContext(GetCurrentThread(), &context);
#else
		Wow64SetThreadContext(GetCurrentThread(), &context);
#endif
	}
};