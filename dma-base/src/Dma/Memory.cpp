#include <Pch.h>
#include "Memory.h"

#include <thread>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <vector>

Memory::Memory()
{
	//INFO("Loading libraries...");
	modules.VMM = LoadLibraryA("vmm.dll");
	modules.FTD3XX = LoadLibraryA("FTD3XX.dll");
	modules.LEECHCORE = LoadLibraryA("leechcore.dll");

	if (!modules.VMM || !modules.FTD3XX || !modules.LEECHCORE)
	{
		//INFO("VMM: {}", static_cast<void*>(modules.VMM));
		//INFO("FTD: {}", static_cast<void*>(modules.FTD3XX));
		//INFO("LEECH: {}", static_cast<void*>(modules.LEECHCORE));
		DebugBreak();
	}

	INFO("Successfully loaded libraries!");
}

Memory::~Memory()
{
	VMMDLL_Close(this->vHandle);
	DMA_INITIALIZED = false;
	PROCESS_INITIALIZED = false;
}

bool Memory::DumpMemoryMap(bool debug)
{
	LPSTR args[] = { (LPSTR)"", (LPSTR)"-device", (LPSTR)"fpga://algo=0", (LPSTR)"", (LPSTR)"" };
	int argc = 3;
	if (debug)
	{
		args[argc++] = (LPSTR)"-v";
		args[argc++] = (LPSTR)"-printf";
	}

	VMM_HANDLE handle = VMMDLL_Initialize(argc, args);
	if (!handle)
	{
		ERROR("Failed to open a VMM Handle");
		return false;
	}

	PVMMDLL_MAP_PHYSMEM pPhysMemMap = NULL;
	if (!VMMDLL_Map_GetPhysMem(handle, &pPhysMemMap))
	{
		ERROR("Failed to get physical memory map");
		VMMDLL_Close(handle);
		return false;
	}

	if (pPhysMemMap->dwVersion != VMMDLL_MAP_PHYSMEM_VERSION)
	{
		ERROR("Invalid VMM Map Version");
		VMMDLL_MemFree(pPhysMemMap);
		VMMDLL_Close(handle);
		return false;
	}

	if (pPhysMemMap->cMap == 0)
	{
		ERROR("Failed to get physical memory map");
		VMMDLL_MemFree(pPhysMemMap);
		VMMDLL_Close(handle);
		return false;
	}
	//Dump map to file
	std::stringstream sb;
	for (DWORD i = 0; i < pPhysMemMap->cMap; i++)
	{
		sb << std::setfill('0') << std::setw(4) << i << "  " << std::hex << pPhysMemMap->pMap[i].pa << "  -  " << (pPhysMemMap->pMap[i].pa + pPhysMemMap->pMap[i].cb - 1) << "  ->  " << pPhysMemMap->pMap[i].pa << std::endl;
	}

	auto temp_path = std::filesystem::temp_directory_path();
	std::ofstream nFile(temp_path.string() + "\\mmap.txt");
	nFile << sb.str();
	nFile.close();

	VMMDLL_MemFree(pPhysMemMap);
	INFO("Successfully dumped memory map to file!");
	//Little sleep to make sure it's written to file.
	Sleep(3000);
	VMMDLL_Close(handle);
	return true;
}

unsigned char abort2[4] = { 0x10, 0x00, 0x10, 0x00 };

bool Memory::SetFPGA()
{
	bool result;
	ULONG64 qwID = 0;
	ULONG64 qwVersionMajor = 0;
	ULONG64 qwVersionMinor = 0;
	if (!VMMDLL_ConfigGet(this->vHandle, LC_OPT_FPGA_FPGA_ID, &qwID) && VMMDLL_ConfigGet(this->vHandle, LC_OPT_FPGA_VERSION_MAJOR, &qwVersionMajor) && VMMDLL_ConfigGet(this->vHandle, LC_OPT_FPGA_VERSION_MINOR, &qwVersionMinor))
	{
		ERROR("Failed to lookup FPGA device, Attempting to proceed");
		return false;
	}

	//INFO("VMMDLL_ConfigGet");
	//INFO("ID = {}", qwID);
	//INFO("VERSION = {}.{}", qwVersionMajor, qwVersionMinor);

	if ((qwVersionMajor >= 4) && ((qwVersionMajor >= 5) || (qwVersionMinor >= 7)))
	{
		HANDLE handle;
		LC_CONFIG config = { .dwVersion = LC_CONFIG_VERSION, .szDevice = "existing" };
		handle = LcCreate(&config);
		if (!handle)
		{
			ERROR("Failed to create FPGA device");
			return false;
		}
		
		LcCommand(handle, LC_CMD_FPGA_CFGREGPCIE_MARKWR | 0x002, 4, (PBYTE)&abort2, NULL, NULL);
		//INFO("[-] Register auto cleared");
		LcClose(handle);
	}

	return true;
}

