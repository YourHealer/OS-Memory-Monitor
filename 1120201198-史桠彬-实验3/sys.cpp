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

//��ѯϵͳ������Ϣ
void getSystemInfo();

//�����ڴ�ʹ�����
void getMemoryInfo();

//��ӡ���н��̵Ļ�����Ϣ
void getProcessInfo();

//��õ����ڴ��ʹ�����
void showSingleProcessMemDetail(int PID);

void help();

int main()
{
	setlocale(LC_ALL, "");
	int state = ProgramState::EXIT;//Ĭ��Ϊ�˳�״̬
	while (1)
	{
		help();
		std::cout.fill(' ');
		std::cout.setf(ios::dec);//ȷ��cout���Ϊʮ����
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
			getProcessInfo();  //��ǰ�������н��̻�����Ϣ
			
			std::cout << "�������PID�Բ鿴�������ڴ���Ϣ" << endl;
			int PID;
			cin >> PID;
			
			showSingleProcessMemDetail(PID);
		}
		else if (state == ProgramState::EXIT)
		{
			return 0;    //�������������
		}
	}
	return 0;
}


//���ֽ���תΪ�ַ�����ӡ���
inline void printStrFormatByte(const WCHAR* info, DWORDLONG bytes)
{
	TCHAR tmp[MAX_PATH];
	ZeroMemory(tmp, sizeof(tmp));
	StrFormatByteSize(bytes, tmp, MAX_PATH);
	wcout << info << tmp << endl;
	return;
}

//��ӡ��ַ
inline void printAddress(const WCHAR* info, LPVOID addr)
{
	wcout << info << hex << setw(8) << addr << endl;
}

inline void printDword(const WCHAR* info, DWORDLONG dw)//��DWORDתΪDWORDLONG
{
	wcout << info;
	std::cout << dw << endl;
}

//��ѯϵͳ������Ϣ
void getSystemInfo()
{
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(si));
	GetNativeSystemInfo(&si);
	DWORD mem_size = (DWORD*)si.lpMaximumApplicationAddress - (DWORD*)si.lpMinimumApplicationAddress;
	printDword(L"����������                ", si.dwNumberOfProcessors);
	printStrFormatByte(L"����ҳ��С                ", si.dwPageSize);
	printAddress(L"������СѰַ�ռ䣺        0x", si.lpMinimumApplicationAddress);
	printAddress(L"�������Ѱַ��ַ:         0x", si.lpMaximumApplicationAddress);
	printStrFormatByte(L"���̿��ÿռ��С��        ", mem_size);
	return;
}

