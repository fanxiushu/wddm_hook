////by fanxiushu 2018-09-03
#include "filter.h"

static D3DKMDT_2DREGION  Modes[]=
{
	{1024, 768},
	{1366, 768},
	{1920, 1080},
//	{6000, 4000}
};

static NTSTATUS 
add_source_mode(D3DKMDT_HVIDPNSOURCEMODESET source_mode_set_hdl,
	CONST DXGK_VIDPNSOURCEMODESET_INTERFACE *source_mode_set_if,
	D3DKMDT_2DREGION* mode)
{
	NTSTATUS status = STATUS_SUCCESS;
	D3DKMDT_VIDPN_SOURCE_MODE *source_mode;
	D3DKMDT_GRAPHICS_RENDERING_FORMAT *fmt;

	status = source_mode_set_if->pfnCreateNewModeInfo(source_mode_set_hdl,
		&source_mode);
	if (!NT_SUCCESS(status)) {
		DPT("** pfnCreateNewModeInfo(Source) err=0x%X\n", status );
		return status;
	}

	/* Let OS assign the ID, set the type.*/
	source_mode->Type = D3DKMDT_RMT_GRAPHICS;

	/* Initialize the rendering format per our constraints and the current mode. */
	fmt = &source_mode->Format.Graphics;
	fmt->PrimSurfSize.cx = mode->cx;
	fmt->PrimSurfSize.cy = mode->cy;
	fmt->VisibleRegionSize.cx = mode->cx;
	fmt->VisibleRegionSize.cy = mode->cy;
	fmt->Stride = mode->cx*4 ; // RGBA 
	fmt->PixelFormat = D3DDDIFMT_A8R8G8B8;
	fmt->ColorBasis = D3DKMDT_CB_SRGB;
	fmt->PixelValueAccessMode = D3DKMDT_PVAM_DIRECT;

	status = source_mode_set_if->pfnAddMode(source_mode_set_hdl, source_mode);
	if (!NT_SUCCESS(status)) {
		DPT("** pfnAddMode(Source) err=0x%X\n", status );
		source_mode_set_if->pfnReleaseModeInfo(source_mode_set_hdl, source_mode);
	}

	///
	return status;
}
static NTSTATUS update_source_modes(
	CONST D3DKMDT_HVIDPN vidpn_hdl,
	CONST D3DKMDT_VIDPN_PRESENT_PATH *curr_path_info, 
	CONST DXGK_VIDPN_INTERFACE* vidpn_if)
{
	NTSTATUS status = STATUS_SUCCESS;

	D3DKMDT_HVIDPNSOURCEMODESET source_mode_set_hdl = NULL;
	CONST DXGK_VIDPNSOURCEMODESET_INTERFACE *source_mode_set_if;
	CONST D3DKMDT_VIDPN_SOURCE_MODE *src_mode_info = NULL;

	status = vidpn_if->pfnAcquireSourceModeSet(vidpn_hdl,
		curr_path_info->VidPnSourceId,
		&source_mode_set_hdl,
		&source_mode_set_if);
	if (!NT_SUCCESS(status)) {
		DPT("** not pfnAcquireSourceModeSet st=0x%X\n", status );
		return status;
	}

	////
	status = source_mode_set_if->pfnAcquirePinnedModeInfo(source_mode_set_hdl,
		&src_mode_info);
	if (!NT_SUCCESS(status)) {
		vidpn_if->pfnReleaseSourceModeSet(vidpn_hdl, source_mode_set_hdl);
		DPT("pfnAcquirePinnedModeInfo(Source) err=0x%X\n", status );
		return status;
	}
	////
	if (src_mode_info != NULL) {
		source_mode_set_if->pfnReleaseModeInfo(source_mode_set_hdl, src_mode_info);
	}
	vidpn_if->pfnReleaseSourceModeSet(vidpn_hdl, source_mode_set_hdl);
	source_mode_set_hdl = NULL; ///

	///
	if (status == STATUS_SUCCESS && src_mode_info != NULL) { // pinned mode .
		///
		DPT("Source Mode Pinned Mode: 0x%X -> 0x%X\n", curr_path_info->VidPnSourceId, curr_path_info->VidPnTargetId);
		return STATUS_SUCCESS;///已经绑定了，不处理
	}

	////

	status = vidpn_if->pfnCreateNewSourceModeSet(vidpn_hdl,
		curr_path_info->VidPnSourceId,
		&source_mode_set_hdl,
		&source_mode_set_if);

	if (!NT_SUCCESS(status)) {
		DPT("** pfnCreateNewSourceModeSet err=0x%X\n", status);
		return status;
	}
	////
	for (INT i = 0; i < sizeof(Modes) / sizeof(Modes[0]); ++i) {
		////
		status = add_source_mode(source_mode_set_hdl, source_mode_set_if, &Modes[i]);

		if (!NT_SUCCESS(status)) {
			///
			vidpn_if->pfnReleaseSourceModeSet(vidpn_hdl, source_mode_set_hdl);
			DPT("add_source_mode err=0x%X\n", status);
			return status;
		}
		////
	}

	//////
	status = vidpn_if->pfnAssignSourceModeSet(vidpn_hdl,
		curr_path_info->VidPnSourceId,
		source_mode_set_hdl);

	if (!NT_SUCCESS(status)) {
		DPT("** pfnAssignSourceModeSet err=0x%X\n", status);
		vidpn_if->pfnReleaseSourceModeSet(vidpn_hdl, source_mode_set_hdl);
	}

	////
	return status;
}