bool Memory::Init(std::string process_name, bool memMap, bool debug)
{
	if (!DMA_INITIALIZED)
	{
		INFO("Inizializing...");
	reinit:
		LPSTR args[] = { (LPSTR)"", (LPSTR)"-device", (LPSTR)"fpga://algo=0", (LPSTR)"", (LPSTR)"", (LPSTR)"", (LPSTR)"" };
		DWORD argc = 3;
		if (debug)
		{
			args[argc++] = (LPSTR)"-v";
			args[argc++] = (LPSTR)"-printf";
		}

		std::string path = "";
		if (memMap)
		{
			auto temp_path = std::filesystem::temp_directory_path();
			path = (temp_path.string() + "\\mmap.txt");
			bool dumped = false;
			if (!std::filesystem::exists(path))
				dumped = this->DumpMemoryMap(debug);
			else
				dumped = true;
			INFO("Dumping memory map to file...");
			if (!dumped)
			{
				WARN("Could not dump memory map! Defaulting to no memory map!");
			}
			else
			{
				INFO("Dumped memory map!");

				//Add the memory map to the arguments and increase arg count.
				args[argc++] = (LPSTR)"-memmap";
				args[argc++] = (LPSTR)path.c_str();
			}
		}
		this->vHandle = VMMDLL_Initialize(argc, args);
		if (!this->vHandle)
		{
			if (memMap)
			{
				memMap = false;
				ERROR("Initialization failed with Memory map? Try without MMap");
				goto reinit;
			}
			ERROR("Initialization failed! Is the DMA in use or disconnected?");
			return false;
		}

		ULONG64 FPGA_ID, DEVICE_ID;

		VMMDLL_ConfigGet(this->vHandle, LC_OPT_FPGA_FPGA_ID, &FPGA_ID);
		VMMDLL_ConfigGet(this->vHandle, LC_OPT_FPGA_DEVICE_ID, &DEVICE_ID);

		INFO("FPGA ID: {} - DEVICE ID: {}", FPGA_ID, DEVICE_ID);

		if (!this->SetFPGA())
		{
			ERROR("Could not set FPGA!");
			VMMDLL_Close(this->vHandle);
			return false;
		}

		DMA_INITIALIZED = TRUE;
	}
	else
		WARN("DMA already initialized!");

	if (PROCESS_INITIALIZED)
	{
		WARN("Process already initialized!");
		return true;
	}

	this->current_process.PID = GetPidFromName(process_name);
	if (!this->current_process.PID)
	{
		ERROR("Could not get PID from name!");
		return false;
	}
	this->current_process.process_name = process_name;
	if (!Mem.FixCr3())
		ERROR("Failed to fix CR3");
	else
		INFO("CR3 fixed");

	this->current_process.base_address = GetBaseAddress(process_name);
	if (!this->current_process.base_address)
	{
		ERROR("Could not get base address!");
		return false;
	}

	this->current_process.base_size = GetBaseSize(process_name);
	if (!this->current_process.base_size)
	{
		ERROR("Could not get base size!");
		return false;
	}

	INFO("Process information of {}", process_name.c_str());
	INFO("  PID: {}", this->current_process.PID);
	INFO("  Base Address: 0x{}", this->current_process.base_address);
	INFO("  Base Size: 0x{}", this->current_process.base_size);

	PROCESS_INITIALIZED = TRUE;

	return true;
}