//�����ڴ�ʹ�����
void getMemoryInfo()
{
	MEMORYSTATUSEX mem_stat;
	ZeroMemory(&mem_stat, sizeof(mem_stat));
	mem_stat.dwLength = sizeof(mem_stat);//����ִ����һ��
	GlobalMemoryStatusEx(&mem_stat); //ȡ���ڴ�״̬
	std::cout << "�ڴ�������        \t" << mem_stat.dwMemoryLoad << endl;
	printStrFormatByte(L"�����ڴ棺        \t", mem_stat.ullTotalPhys);
	printStrFormatByte(L"���������ڴ棺      \t", mem_stat.ullAvailPhys);
	printStrFormatByte(L"�ܹ�ҳ�ļ���С��    \t", mem_stat.ullTotalPageFile);
	printStrFormatByte(L"����ҳ�ļ���С��    \t", mem_stat.ullAvailPageFile);
	printStrFormatByte(L"�����ڴ��С��    \t", mem_stat.ullTotalVirtual);
	printStrFormatByte(L"���������ڴ��С��\t", mem_stat.ullAvailVirtual);
	printStrFormatByte(L"������չ�ڴ��С��\t", mem_stat.ullAvailExtendedVirtual);
	
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

//��ӡ���н��̵Ļ�����Ϣ
void getProcessInfo()
{
	//��������snapshot
	HANDLE h_process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (h_process_snapshot == INVALID_HANDLE_VALUE)
	{
		cout << "CreateToolhelp32Snapshot����ʧ�ܣ�\n";
		exit(-1);
	}
	
	
	PROCESSENTRY32 process_entry;
	process_entry.dwSize = sizeof(process_entry);//����Ҫָ����С
	
	//ѭ������������н��̵���Ϣ
	if (Process32First(h_process_snapshot, &process_entry))
	{
		wcout << setiosflags(ios::left) << setw(40) << L"Process Name";
		cout << setiosflags(ios::right) << setw(15) << "PID";
		wcout << L"\t\t�߳�����" << endl << endl;
		do
		{
			wcout << setiosflags(ios::left) << setw(40) << process_entry.szExeFile;//������
			std::cout << "\t" << setw(7) << process_entry.th32ProcessID;//pid
			std::cout << "  \t" << setw(3) << process_entry.cntThreads << endl;//�߳���Ŀ
		} while (Process32Next(h_process_snapshot, &process_entry));
	}
	CloseHandle(h_process_snapshot);
}

//��ʾ��ǰ��ҳ����ʷ�ʽ
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

//����������̵���ϸ��Ϣ
void showSingleProcessMemDetail(int PID)
{
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(si));
	GetNativeSystemInfo(&si);           //���ϵͳ��Ϣ 
	
	//ѭ�������������̵�ַ�ռ� 
	LPCVOID p_begin = (LPVOID)si.lpMinimumApplicationAddress; //p_beginָ��ʼ�ĵ�ַ
	std::cout.setf(ios::left);
	//�����ͷ
	wcout << setiosflags(ios::left) << setw(21) << L"���ַ"
	<< setw(10) << L"���С"
	<< setw(10) << L"����ҳ״̬"
	<< setw(12) << L"����ҳ������ʽ"
	<< setw(10) << L"������" << endl;
	
	HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, 0, PID);	//�õ�PID��ֵ
	if (h_process == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to OpenProcess" << endl;
		exit(-1);
	}
	
	MEMORY_BASIC_INFORMATION mem;    //�����ڴ�ռ�Ļ�����Ϣ�ṹ 
	ZeroMemory(&mem, sizeof(mem));
	while (p_begin < (LPVOID)si.lpMaximumApplicationAddress)
	{
		//��ѯ������p_begin��ʼ�Ŀ���Ϣ
		VirtualQueryEx(
			h_process,                       //���̾��
			p_begin,                         //��ʼλ�õĵ�ַ
			&mem,                           //������
			sizeof(mem));
		//�������ַ 
		LPCVOID p_end = (PBYTE)p_begin + mem.RegionSize;
		
		//�������ʼ��������ַ
		std::cout << hex << setw(8) << setfill('0') << (DWORD*)p_begin
		<< "-"
		<< hex << setw(8) << setfill('0') << (DWORD*)p_end;
		
		//������С
		TCHAR tmp[MAX_PATH];
		ZeroMemory(tmp, sizeof(tmp));
		StrFormatByteSize(mem.RegionSize, tmp, MAX_PATH);
		std::wcout << "\t" << setw(8) << tmp;
		
		//������״̬ 
		std::cout.fill(' ');
		if (mem.State == MEM_COMMIT)
		{
			std::cout << setw(10) << "���ύ";
		}
		else if (mem.State == MEM_FREE)
		{
			std::cout << setw(10) << "����";
			
		}
		else if (mem.State == MEM_RESERVE)
		{
			std::cout << setw(10) << "����";
		}
		
		//��ʾ����ҳ�ı�����ʽ
		if (mem.Protect == 0 && mem.State != MEM_FREE)
		{
			mem.Protect = PAGE_READONLY;
		}
		printPageProtection(mem.Protect);
		
		//��ʾ������� �ڽ�ҳ������洢������ָ�����������ַ����ҳ����ͬ�Ĵ洢������
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
		
		//�ƶ���ָ������һ���� 
		if (p_begin == p_end)//���ֽ�����0�Ž����޷����пռ����
			break;
		p_begin = p_end;
	}
}

void help()
{
	std::cout << "\n\nMenu��" << endl
	<< ProgramState::QUERY_SYS_INFO << " - �鿴ϵͳ��Ϣ" << endl
	<< ProgramState::QUERY_MEM_INFO << " - �鿴�ڴ����" << endl
	<< ProgramState::QUERY_PRE_INFO << " - �鿴��ǰ���н�����Ϣ���������ַ�ռ䲼�ֺ͹�������Ϣ" << endl
	<< ProgramState::EXIT << " - �˳�\n\n";
}

