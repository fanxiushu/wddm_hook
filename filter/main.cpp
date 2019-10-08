/// by fanxiushu 2018-08-29

#include "filter.h"

static NTSTATUS commonDispatch(PDEVICE_OBJECT devObj, PIRP irp)
{
	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(irp);

	switch (irpStack->MajorFunction)
	{
	case IRP_MJ_CREATE:
		break;
	case IRP_MJ_CLEANUP:
		break;
	case IRP_MJ_CLOSE:
		break;

	case IRP_MJ_INTERNAL_DEVICE_CONTROL:
		if (irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_VIDEO_DDI_FUNC_REGISTER) {
			///////显卡驱动在DxgkInitialize函数中调用 IOCTL获取dxgkrnl.sys的注册回调函数，我们hook此处，获取到显卡驱动提供的所有DDI函数

			irp->IoStatus.Information = 0;
			irp->IoStatus.Status = STATUS_SUCCESS;

			///把我们的回调函数返回给显卡驱动.
			if (irp->UserBuffer) {
				///
				irp->IoStatus.Information = sizeof(PDXGKRNL_DPIINITIALIZE);
				*((PDXGKRNL_DPIINITIALIZE*)irp->UserBuffer) = DpiInitialize;
			}

			/////
			IoCompleteRequest(irp, IO_NO_INCREMENT);

			return STATUS_SUCCESS;
			///
		}
		break;
	}
	////
	return call_lower_driver(irp);
}

extern "C" NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT  DriverObject,
	IN PUNICODE_STRING RegistryPath)
{

	NTSTATUS status = STATUS_SUCCESS;

	for (UCHAR i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i) {
		DriverObject->MajorFunction[i] = commonDispatch;
	}

	status = create_wddm_filter_ctrl_device(DriverObject);
	///
	DriverObject->DriverUnload = NULL; ///不允许卸载

	return status;
}