DWORD Memory::GetPidFromName(std::string process_name)
{
	DWORD pid = 0;
	VMMDLL_PidGetFromName(this->vHandle, (LPSTR)process_name.c_str(), &pid);
	return pid;
}

std::vector<int> Memory::GetPidListFromName(std::string name)
{
	PVMMDLL_PROCESS_INFORMATION process_info = NULL;
	DWORD total_processes = 0;
	std::vector<int> list = { };

	if (!VMMDLL_ProcessGetInformationAll(this->vHandle, &process_info, &total_processes))
	{
		ERROR("Failed to get process list");
		return list;
	}

	for (size_t i = 0; i < total_processes; i++)
	{
		auto process = process_info[i];
		if (strstr(process.szNameLong, name.c_str()))
			list.push_back(process.dwPID);
	}

	return list;
}

std::vector<std::string> Memory::GetModuleList(std::string process_name)
{
	std::vector<std::string> list = { };
	PVMMDLL_MAP_MODULE module_info;
	if (!VMMDLL_Map_GetModuleU(this->vHandle, this->current_process.PID, &module_info, VMMDLL_MODULE_FLAG_NORMAL))
	{
		ERROR("Failed to get module list");
		return list;
	}

	for (size_t i = 0; i < module_info->cMap; i++)
	{
		auto module = module_info->pMap[i];
		list.push_back(module.uszText);
	}

	return list;
}

VMMDLL_PROCESS_INFORMATION Memory::GetProcessInformation()
{
	VMMDLL_PROCESS_INFORMATION info;
	SIZE_T process_information = sizeof(VMMDLL_PROCESS_INFORMATION);
	ZeroMemory(&info, sizeof(VMMDLL_PROCESS_INFORMATION));
	info.magic = VMMDLL_PROCESS_INFORMATION_MAGIC;
	info.wVersion = VMMDLL_PROCESS_INFORMATION_VERSION;

	if (!VMMDLL_ProcessGetInformation(this->vHandle, this->current_process.PID, &info, &process_information))
	{
		ERROR("Failed to find process information");
		return { };
	}

	INFO("Found process information");
	return info;
}

size_t Memory::GetBaseAddress(std::string module_name)
{
	std::wstring str(module_name.begin(), module_name.end());
	if (!Modules.contains(str))
	{


		PVMMDLL_MAP_MODULEENTRY module_info;
		if (!VMMDLL_Map_GetModuleFromNameW(this->vHandle, this->current_process.PID, (LPWSTR)str.c_str(), &module_info, VMMDLL_MODULE_FLAG_NORMAL))
		{
			ERROR("Couldn't find Base Address for {}", module_name.c_str());
			return 0;
		}

		INFO("Found Base Address for {0} at 0x{1}", module_name.c_str(), module_info->vaBase);
		Modules[str] = module_info->vaBase;
		return module_info->vaBase;
	}
	else
	{
		return Modules[str];
	}

}

size_t Memory::GetBaseSize(std::string module_name)
{
	std::wstring str(module_name.begin(), module_name.end());

	PVMMDLL_MAP_MODULEENTRY module_info;
	auto bResult = VMMDLL_Map_GetModuleFromNameW(this->vHandle, this->current_process.PID, (LPWSTR)str.c_str(), &module_info, VMMDLL_MODULE_FLAG_NORMAL);
	if (bResult)
	{
		INFO("Found Base Size for {0} at 0x{1}", module_name.c_str(), module_info->cbImageSize);
		return module_info->cbImageSize;
	}

	ERROR("Failed to find Base Size for {}", module_name.c_str());
	return 0;
}