//// target
static NTSTATUS
add_target_mode(D3DKMDT_HVIDPNTARGETMODESET tgt_mode_set_hdl,
	CONST DXGK_VIDPNTARGETMODESET_INTERFACE *target_mode_set_if,
	D3DKMDT_2DREGION* mode)
{
	D3DKMDT_VIDPN_TARGET_MODE *target_mode;
	D3DKMDT_VIDEO_SIGNAL_INFO *signal_info;
	NTSTATUS status;

	status = target_mode_set_if->pfnCreateNewModeInfo(tgt_mode_set_hdl,
		&target_mode);
	if (!NT_SUCCESS(status)) {
		DPT("** pfnCreateNewModeInfo(Target) err=0x%X\n", status );
		return status;
	}
	////
	/* Let OS assign the ID, set the preferred mode field.*/
	target_mode->Preference = D3DKMDT_MP_PREFERRED;

	////
#define REFRESH_RATE 60
	signal_info = &target_mode->VideoSignalInfo;
	signal_info->VideoStandard = D3DKMDT_VSS_VESA_DMT;// D3DKMDT_VSS_OTHER;
	signal_info->TotalSize.cx = mode->cx;
	signal_info->TotalSize.cy = mode->cy;
	signal_info->ActiveSize.cx = mode->cx;
	signal_info->ActiveSize.cy = mode->cy;
	signal_info->PixelRate = mode->cx * mode->cy * REFRESH_RATE;
	signal_info->VSyncFreq.Numerator = REFRESH_RATE * 1000;
	signal_info->VSyncFreq.Denominator = 1000;
	signal_info->HSyncFreq.Numerator = (UINT)((signal_info->PixelRate / signal_info->TotalSize.cy) * 1000);
	signal_info->HSyncFreq.Denominator = 1000;

	signal_info->ScanLineOrdering = D3DDDI_VSSLO_PROGRESSIVE;

	status = target_mode_set_if->pfnAddMode(tgt_mode_set_hdl, target_mode);
	if (!NT_SUCCESS(status)) {
		DPT("pfnAddMode failed: 0x%x", status);
		target_mode_set_if->pfnReleaseModeInfo(tgt_mode_set_hdl, target_mode);
		return status; 
	}

	return status;
}

