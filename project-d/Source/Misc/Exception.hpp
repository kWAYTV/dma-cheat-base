#pragma once

class c_exception_handler
{
public:
	static void log_file(const string& output)
	{
		const string file_name = "crashlog.txt";

		if (filesystem::exists(file_name))
		{
			if (!filesystem::remove(file_name))
			{
				LOG_ERROR("Failed to remove existing file '%s'", file_name.c_str());
				return;
			}
		}

		ofstream output_file(file_name);
		if (!output_file)
		{
			LOG_ERROR("Failed to open '%s' for writing", file_name.c_str());
			return;
		}

		output_file << output << endl;
	}

	static long __stdcall handler(EXCEPTION_POINTERS* info)
	{
		const auto code = info->ExceptionRecord->ExceptionCode;
		if (code != EXCEPTION_ACCESS_VIOLATION)
			return 0;

		const auto address = info->ExceptionRecord->ExceptionAddress;
		MEMORY_BASIC_INFORMATION memory_info{};
		uintptr_t alloc_base{};

		if (VirtualQuery(address, &memory_info, sizeof(memory_info)))
			alloc_base = reinterpret_cast<uintptr_t>(memory_info.AllocationBase);

		const auto output = format("App crashed at usermode.exe+{:#04x}", static_cast<uintptr_t>(info->ContextRecord->Rip) - alloc_base);
		log_file(output);

		return 0;
	}

	static bool setup()
	{
		const auto handle = AddVectoredExceptionHandler(false, handler);
		if (!handle)
		{
			LOG_ERROR("Failed to add vectored exception handler");
			return false;
		}

		return true;
	}
};