#include <ntddk.h>
#define INITCODE code_seg("INIT") 
#define PAGECODE code_seg("PAGE") /*表示内存不足时，可以被置换到硬盘*/
#pragma INITCODE /*指的代码运行后 就从内存释放掉*/
NTSTATUS CreateMyDevice (IN PDRIVER_OBJECT pDriverObject) 
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;/*用来返回创建设备*/

	//创建设备名称
	UNICODE_STRING devName;
	UNICODE_STRING symLinkName; // 
	//设备对象一般都位于该命名空间的\Device子目录下
	RtlInitUnicodeString(&devName,L"\\Device\\yjxDDK_Device");/*对devName初始化字串为 "\\Device\\yjxDDK_Device"*/
/*
NTSTATUS   IoCreateDevice(
  	IN PDRIVER_OBJECT  DriverObject,    //设备对象所对应的驱动对象
  	IN ULONG  DeviceExtensionSize,    //设备扩展区的大小
  	IN PUNICODE_STRING  DeviceName  OPTIONAL,  //设备对象名称
  	IN DEVICE_TYPE  DeviceType,   //设备类型
  	IN ULONG  DeviceCharacteristics,  //设备对象特征
  	IN BOOLEAN  Exclusive,  // 一般设为FALSE
  	OUT PDEVICE_OBJECT  *DeviceObject   //由操作系统创建的设备对象                                                                              // 的起始地址利用该参数返回
  );
*/
	//创建设备
	status = IoCreateDevice( pDriverObject,\
		0,\
		&devName,\
		FILE_DEVICE_UNKNOWN,\
		0, TRUE,\
		&pDevObj);
	if (!NT_SUCCESS(status))
	{
		if (status==STATUS_INSUFFICIENT_RESOURCES)
		{
			KdPrint(("资源不足 STATUS_INSUFFICIENT_RESOURCES"));
		}
		if (status==STATUS_OBJECT_NAME_EXISTS )
		{
			KdPrint(("指定对象名存在"));
		}
		if (status==STATUS_OBJECT_NAME_COLLISION)
		{
			KdPrint(("//对象名有冲突"));
		}
		KdPrint(("设备创建失败...++++++++"));
		return status;
	}
	KdPrint(("设备创建成功...++++++++"));

	pDevObj->Flags |= DO_BUFFERED_IO;
//应用程序只能访问Windows内核对象空间中的“\??”子目录，而对对\Device子目录没有访问权限，
//因此必须在“\??”子目录中创建一个符号链接，将其指向\Device子目录中的名字
	//创建符号链接
	RtlInitUnicodeString(&symLinkName,L"\\??\\yjx888");
	status = IoCreateSymbolicLink( &symLinkName,&devName );
	if (!NT_SUCCESS(status)) /*status等于0*/
	{
		IoDeleteDevice( pDevObj );
		return status;
	}
	return STATUS_SUCCESS;
}

VOID DDK_Unload (IN PDRIVER_OBJECT pDriverObject); //前置说明 卸载例程
NTSTATUS ddk_DispatchRoutine_CONTROL(IN PDEVICE_OBJECT pDevobj,IN PIRP pIrp	);//派遣函数
typedef struct _ServiceDescriptorTable {
	PVOID ServiceTableBase; //System Service Dispatch Table 的基地址  
	PVOID ServiceCounterTable;
	//包含着 SSDT 中每个服务被调用次数的计数器。这个计数器一般由sysenter 更新。 
	unsigned int NumberOfServices;//由 ServiceTableBase 描述的服务的数目。  
	PVOID ParamTableBase; //包含每个系统服务参数字节数表的基地址-系统服务参数表 
}*PServiceDescriptorTable;  
extern PServiceDescriptorTable KeServiceDescriptorTable;
ULONG GetNt_CurAddr() //获取当前SSDT_NtOpenProcess的当前地址
{

LONG *SSDT_Adr,SSDT_NtOpenProcess_Cur_Addr,t_addr; 
KdPrint(("驱动成功被加载中.............................\n"));
//读取SSDT表中索引值为0x7A的函数
//poi(poi(KeServiceDescriptorTable)+0x7a*4)
t_addr=(LONG)KeServiceDescriptorTable->ServiceTableBase;
KdPrint(("当前ServiceTableBase地址为%x \n",t_addr));
SSDT_Adr=(PLONG)(t_addr+0x7A*4);
KdPrint(("当前t_addr+0x7A*4=%x \n",SSDT_Adr)); 
SSDT_NtOpenProcess_Cur_Addr=*SSDT_Adr;	
KdPrint(("当前SSDT_NtOpenProcess_Cur_Addr地址为%x \n",SSDT_NtOpenProcess_Cur_Addr));
return SSDT_NtOpenProcess_Cur_Addr;
}

ULONG GetNt_OldAddr()
{
	UNICODE_STRING Old_NtOpenProcess;
    ULONG Old_Addr;
	RtlInitUnicodeString(&Old_NtOpenProcess,L"NtOpenProcess");
	Old_Addr=(ULONG)MmGetSystemRoutineAddress(&Old_NtOpenProcess);//取得NtOpenProcess的地址
	KdPrint(("取得原函数NtOpenProcess的值为 %x",Old_Addr));
	return Old_Addr;

}