uintptr_t Memory::GetExportTableAddress(std::string import, std::string process, std::string module)
{
	PVMMDLL_MAP_EAT eat_map = NULL;
	PVMMDLL_MAP_EATENTRY export_entry;
	bool result = VMMDLL_Map_GetEATU(Mem.vHandle, Mem.GetPidFromName(process) /*| VMMDLL_PID_PROCESS_WITH_KERNELMEMORY*/, (LPSTR)module.c_str(), &eat_map);
	if (!result)
	{
		ERROR("Failed to get Export Table");
		return 0;
	}

	if (eat_map->dwVersion != VMMDLL_MAP_EAT_VERSION)
	{
		VMMDLL_MemFree(eat_map);
		eat_map = NULL;
		ERROR("Invalid VMM Map Version");
		return 0;
	}

	uintptr_t addr = 0;
	for (int i = 0; i < eat_map->cMap; i++)
	{
		export_entry = eat_map->pMap + i;
		if (strcmp(export_entry->uszFunction, import.c_str()) == 0)
		{
			addr = export_entry->vaFunction;
			break;
		}
	}

	VMMDLL_MemFree(eat_map);
	eat_map = NULL;

	return addr;
}

uintptr_t Memory::GetImportTableAddress(std::string import, std::string process, std::string module)
{
	PVMMDLL_MAP_IAT iat_map = NULL;
	PVMMDLL_MAP_IATENTRY import_entry;
	bool result = VMMDLL_Map_GetIATU(Mem.vHandle, Mem.GetPidFromName(process) /*| VMMDLL_PID_PROCESS_WITH_KERNELMEMORY*/, (LPSTR)module.c_str(), &iat_map);
	if (!result)
	{
		ERROR("Failed to get Import Table");
		return 0;
	}

	if (iat_map->dwVersion != VMMDLL_MAP_IAT_VERSION)
	{
		VMMDLL_MemFree(iat_map);
		iat_map = NULL;
		ERROR("Invalid VMM Map Version");
		return 0;
	}

	uintptr_t addr = 0;
	for (int i = 0; i < iat_map->cMap; i++)
	{
		import_entry = iat_map->pMap + i;
		if (strcmp(import_entry->uszFunction, import.c_str()) == 0)
		{
			addr = import_entry->vaFunction;
			break;
		}
	}

	VMMDLL_MemFree(iat_map);
	iat_map = NULL;

	return addr;
}

uint64_t cbSize = 0x80000;
//callback for VfsFileListU
VOID cbAddFile(_Inout_ HANDLE h, _In_ LPSTR uszName, _In_ ULONG64 cb, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo)
{
	if (strcmp(uszName, "dtb.txt") == 0)
		cbSize = cb;
}

struct Info
{
	uint32_t index;
	uint32_t process_id;
	uint64_t dtb;
	uint64_t kernelAddr;
	std::string name;
};

