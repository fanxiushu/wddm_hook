/// by fanxiushu 2018-08-29

#pragma once

#include <ntddk.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include <ntddvdeo.h>

#include <initguid.h>

#include <Dispmprt.h>
#include <d3dkmdt.h>

////////////////////////////////////////////////////////////
#ifdef DBG
#define DPT   DbgPrint
#else
#define DPT   //
#endif

///定义VIDPN中子设备ID，
#define VIDPN_CHILD_UDID             0x667b0099

/////////
///0x23003F , dxgkrnl.sys 导出注册函数 DXGKRNL_DPIINITIALIZE
#define IOCTL_VIDEO_DDI_FUNC_REGISTER \
	CTL_CODE( FILE_DEVICE_VIDEO, 0xF, METHOD_NEITHER, FILE_ANY_ACCESS  )

typedef __checkReturn NTSTATUS
DXGKRNL_DPIINITIALIZE(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath,
	DRIVER_INITIALIZATION_DATA* DriverInitData
);
typedef DXGKRNL_DPIINITIALIZE* PDXGKRNL_DPIINITIALIZE;


///////
struct vidpn_target_id
{
	LONG    num;
	D3DDDI_VIDEO_PRESENT_TARGET_ID ids[1];
};
struct vidpn_paths_t
{
	LONG               num_paths;
	vidpn_target_id*   target_paths[1];
};

struct vidpn_intf_t
{
	LIST_ENTRY                   list;
	///
    D3DKMDT_HVIDPN               hVidPn;
	DXGK_VIDPN_INTERFACE         vidpn_if, mod_vidpn_if;

	////
	D3DKMDT_HVIDPNTOPOLOGY              hTopology;
	DXGK_VIDPNTOPOLOGY_INTERFACE        topology_if, mod_topology_if;

	vidpn_paths_t*                      paths; ////
};

struct wddm_filter_t
{
	PDRIVER_OBJECT          driver_object;
	////
	PDEVICE_OBJECT          ctrl_devobj;
	
	////
	PFILE_OBJECT            dxgkrnl_fileobj;
	PDEVICE_OBJECT          dxgkrnl_pdoDevice;
	PDEVICE_OBJECT          dxgkrnl_nextDevice; ///

	PDXGKRNL_DPIINITIALIZE  dxgkrnl_dpiInit;

	///
	KSPIN_LOCK              spin_lock;
	KIRQL                   kirql;

	LIST_ENTRY              vidpn_if_head;
	LIST_ENTRY              topology_if_head;
	////
	DRIVER_INITIALIZATION_DATA  orgDpiFunc;  //原始的DRIVER_INITIALIZATION_DATA

	ULONG                       vidpn_source_count;
	ULONG                       vidpn_target_count;
	DXGKRNL_INTERFACE           DxgkInterface;

};

extern wddm_filter_t __gbl_wddm_filter;
#define wf (&(__gbl_wddm_filter))
#define wf_lock()   KeAcquireSpinLock(&wf->spin_lock, &wf->kirql);
#define wf_unlock() KeReleaseSpinLock(&wf->spin_lock, wf->kirql);

////////////////function
NTSTATUS create_wddm_filter_ctrl_device(PDRIVER_OBJECT drvObj);

inline NTSTATUS call_lower_driver(PIRP irp)
{
	IoSkipCurrentIrpStackLocation(irp);
	return IoCallDriver(wf->dxgkrnl_nextDevice, irp);
}

NTSTATUS DpiInitialize(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath,
	DRIVER_INITIALIZATION_DATA* DriverInitData);

NTSTATUS DxgkDdiEnumVidPnCofuncModality(CONST HANDLE  hAdapter, 
	CONST DXGKARG_ENUMVIDPNCOFUNCMODALITY* CONST  pEnumCofuncModalityArg);

NTSTATUS DxgkDdiIsSupportedVidPn(
	IN_CONST_HANDLE hAdapter,
	INOUT_PDXGKARG_ISSUPPORTEDVIDPN pIsSupportedVidPn);

NTSTATUS DxgkDdiCommitVidPn(
	IN_CONST_HANDLE hAdapter,
	IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn);

NTSTATUS DxgkDdiSetVidPnSourceVisibility(
	IN_CONST_HANDLE hAdapter,
	IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY pSetVidPnSourceVisibility);

NTSTATUS APIENTRY DxgkDdiSetVidPnSourceAddress(
	const HANDLE                        hAdapter,
	const DXGKARG_SETVIDPNSOURCEADDRESS *pSetVidPnSourceAddress);


