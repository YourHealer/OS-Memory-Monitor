#include <iostream>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <iomanip>	 
#include <Windows.h> 
#include <Tlhelp32.h>  
#include <shlwapi.h>
#include <Psapi.h>


using namespace std;

enum ProgramState { QUERY_SYS_INFO, QUERY_MEM_INFO, QUERY_PRE_INFO, EXIT };

//查询系统配置信息
void getSystemInfo();

//物理内存使用情况
void getMemoryInfo();

//打印所有进程的基本信息
void getProcessInfo();

//获得单个内存的使用情况
void showSingleProcessMemDetail(int PID);

void help();

int main()
{
	setlocale(LC_ALL, "");
	int state = ProgramState::EXIT;//默认为退出状态
	while (1)
	{
		help();
		std::cout.fill(' ');
		std::cout.setf(ios::dec);//确保cout输出为十进制
		cin >> state;
		std::cout << "\n";
		if (state == ProgramState::QUERY_SYS_INFO)
		{
			getSystemInfo();
		}
		else if (state == ProgramState::QUERY_MEM_INFO)
		{
			getMemoryInfo();
		}
		else if (state == ProgramState::QUERY_PRE_INFO)
		{
			getProcessInfo();  //当前所有运行进程基本信息
			
			std::cout << "输入进程PID以查看其虚拟内存信息" << endl;
			int PID;
			cin >> PID;
			
			showSingleProcessMemDetail(PID);
		}
		else if (state == ProgramState::EXIT)
		{
			return 0;    //结束程序的运行
		}
	}
	return 0;
}


//将字节数转为字符串打印输出
inline void printStrFormatByte(const WCHAR* info, DWORDLONG bytes)
{
	TCHAR tmp[MAX_PATH];
	ZeroMemory(tmp, sizeof(tmp));
	StrFormatByteSize(bytes, tmp, MAX_PATH);
	wcout << info << tmp << endl;
	return;
}

//打印地址
inline void printAddress(const WCHAR* info, LPVOID addr)
{
	wcout << info << hex << setw(8) << addr << endl;
}

inline void printDword(const WCHAR* info, DWORDLONG dw)//将DWORD转为DWORDLONG
{
	wcout << info;
	std::cout << dw << endl;
}

//查询系统配置信息
void getSystemInfo()
{
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(si));
	GetNativeSystemInfo(&si);
	DWORD mem_size = (DWORD*)si.lpMaximumApplicationAddress - (DWORD*)si.lpMinimumApplicationAddress;
	printDword(L"处理器个数                ", si.dwNumberOfProcessors);
	printStrFormatByte(L"物理页大小                ", si.dwPageSize);
	printAddress(L"进程最小寻址空间：        0x", si.lpMinimumApplicationAddress);
	printAddress(L"进程最大寻址地址:         0x", si.lpMaximumApplicationAddress);
	printStrFormatByte(L"进程可用空间大小：        ", mem_size);
	return;
}

//物理内存使用情况
void getMemoryInfo()
{
	MEMORYSTATUSEX mem_stat;
	ZeroMemory(&mem_stat, sizeof(mem_stat));
	mem_stat.dwLength = sizeof(mem_stat);//必须执行这一步
	GlobalMemoryStatusEx(&mem_stat); //取得内存状态
	std::cout << "内存利用率        \t" << mem_stat.dwMemoryLoad << endl;
	printStrFormatByte(L"物理内存：        \t", mem_stat.ullTotalPhys);
	printStrFormatByte(L"可用物理内存：      \t", mem_stat.ullAvailPhys);
	printStrFormatByte(L"总共页文件大小：    \t", mem_stat.ullTotalPageFile);
	printStrFormatByte(L"空闲页文件大小：    \t", mem_stat.ullAvailPageFile);
	printStrFormatByte(L"虚拟内存大小：    \t", mem_stat.ullTotalVirtual);
	printStrFormatByte(L"空闲虚拟内存大小：\t", mem_stat.ullAvailVirtual);
	printStrFormatByte(L"空闲拓展内存大小：\t", mem_stat.ullAvailExtendedVirtual);
	
	cout << endl << endl;
	
	PERFORMANCE_INFORMATION pi;
	GetPerformanceInfo(&pi, sizeof(pi));
	DWORDLONG page_size = pi.PageSize;
	printStrFormatByte(L"Commit Total           \t", pi.CommitTotal * page_size);
	printStrFormatByte(L"Commit Limit           \t", pi.CommitLimit * page_size);
	printStrFormatByte(L"Commit Peak            \t", pi.CommitPeak * page_size);
	printStrFormatByte(L"Physical Memory        \t", pi.PhysicalTotal * page_size);
	printStrFormatByte(L"Physical Memory Avaliable   ", pi.PhysicalAvailable * page_size);
	printStrFormatByte(L"System Cache           \t", page_size*pi.SystemCache);
	printStrFormatByte(L"Kerbel Total           \t", page_size * pi.KernelTotal);
	printStrFormatByte(L"Kernel Paged           \t", page_size * pi.KernelPaged);
	printStrFormatByte(L"Kernel Nonpaged        \t", page_size * pi.KernelNonpaged);
	printStrFormatByte(L"Page Size              \t", page_size * pi.PageSize);
	printDword(L"Handle Count           \t", page_size * pi.HandleCount);
	printDword(L"Process Count          \t", page_size * pi.ProcessCount);
	printDword(L"Thread Count           \t", page_size * pi.ThreadCount);
	
	
	
}