bool Memory::FixCr3()
{
	PVMMDLL_MAP_MODULEENTRY module_entry;
	bool result = VMMDLL_Map_GetModuleFromNameU(this->vHandle, this->current_process.PID, (LPSTR)this->current_process.process_name.c_str(), &module_entry, NULL);
	if (result)
		return true; //Doesn't need to be patched lol

	if (!VMMDLL_InitializePlugins(this->vHandle))
	{
		ERROR("[-] Failed VMMDLL_InitializePlugins call");
		return false;
	}

	//have to sleep a little or we try reading the file before the plugin initializes fully
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	while (true)
	{
		BYTE bytes[4] = { 0 };
		DWORD i = 0;
		auto nt = VMMDLL_VfsReadW(this->vHandle, (LPWSTR)L"\\misc\\procinfo\\progress_percent.txt", bytes, 3, &i, 0);
		if (nt == VMMDLL_STATUS_SUCCESS && atoi((LPSTR)bytes) == 100)
			break;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	VMMDLL_VFS_FILELIST2 VfsFileList;
	VfsFileList.dwVersion = VMMDLL_VFS_FILELIST_VERSION;
	VfsFileList.h = 0;
	VfsFileList.pfnAddDirectory = 0;
	VfsFileList.pfnAddFile = cbAddFile; //dumb af callback who made this system

	result = VMMDLL_VfsListU(this->vHandle, (LPSTR)"\\misc\\procinfo\\", &VfsFileList);
	if (!result)
		return false;

	//read the data from the txt and parse it
	const size_t buffer_size = cbSize;
	std::unique_ptr<BYTE[]> bytes(new BYTE[buffer_size]);
	DWORD j = 0;
	auto nt = VMMDLL_VfsReadW(this->vHandle, (LPWSTR)L"\\misc\\procinfo\\dtb.txt", bytes.get(), buffer_size - 1, &j, 0);
	if (nt != VMMDLL_STATUS_SUCCESS)
		return false;

	std::vector<uint64_t> possible_dtbs;
	std::string lines(reinterpret_cast<char*>(bytes.get()));
	std::istringstream iss(lines);
	std::string line;

	while (std::getline(iss, line))
	{
		Info info = { };

		std::istringstream info_ss(line);
		if (info_ss >> std::hex >> info.index >> std::dec >> info.process_id >> std::hex >> info.dtb >> info.kernelAddr >> info.name)
		{
			if (info.process_id == 0) //parts that lack a name or have a NULL pid are suspects
				possible_dtbs.push_back(info.dtb);
			if (this->current_process.process_name.find(info.name) != std::string::npos)
				possible_dtbs.push_back(info.dtb);
		}
	}

	//loop over possible dtbs and set the config to use it til we find the correct one
	for (size_t i = 0; i < possible_dtbs.size(); i++)
	{
		auto dtb = possible_dtbs[i];
		VMMDLL_ConfigSet(this->vHandle, VMMDLL_OPT_PROCESS_DTB | this->current_process.PID, dtb);
		result = VMMDLL_Map_GetModuleFromNameU(this->vHandle, this->current_process.PID, (LPSTR)this->current_process.process_name.c_str(), &module_entry, NULL);
		if (result)
		{
			INFO("Patched DTB");
			return true;
		}
	}

	ERROR("[-] Failed to patch module");
	return false;
}

bool Memory::DumpMemory(uintptr_t address, std::string path)
{
	INFO("Memory dumping currently does not rebuild the IAT table, imports will be missing from the dump.");
	IMAGE_DOS_HEADER dos;
	Read(address, &dos, sizeof(IMAGE_DOS_HEADER));

	//Check if memory has a PE 
	if (dos.e_magic != 0x5A4D) //Check if it starts with MZ
	{
		ERROR("[-] Invalid PE Header");
		return false;
	}

	IMAGE_NT_HEADERS64 nt;
	Read(address + dos.e_lfanew, &nt, sizeof(IMAGE_NT_HEADERS64));

	//Sanity check
	if (nt.Signature != IMAGE_NT_SIGNATURE || nt.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		ERROR("[-] Failed signature check");
		return false;
	}

	//Shouldn't change ever. so const 
	const size_t target_size = nt.OptionalHeader.SizeOfImage;
	//Crashes if we don't make it a ptr :(
	auto target = std::unique_ptr<uint8_t[]>(new uint8_t[target_size]);

	//Read whole modules memory
	Read(address, target.get(), target_size);
	auto nt_header = (PIMAGE_NT_HEADERS64)(target.get() + dos.e_lfanew);
	auto sections = (PIMAGE_SECTION_HEADER)(target.get() + dos.e_lfanew + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + nt.FileHeader.SizeOfOptionalHeader);

	for (size_t i = 0; i < nt.FileHeader.NumberOfSections; i++, sections++)
	{
		//Rewrite the file offsets to the virtual addresses
		ERROR("Rewriting file offsets at 0x{0} size 0x{1}", sections->VirtualAddress, sections->Misc.VirtualSize);
		sections->PointerToRawData = sections->VirtualAddress;
		sections->SizeOfRawData = sections->Misc.VirtualSize;
	}

	//Find all modules used by this process
	//auto descriptor = Read<IMAGE_IMPORT_DESCRIPTOR>(address + ntHeader->OptionalHeader.DataDirectory[1].VirtualAddress);

	//int descriptor_count = 0;
	//int thunk_count = 0;

	/*std::vector<ModuleData> modulelist;
	while (descriptor.Name) {
		auto first_thunk = Read<IMAGE_THUNK_DATA>(moduleAddr + descriptor.FirstThunk);
		auto original_first_thunk = Read<IMAGE_THUNK_DATA>(moduleAddr + descriptor.OriginalFirstThunk);
		thunk_count = 0;

		char ModuleName[256];
		ReadMemory(moduleAddr + descriptor.Name, (void*)&ModuleName, 256);

		std::string DllName = ModuleName;

		ModuleData tmpModuleData;

		//if(std::find(modulelist.begin(), modulelist.end(), tmpModuleData) == modulelist.end())
		//	modulelist.push_back(tmpModuleData);
		while (original_first_thunk.u1.AddressOfData) {
			char name[256];
			ReadMemory(moduleAddr + original_first_thunk.u1.AddressOfData + 0x2, (void*)&name, 256);

			std::string str_name = name;
			auto thunk_offset{ thunk_count * sizeof(uintptr_t) };

			//if (str_name.length() > 0)
			//	imports[str_name] = moduleAddr + descriptor.FirstThunk + thunk_offset;

			++thunk_count;
			first_thunk = Read<IMAGE_THUNK_DATA>(moduleAddr + descriptor.FirstThunk + sizeof(IMAGE_THUNK_DATA) * thunk_count);
			original_first_thunk = Read<IMAGE_THUNK_DATA>(moduleAddr + descriptor.OriginalFirstThunk + sizeof(IMAGE_THUNK_DATA) * thunk_count);
		}

		++descriptor_count;
		descriptor = Read<IMAGE_IMPORT_DESCRIPTOR>(moduleAddr + ntHeader->OptionalHeader.DataDirectory[1].VirtualAddress + sizeof(IMAGE_IMPORT_DESCRIPTOR) * descriptor_count);
	}*/

	//Rebuild import table

	//ERROR("Creating new import section");

	//Create New Import Section

	//Build new import Table

	//Dump file
	const auto dumped_file = CreateFileW(std::wstring(path.begin(), path.end()).c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_COMPRESSED, NULL);
	if (dumped_file == INVALID_HANDLE_VALUE)
	{
		ERROR("Failed creating file: {}", GetLastError());
		return false;
	}

	if (!WriteFile(dumped_file, target.get(), static_cast<DWORD>(target_size), NULL, NULL))
	{
		ERROR("Failed writing file: {}", GetLastError());
		CloseHandle(dumped_file);
		return false;
	}

	INFO("Successfully dumped memory at {}", path.c_str());
	CloseHandle(dumped_file);
	return true;
}

static const char* hexdigits =
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\001\002\003\004\005\006\007\010\011\000\000\000\000\000\000"
"\000\012\013\014\015\016\017\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\012\013\014\015\016\017\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000";

static uint8_t GetByte(const char* hex)
{
	return (uint8_t)((hexdigits[hex[0]] << 4) | (hexdigits[hex[1]]));
}

uint64_t Memory::FindSignature(const char* signature, uint64_t range_start, uint64_t range_end, int PID)
{
	if (!signature || signature[0] == '\0' || range_start >= range_end)
		return 0;

	if (PID == 0)
		PID = this->current_process.PID;

	std::vector<uint8_t> buffer(range_end - range_start);
	if (!VMMDLL_MemReadEx(this->vHandle, PID, range_start, buffer.data(), buffer.size(), 0, VMMDLL_FLAG_NOCACHE))
		return 0;

	const char* pat = signature;
	uint64_t first_match = 0;
	for (uint64_t i = range_start; i < range_end; i++)
	{
		if (*pat == '?' || buffer[i - range_start] == GetByte(pat))
		{
			if (!first_match)
				first_match = i;

			if (!pat[2])
				break;

			pat += (*pat == '?') ? 2 : 3;
		}
		else
		{
			pat = signature;
			first_match = 0;
		}
	}

	return first_match;
}

bool Memory::Write(uintptr_t address, void* buffer, size_t size) const
{
	if (!VMMDLL_MemWrite(this->vHandle, this->current_process.PID, address, (PBYTE)buffer, size))
	{
		ERROR("Failed to write Memory at 0x{}", address);
		return false;
	}
	return true;
}

bool Memory::Write(uintptr_t address, void* buffer, size_t size, int pid) const
{
	if (!VMMDLL_MemWrite(this->vHandle, pid, address, (PBYTE)buffer, size))
	{
		ERROR("Failed to write Memory at 0x{}", address);
		return false;
	}
	return true;
}

bool Memory::Read(uintptr_t address, void* buffer, size_t size) const
{
	if (!VMMDLL_MemReadEx(this->vHandle, this->current_process.PID, address, (PBYTE)buffer, size, NULL, VMMDLL_FLAG_NOCACHE))
	{
		ERROR("Failed to read Memory at 0x{}", address);
		return false;
	}
	return true;
}

bool Memory::Read(uintptr_t address, void* buffer, size_t size, int pid) const
{
	if (!VMMDLL_MemReadEx(this->vHandle, pid, address, (PBYTE)buffer, size, NULL, VMMDLL_FLAG_NOCACHE))
	{
		ERROR("Failed to read Memory at 0x{}", address);
		return false;
	}
	return true;
}

VMMDLL_SCATTER_HANDLE Memory::CreateScatterHandle()
{
	VMMDLL_SCATTER_HANDLE ScatterHandle = VMMDLL_Scatter_Initialize(this->vHandle, this->current_process.PID, VMMDLL_FLAG_NOCACHE);
	if (!ScatterHandle)
		ERROR("Failed to create scatter handle");
	return ScatterHandle;
}

VMMDLL_SCATTER_HANDLE Memory::CreateScatterHandle(int pid)
{
	VMMDLL_SCATTER_HANDLE ScatterHandle = VMMDLL_Scatter_Initialize(this->vHandle, pid, VMMDLL_FLAG_NOCACHE);
	if (!ScatterHandle)
		ERROR("Failed to create scatter handle");
	return ScatterHandle;
}

void Memory::CloseScatterHandle(VMMDLL_SCATTER_HANDLE handle)
{
	VMMDLL_Scatter_CloseHandle(handle);
}

void Memory::AddScatterReadRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size)
{
	DWORD memoryPrepared = NULL;
	if (!VMMDLL_Scatter_PrepareEx(handle, address, size, (PBYTE)buffer, &memoryPrepared))
	{
		ERROR("Failed to prepare scatter read at 0x{}", address);
	}
}