static NTSTATUS
update_target_modes(
	CONST D3DKMDT_HVIDPN vidpn_hdl,
	CONST D3DKMDT_VIDPN_PRESENT_PATH *curr_path_info,
	CONST DXGK_VIDPN_INTERFACE* vidpn_if)
{
	NTSTATUS status = STATUS_SUCCESS;
	D3DKMDT_HVIDPNTARGETMODESET tgt_mode_set_hdl = NULL;
	CONST DXGK_VIDPNTARGETMODESET_INTERFACE *target_mode_set_if;
	CONST D3DKMDT_VIDPN_TARGET_MODE *tgt_mode_info = NULL;

	status = vidpn_if->pfnAcquireTargetModeSet(vidpn_hdl,
		curr_path_info->VidPnTargetId,
		&tgt_mode_set_hdl,
		&target_mode_set_if);
	if (!NT_SUCCESS(status)) {
		DPT("** pfnAcquireTargetModeSet err=0x%X\n", status );
		return status;
	}

	status = target_mode_set_if->pfnAcquirePinnedModeInfo(tgt_mode_set_hdl, &tgt_mode_info);
	if (!NT_SUCCESS(status)) {
		vidpn_if->pfnReleaseTargetModeSet(vidpn_hdl, tgt_mode_set_hdl);
		DPT("** pfnAcquirePinnedModeInfo(Source) err=0x%X\n", status );
		return status;
	}

	////
	if (tgt_mode_info) {
		target_mode_set_if->pfnReleaseModeInfo(tgt_mode_set_hdl, tgt_mode_info);
	}
	vidpn_if->pfnReleaseTargetModeSet(vidpn_hdl, tgt_mode_set_hdl);
	tgt_mode_set_hdl = NULL;

	if (status == STATUS_SUCCESS && tgt_mode_info != NULL) {
		DPT("Target Mode Pinned Mode: 0x%X -> 0x%X\n", curr_path_info->VidPnSourceId, curr_path_info->VidPnTargetId);
		return STATUS_SUCCESS;///已经绑定了，不处理
		///
	}

	/////
	status = vidpn_if->pfnCreateNewTargetModeSet(vidpn_hdl,
		curr_path_info->VidPnTargetId,
		&tgt_mode_set_hdl,
		&target_mode_set_if);
	if (!NT_SUCCESS(status)) {
		DPT("** pfnCreateNewTargetModeSet err=0x%X\n", status );
		return status;
	}

	///add target 
	for (INT i = 0; i < sizeof(Modes) / sizeof(Modes[0]); ++i) {

		status = add_target_mode(tgt_mode_set_hdl, target_mode_set_if, &Modes[i]);

		if (!NT_SUCCESS(status)) {
			///
			vidpn_if->pfnReleaseTargetModeSet(vidpn_hdl, tgt_mode_set_hdl);
			DPT("add_target_mode err=0x%X\n", status);
			return status;
		}
		///
	}

	//////
	status = vidpn_if->pfnAssignTargetModeSet(vidpn_hdl,
		curr_path_info->VidPnTargetId,
		tgt_mode_set_hdl);

	if (!NT_SUCCESS(status)) {
		DPT("** pfnAssignTargetModeSet err=0x%x\n", status );
		vidpn_if->pfnReleaseTargetModeSet(vidpn_hdl, tgt_mode_set_hdl);
		return status;
	}
	return status;
}

static NTSTATUS DxgkDdiEnumVidPnCofuncModality_modify(CONST DXGKARG_ENUMVIDPNCOFUNCMODALITY* CONST  arg)
{
	NTSTATUS status = STATUS_SUCCESS;
	D3DKMDT_HVIDPN hConstrainingVidPn = arg->hConstrainingVidPn;
	CONST DXGK_VIDPN_INTERFACE* vidpn_if;

	status = wf->DxgkInterface.DxgkCbQueryVidPnInterface(
		hConstrainingVidPn,
		DXGK_VIDPN_INTERFACE_VERSION_V1,
		&vidpn_if);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	////
	D3DKMDT_HVIDPNTOPOLOGY topology_handle = NULL;
	CONST DXGK_VIDPNTOPOLOGY_INTERFACE*		topology_if = NULL;
	CONST D3DKMDT_VIDPN_PRESENT_PATH *curr_path_info;
	CONST D3DKMDT_VIDPN_PRESENT_PATH *next_path_info;

	status = vidpn_if->pfnGetTopology(hConstrainingVidPn,
		&topology_handle, &topology_if);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	////
	status = topology_if->pfnAcquireFirstPathInfo(topology_handle, &curr_path_info);
	if (status == STATUS_GRAPHICS_DATASET_IS_EMPTY) {
		// Empty topology, nothing to do. 
		DPT("pfnAcquireFirstPathInfo: Empty topology.\n");
		return STATUS_SUCCESS;
	}
	else if (!NT_SUCCESS(status)) {
		DPT("pfnAcquireFirstPathInfo failed: 0x%x", status);
		return STATUS_NO_MEMORY; ////
	}

	////
	do {
		////对于每个路径
		DPT("0x%X --> 0x%X\n", curr_path_info->VidPnSourceId, curr_path_info->VidPnTargetId);

		if (curr_path_info->VidPnTargetId == VIDPN_CHILD_UDID) {//路径目标是我们自己的
			///
			if ((arg->EnumPivotType != D3DKMDT_EPT_VIDPNSOURCE) ||
				(arg->EnumPivot.VidPnSourceId != curr_path_info->VidPnSourceId))
			{
				/////
				
				status = update_source_modes(arg->hConstrainingVidPn, curr_path_info, vidpn_if);  DPT("update_source_modes st=0x%X\n",status );
				if (!NT_SUCCESS(status)) {
					DPT("** update_source_modes err=0x%X\n", status );
				}
				//////
			}

			/////
			if ((arg->EnumPivotType != D3DKMDT_EPT_VIDPNTARGET) ||
				(arg->EnumPivot.VidPnTargetId != curr_path_info->VidPnTargetId))
			{
				status = update_target_modes(arg->hConstrainingVidPn, curr_path_info, vidpn_if);  DPT("update_target_modes st=0x%X\n", status);
				if (!NT_SUCCESS(status)) {
					DPT("** update_target_modes err=0x%X\n", status);
				}
			}
			////////
		}

		///next 
		status = topology_if->pfnAcquireNextPathInfo(topology_handle, curr_path_info, &next_path_info);
		topology_if->pfnReleasePathInfo(topology_handle, curr_path_info);
		curr_path_info = next_path_info;

		if (status == STATUS_GRAPHICS_NO_MORE_ELEMENTS_IN_DATASET) { /// end
			curr_path_info = NULL;
	//		DPT("pfnAcquireNextPathInfo no more data.\n");
			break;
		}
		else if (!NT_SUCCESS(status)) {
			curr_path_info = NULL;
			DPT("pfnAcquireNextPathInfo err=0x%X\n", status );
			break;
		}

		/////
	} while (TRUE);

	return status;
}