//打印所有进程的基本信息
void getProcessInfo()
{
	//创建进程snapshot
	HANDLE h_process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (h_process_snapshot == INVALID_HANDLE_VALUE)
	{
		cout << "CreateToolhelp32Snapshot调用失败！\n";
		exit(-1);
	}
	
	
	PROCESSENTRY32 process_entry;
	process_entry.dwSize = sizeof(process_entry);//必须要指定大小
	
	//循环遍历输出所有进程的信息
	if (Process32First(h_process_snapshot, &process_entry))
	{
		wcout << setiosflags(ios::left) << setw(40) << L"Process Name";
		cout << setiosflags(ios::right) << setw(15) << "PID";
		wcout << L"\t\t线程数量" << endl << endl;
		do
		{
			wcout << setiosflags(ios::left) << setw(40) << process_entry.szExeFile;//进程名
			std::cout << "\t" << setw(7) << process_entry.th32ProcessID;//pid
			std::cout << "  \t" << setw(3) << process_entry.cntThreads << endl;//线程数目
		} while (Process32Next(h_process_snapshot, &process_entry));
	}
	CloseHandle(h_process_snapshot);
}

//显示当前块页面访问方式
void printPageProtection(DWORD dwTarget)
{
	const int width = 20;
	switch (dwTarget)
	{
	case(PAGE_READONLY):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "READONLY";
			break;
			
		}
	case(PAGE_GUARD):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "GUARD";
			break;
			
		}
	case(PAGE_NOCACHE):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "NOCACHE";
			break;
			
		}
	case(PAGE_NOACCESS):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "NOACCESS";
			break;
			
		}
	case(PAGE_READWRITE):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "READWRITE";
			break;
			
		}
	case(PAGE_WRITECOPY):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "WRITECOPY";
			break;
			
		}
	case(PAGE_EXECUTE):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "EXECUTE";
			break;
			
		}
	case(PAGE_EXECUTE_READ):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "EXECUTE_READ";
			break;
			
		}
	case(PAGE_EXECUTE_READWRITE):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "EXECUTE_READWRITE";
			break;
			
		}
	case(PAGE_EXECUTE_WRITECOPY):
		{
			std::cout << setiosflags(ios::left) << setw(width) << "EXECUTE_WRITECOPY";
			break;
		}
		default:
			break;
	}
	
}

//输出单个进程的详细信息
void showSingleProcessMemDetail(int PID)
{
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(si));
	GetNativeSystemInfo(&si);           //获得系统信息 
	
	//循环访问整个进程地址空间 
	LPCVOID p_begin = (LPVOID)si.lpMinimumApplicationAddress; //p_begin指向开始的地址
	std::cout.setf(ios::left);
	//输出表头
	wcout << setiosflags(ios::left) << setw(21) << L"块地址"
	<< setw(10) << L"块大小"
	<< setw(10) << L"块内页状态"
	<< setw(12) << L"块内页保护方式"
	<< setw(10) << L"块类型" << endl;
	
	HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, 0, PID);	//得到PID的值
	if (h_process == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to OpenProcess" << endl;
		exit(-1);
	}
	
	MEMORY_BASIC_INFORMATION mem;    //虚拟内存空间的基本信息结构 
	ZeroMemory(&mem, sizeof(mem));
	while (p_begin < (LPVOID)si.lpMaximumApplicationAddress)
	{
		//查询进程在p_begin开始的块信息
		VirtualQueryEx(
			h_process,                       //进程句柄
			p_begin,                         //开始位置的地址
			&mem,                           //缓冲区
			sizeof(mem));
		//块结束地址 
		LPCVOID p_end = (PBYTE)p_begin + mem.RegionSize;
		
		//输出块起始、结束地址
		std::cout << hex << setw(8) << setfill('0') << (DWORD*)p_begin
		<< "-"
		<< hex << setw(8) << setfill('0') << (DWORD*)p_end;
		
		//输出块大小
		TCHAR tmp[MAX_PATH];
		ZeroMemory(tmp, sizeof(tmp));
		StrFormatByteSize(mem.RegionSize, tmp, MAX_PATH);
		std::wcout << "\t" << setw(8) << tmp;
		
		//输出块的状态 
		std::cout.fill(' ');
		if (mem.State == MEM_COMMIT)
		{
			std::cout << setw(10) << "已提交";
		}
		else if (mem.State == MEM_FREE)
		{
			std::cout << setw(10) << "空闲";
			
		}
		else if (mem.State == MEM_RESERVE)
		{
			std::cout << setw(10) << "保留";
		}
		
		//显示块内页的保护方式
		if (mem.Protect == 0 && mem.State != MEM_FREE)
		{
			mem.Protect = PAGE_READONLY;
		}
		printPageProtection(mem.Protect);
		
		//显示块的类型 邻近页面物理存储器类型指的是与给定地址所在页面相同的存储器类型
		std::cout.fill(' ');
		if (mem.Type == MEM_IMAGE)
		{
			std::cout << "\t\tImage";
		}
		else if (mem.Type == MEM_PRIVATE)
		{
			std::cout << "\t\tPrivate";
		}
		else if (mem.Type == MEM_MAPPED)
		{
			std::cout << "\t\tMapped";
		}
		cout << endl;
		
		//移动块指针获得下一个块 
		if (p_begin == p_end)//部分进程如0号进程无法进行空间遍历
			break;
		p_begin = p_end;
	}
}

void help()
{
	std::cout << "\n\nMenu：" << endl
	<< ProgramState::QUERY_SYS_INFO << " - 查看系统信息" << endl
	<< ProgramState::QUERY_MEM_INFO << " - 查看内存情况" << endl
	<< ProgramState::QUERY_PRE_INFO << " - 查看当前运行进程信息及其虚拟地址空间布局和工作集信息" << endl
	<< ProgramState::EXIT << " - 退出\n\n";
}