void Memory::AddScatterWriteRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size)
{
	if (!VMMDLL_Scatter_PrepareWrite(handle, address, (PBYTE)buffer, size))
	{
		ERROR("Failed to prepare scatter write at 0x{}", address);
	}
}
void Memory::ExecuteScatterWrite(VMMDLL_SCATTER_HANDLE handle)
{

	if (!VMMDLL_Scatter_Execute(handle))
	{
		ERROR("[-] Failed to Execute Scatter Read");
	}

	//Clear after using it
	if (!VMMDLL_Scatter_Clear(handle, this->current_process.PID, NULL))
	{
		ERROR("[-] Failed to clear Scatter");
	}
}
void Memory::ExecuteScatterRead(VMMDLL_SCATTER_HANDLE handle)
{

	if (!VMMDLL_Scatter_ExecuteRead(handle))
	{
		ERROR("[-] Failed to Execute Scatter Read");
	}

	//Clear after using it
	if (!VMMDLL_Scatter_Clear(handle, this->current_process.PID, NULL))
	{
		ERROR("[-] Failed to clear Scatter");
	}
}
void Memory::ExecuteReadScatter(VMMDLL_SCATTER_HANDLE handle, int pid)
{
	if (pid == 0)
		pid = this->current_process.PID;

	if (!VMMDLL_Scatter_ExecuteRead(handle))
	{
		ERROR("[-] Failed to Execute Scatter Read");
	}

	//Clear after using it
	if (!VMMDLL_Scatter_Clear(handle, pid, NULL))
	{
		ERROR("[-] Failed to clear Scatter");
	}
}

void Memory::ExecuteWriteScatter(VMMDLL_SCATTER_HANDLE handle, int pid)
{
	if (pid == 0)
		pid = this->current_process.PID;

	if (!VMMDLL_Scatter_Execute(handle))
	{
		ERROR("[-] Failed to Execute Scatter Read");
	}

	//Clear after using it
	if (!VMMDLL_Scatter_Clear(handle, pid, NULL))
	{
		ERROR("[-] Failed to clear Scatter");
	}
}
