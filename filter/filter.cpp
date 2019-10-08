/// by fanxiushu 2018-08-29
#include "filter.h"
#include "ioctl.h"

///global var
wddm_filter_t __gbl_wddm_filter;

#define DEV_NAME L"\\Device\\WddmFilterCtrlDevice"
#define DOS_NAME L"\\DosDevices\\WddmFilterCtrlDevice"

/////
static NTSTATUS create_ctrl_device()
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT devObj;
	UNICODE_STRING dev_name;
	UNICODE_STRING dos_name;

	RtlInitUnicodeString(&dev_name, DEV_NAME);
	RtlInitUnicodeString(&dos_name, DOS_NAME);

	status = IoCreateDevice(
		wf->driver_object,
		0,
		&dev_name, //dev name
		FILE_DEVICE_VIDEO,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&devObj);
	if (!NT_SUCCESS(status)) {
		DPT("IoCreateDevice err=0x%X\n", status );
		return status;
	}

	status = IoCreateSymbolicLink(&dos_name, &dev_name);
	if (!NT_SUCCESS(status)) {
		DPT("IoCreateSymbolicLink err=0x%X\n", status );
		IoDeleteDevice(devObj);
		return status;
	}

	// attach
	wf->dxgkrnl_nextDevice = IoAttachDeviceToDeviceStack(devObj, wf->dxgkrnl_pdoDevice);
	if (!wf->dxgkrnl_nextDevice) {
		DPT("IoAttachDeviceToDeviceStack error.\n");
		IoDeleteDevice(devObj);
		IoDeleteSymbolicLink(&dos_name);
		return STATUS_NOT_FOUND;
	}
	devObj->Flags |= DO_POWER_PAGABLE | DO_BUFFERED_IO | DO_DIRECT_IO;
	wf->ctrl_devobj = devObj;

	/////
	return status;
}
NTSTATUS create_wddm_filter_ctrl_device(PDRIVER_OBJECT drvObj )
{
	NTSTATUS status = STATUS_SUCCESS;

	UNICODE_STRING drvPath;
	UNICODE_STRING drvName;
	RtlInitUnicodeString(&drvPath, L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\SERVICES\\DXGKrnl");
	RtlInitUnicodeString(&drvName, L"\\Device\\Dxgkrnl");
	//
	RtlZeroMemory(wf, sizeof(wddm_filter_t));

	wf->driver_object = drvObj;
	KeInitializeSpinLock(&wf->spin_lock);
	InitializeListHead(&wf->vidpn_if_head);
	InitializeListHead(&wf->topology_if_head);

	//加载dxgkrnl.sys驱动
	status = ZwLoadDriver(&drvPath);
	if (!NT_SUCCESS(status)) {
		if (status != STATUS_IMAGE_ALREADY_LOADED) {
			DPT("ZwLoadDriver error st=0x%X\n", status );
			return status;
		}
	}

	status = IoGetDeviceObjectPointer(&drvName, FILE_ALL_ACCESS, &wf->dxgkrnl_fileobj, &wf->dxgkrnl_pdoDevice);
	if (!NT_SUCCESS(status)) {
		DPT("IoGetDeviceObjectPointer Get DxGkrnl err=0x%X\n", status );
		return status;
	}

	KEVENT evt;
	IO_STATUS_BLOCK ioStatus;

	KeInitializeEvent(&evt, NotificationEvent, FALSE);

	PIRP pIrp = IoBuildDeviceIoControlRequest(
		IOCTL_VIDEO_DDI_FUNC_REGISTER, //0x23003F , dxgkrnl.sys 导出注册函数
		wf->dxgkrnl_pdoDevice,
		NULL,
		0,
		&wf->dxgkrnl_dpiInit,
		sizeof(PDXGKRNL_DPIINITIALIZE),
		TRUE, // IRP_MJ_INTERNAL_DEVICE_CONTROL
		&evt,
		&ioStatus);

	if (!pIrp) {
		DPT("IoBuildDeviceIoControlRequest return NULL.\n");
		ObDereferenceObject(wf->dxgkrnl_fileobj);
		return STATUS_NO_MEMORY;
	}
	status = IoCallDriver(wf->dxgkrnl_pdoDevice, pIrp);
	if (status == STATUS_PENDING) {
		KeWaitForSingleObject(&evt, Executive, KernelMode, FALSE, NULL);
		status = ioStatus.Status;
	}
	if (!wf->dxgkrnl_dpiInit) {//
		DPT("Can not Load PDXGKRNL_DPIINITIALIZE function address. st=0x%X\n", status );
		ObDereferenceObject(wf->dxgkrnl_fileobj);
		return STATUS_NOT_FOUND;
	}

	///create filter device
	status = create_ctrl_device();
	if (!NT_SUCCESS(status)) {
		ObDereferenceObject(wf->dxgkrnl_fileobj);
		return status;
	}

	////
	return status;
}

NTSTATUS log_event(PUNICODE_STRING str)
{
	NTSTATUS status = STATUS_SUCCESS;

	return status;
}