NTSTATUS DxgkDdiEnumVidPnCofuncModality(CONST HANDLE  hAdapter,
	CONST DXGKARG_ENUMVIDPNCOFUNCMODALITY* CONST  pEnumCofuncModalityArg)
{
	NTSTATUS status = STATUS_SUCCESS;

	DPT("DxgkDdiEnumVidPnCofuncModality: type=%d, 0x%X -> 0x%X,  [%d, %d]\n", 
		pEnumCofuncModalityArg->EnumPivotType, pEnumCofuncModalityArg->EnumPivot.VidPnSourceId, 
		pEnumCofuncModalityArg->EnumPivot.VidPnTargetId, wf->vidpn_source_count, wf->vidpn_target_count );
	///
	DxgkDdiEnumVidPnCofuncModality_modify(pEnumCofuncModalityArg); ////

	////
	status = wf->orgDpiFunc.DxgkDdiEnumVidPnCofuncModality(hAdapter, pEnumCofuncModalityArg);
	if (!NT_SUCCESS(status)) {
		DPT("** DxgkDdiEnumVidPnCofuncModality err=0x%X\n", status );
	}
	return status;
}

NTSTATUS DxgkDdiIsSupportedVidPn(
	IN_CONST_HANDLE hAdapter,
	INOUT_PDXGKARG_ISSUPPORTEDVIDPN pIsSupportedVidPn)
{
	NTSTATUS status;

	status = wf->orgDpiFunc.DxgkDdiIsSupportedVidPn(hAdapter, pIsSupportedVidPn);

	if (NT_SUCCESS(status)) {
		DPT("DxgkDdiIsSupportedVidPn handle=%p, supported=%d, \n", pIsSupportedVidPn->hDesiredVidPn, pIsSupportedVidPn->IsVidPnSupported );
	}
	else {
		DPT("** DxgkDdiIsSupportedVidPn err=0x%X, handle=%p, supported=%d\n", status , pIsSupportedVidPn->hDesiredVidPn, pIsSupportedVidPn->IsVidPnSupported );
	}

	return status;
}

NTSTATUS DxgkDdiCommitVidPn(
	IN_CONST_HANDLE hAdapter,
	IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn)
{
	NTSTATUS status;

	status = wf->orgDpiFunc.DxgkDdiCommitVidPn(hAdapter, pCommitVidPn );

//	if (!NT_SUCCESS(status)) {
		///
		DPT(" DxgkDdiCommitVidPn st=0x%X\n", status );
//	}

	////
	return status;
}

NTSTATUS DxgkDdiSetVidPnSourceVisibility(
	IN_CONST_HANDLE hAdapter,
	IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY pSetVidPnSourceVisibility)
{
	NTSTATUS status;

	status = wf->orgDpiFunc.DxgkDdiSetVidPnSourceVisibility(hAdapter, pSetVidPnSourceVisibility);

	DPT(" DxgkDdiSetVidPnSourceVisibility sourceId=0x%X, visible=0x%X, st=0x%X\n", pSetVidPnSourceVisibility->VidPnSourceId, pSetVidPnSourceVisibility->Visible ,status );

	return status;
}

NTSTATUS APIENTRY DxgkDdiSetVidPnSourceAddress(
	 const HANDLE                        hAdapter,
	 const DXGKARG_SETVIDPNSOURCEADDRESS *pSetVidPnSourceAddress)
{
	NTSTATUS status;

	status = wf->orgDpiFunc.DxgkDdiSetVidPnSourceAddress(hAdapter, pSetVidPnSourceAddress );

	DPT("DxgkDdiSetVidPnSourceAddress sourceId=0x%X, paddr=%llu, st=0x%X\n", pSetVidPnSourceAddress->VidPnSourceId, pSetVidPnSourceAddress->PrimaryAddress.QuadPart, status );
	return status;
}

