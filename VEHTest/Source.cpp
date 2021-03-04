#include <Windows.h>
#include <iostream>
#include "HWBP.hpp"

LONG WINAPI VectoredExceptionHandler1(EXCEPTION_POINTERS* ExceptionInfo)
{
#ifdef _WIN64
	std::cout << std::hex << "RIP: 0x" << ExceptionInfo->ContextRecord->Rip << std::endl;
#else
	std::cout << std::hex << "EIP: 0x" << ExceptionInfo->ContextRecord->Eip << std::endl;
#endif

	if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
	{
		std::cout << "Before function call"<< std::endl;

		ExceptionInfo->ContextRecord->EFlags |= 0x10000;
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void HelloWorld()
{
	std::cout << "Hello world!" << std::endl;
}

void Test1()
{
	auto address = reinterpret_cast<uint64_t>(&HelloWorld);
	auto handle = AddVectoredExceptionHandler(0, VectoredExceptionHandler1);
	if (handle != nullptr)
	{
		auto threadHandle = GetCurrentThread();
		HWBP::SetBreakPoint(threadHandle, address, Register::Dr0, Condition::Execution, Length::Byte, Status::Enabled);
		HelloWorld();
		HWBP::SetBreakPoint(threadHandle, 0, Register::Dr0, Condition::Execution, Length::Byte, Status::Disabled);
		RemoveVectoredExceptionHandler(handle);
	}
}

LONG WINAPI VectoredExceptionHandler2(EXCEPTION_POINTERS* ExceptionInfo)
{
	std::cout << "Before value change" << std::endl;

#ifdef _WIN64
	std::cout << std::hex << "RIP: 0x" <<ExceptionInfo->ContextRecord->Rip << std::endl;
#else
	std::cout << std::hex << "EIP: 0x" << ExceptionInfo->ContextRecord->Eip << std::endl;
#endif
	
	return EXCEPTION_CONTINUE_EXECUTION;
}

void Test2()
{
	auto value = 0;
	auto address = reinterpret_cast<uint64_t>(&value);

	auto handle = AddVectoredExceptionHandler(0, VectoredExceptionHandler2);
	if (handle != nullptr)
	{
		auto threadHandle = GetCurrentThread();
		HWBP::SetBreakPoint(threadHandle, address, Register::Dr0, Condition::Write, Length::DWORD, Status::Enabled);
		value = 1;
		HWBP::SetBreakPoint(threadHandle, 0, Register::Dr0, Condition::Execution, Length::Byte, Status::Disabled);
		RemoveVectoredExceptionHandler(VectoredExceptionHandler2);
	}
}

int main()
{
	Test1();
	Test2();

	return 0;
}
