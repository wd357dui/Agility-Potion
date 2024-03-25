#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <D3DX12.h> // get the latest D3D12_SDK_VERSION

#if !defined(_M_AMD64) || !defined(_M_X64) || !defined(_WIN64)
#error this dll should only target x64
#endif

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
);

typedef HRESULT(WINAPI* PFN_D3D12_ENABLE_EXPERIMENTAL_FEATURES)(
	UINT								NumFeatures,
	__in_ecount(NumFeatures) const IID*	pIIDs,
	__in_ecount_opt(NumFeatures) void*	pConfigurationStructs,
	__in_ecount_opt(NumFeatures) UINT*	pConfigurationStructSizes);

PFN_D3D12_CREATE_DEVICE D3D12CreateDevice_Original;
PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface_Original = NULL;
PFN_D3D12_SERIALIZE_ROOT_SIGNATURE D3D12SerializeRootSignature_Original = NULL;
PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER D3D12CreateRootSignatureDeserializer_Original = NULL;
PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature_Original = NULL;
PFN_D3D12_CREATE_VERSIONED_ROOT_SIGNATURE_DESERIALIZER D3D12CreateVersionedRootSignatureDeserializer_Original = NULL;
PFN_D3D12_GET_INTERFACE D3D12GetInterface_Original = NULL;
PFN_D3D12_ENABLE_EXPERIMENTAL_FEATURES D3D12EnableExperimentalFeatures_Original = NULL;

typedef FARPROC(WINAPI* PFN_GET_PROC_ADDRESS)(_In_ HMODULE hModule, _In_ LPCSTR lpProcName);
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

HMODULE D3D12_DLL = NULL;
HMODULE KERNEL_BASE_DLL = NULL;
PFN_GET_PROC_ADDRESS GetProcAddress_Original = NULL;
constexpr SIZE_T DetourByteCodeCount = 2 + 4 + 8;
constexpr SIZE_T Detour_Address64_ByteCodeIndex = 2 + 4;
BYTE GetProcAddress_OriginalByteCode[DetourByteCodeCount]{ 0x00 };
BYTE GetProcAddress_HookedByteCode[DetourByteCodeCount]{
	0xFF, 0x25, // jmpf
	0x00, 0x00, 0x00, 0x00, // absolute
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64-bit target address
};

void BackupGetProcAddress();
void PatchGetProcAddress();
void UnpatchGetProcAddress();
FARPROC WINAPI GetProcAddressHook(_In_ HMODULE hModule, _In_ LPCSTR lpProcName);
void CaluclateDetourJmpDst64();
void LoadOriginalDll();

HRESULT WINAPI D3D12CreateDevice(
	_In_opt_ IUnknown* pAdapter,
	D3D_FEATURE_LEVEL MinimumFeatureLevel,
	_In_ REFIID riid, // Expected: ID3D12Device
	_COM_Outptr_opt_ void** ppDevice)
{
	LoadOriginalDll();
	if (D3D12CreateDevice_Original == NULL) {
		D3D12CreateDevice_Original = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(D3D12_DLL, "D3D12CreateDevice");
	}
	return D3D12CreateDevice_Original(pAdapter, MinimumFeatureLevel, riid, ppDevice);
}

HRESULT WINAPI D3D12GetDebugInterface(_In_ REFIID riid, _COM_Outptr_opt_ void** ppvDebug)
{
	LoadOriginalDll();
	if (D3D12GetDebugInterface_Original == NULL) {
		D3D12GetDebugInterface_Original = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(D3D12_DLL, "D3D12GetDebugInterface");
	}
	return D3D12GetDebugInterface_Original(riid, ppvDebug);
}

HRESULT WINAPI D3D12EnableExperimentalFeatures(
	UINT                                    NumFeatures,
	_In_count_(NumFeatures) const IID* pIIDs,
	_In_opt_count_(NumFeatures) void* pConfigurationStructs,
	_In_opt_count_(NumFeatures) UINT* pConfigurationStructSizes)
{
	LoadOriginalDll();
	if (D3D12EnableExperimentalFeatures_Original == NULL) {
		D3D12EnableExperimentalFeatures_Original = (PFN_D3D12_ENABLE_EXPERIMENTAL_FEATURES)GetProcAddress(D3D12_DLL, "D3D12EnableExperimentalFeatures");
	}
	return D3D12EnableExperimentalFeatures_Original(NumFeatures, pIIDs, pConfigurationStructs, pConfigurationStructSizes);
}

HRESULT WINAPI D3D12SerializeRootSignature(
	_In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
	_In_ D3D_ROOT_SIGNATURE_VERSION Version,
	_Out_ ID3DBlob** ppBlob,
	_Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
{
	LoadOriginalDll();
	if (D3D12SerializeRootSignature_Original == NULL) {
		D3D12SerializeRootSignature_Original = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)GetProcAddress(D3D12_DLL, "D3D12SerializeRootSignature");
	}
	return D3D12SerializeRootSignature_Original(pRootSignature, Version, ppBlob, ppErrorBlob);
}

HRESULT WINAPI D3D12CreateRootSignatureDeserializer(
	_In_reads_bytes_(SrcDataSizeInBytes) LPCVOID pSrcData,
	_In_ SIZE_T SrcDataSizeInBytes,
	_In_ REFIID pRootSignatureDeserializerInterface,
	_Out_ void** ppRootSignatureDeserializer)
{
	LoadOriginalDll();
	if (D3D12CreateRootSignatureDeserializer_Original == NULL) {
		D3D12CreateRootSignatureDeserializer_Original = (PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER)GetProcAddress(D3D12_DLL, "D3D12CreateRootSignatureDeserializer");
	}
	return D3D12CreateRootSignatureDeserializer_Original(pSrcData, SrcDataSizeInBytes, pRootSignatureDeserializerInterface, ppRootSignatureDeserializer);
}

HRESULT WINAPI D3D12SerializeVersionedRootSignature(
	_In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
	_Out_ ID3DBlob** ppBlob,
	_Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
{
	LoadOriginalDll();
	if (D3D12SerializeVersionedRootSignature_Original == NULL) {
		D3D12SerializeVersionedRootSignature_Original = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress(D3D12_DLL, "D3D12SerializeVersionedRootSignature");
	}
	return D3D12SerializeVersionedRootSignature_Original(pRootSignature, ppBlob, ppErrorBlob);
}

HRESULT WINAPI D3D12CreateVersionedRootSignatureDeserializer(
	_In_reads_bytes_(SrcDataSizeInBytes) LPCVOID pSrcData,
	_In_ SIZE_T SrcDataSizeInBytes,
	_In_ REFIID pRootSignatureDeserializerInterface,
	_Out_ void** ppRootSignatureDeserializer)
{
	LoadOriginalDll();
	if (D3D12CreateVersionedRootSignatureDeserializer_Original == NULL) {
		D3D12CreateVersionedRootSignatureDeserializer_Original = (PFN_D3D12_CREATE_VERSIONED_ROOT_SIGNATURE_DESERIALIZER)GetProcAddress(D3D12_DLL, "D3D12CreateVersionedRootSignatureDeserializer");
	}
	return D3D12CreateVersionedRootSignatureDeserializer_Original(pSrcData, SrcDataSizeInBytes, pRootSignatureDeserializerInterface, ppRootSignatureDeserializer);
}

HRESULT WINAPI D3D12GetInterface(_In_ REFCLSID rclsid, _In_ REFIID riid, _COM_Outptr_opt_ void** ppvDebug)
{
	LoadOriginalDll();
	if (D3D12GetInterface_Original == NULL) {
		D3D12GetInterface_Original = (PFN_D3D12_GET_INTERFACE)GetProcAddress(D3D12_DLL, "D3D12GetInterface");
	}
	return D3D12GetInterface_Original(rclsid, riid, ppvDebug);
}

void BackupGetProcAddress()
{
	DWORD Old;
	VirtualProtect(GetProcAddress_Original, DetourByteCodeCount, PAGE_EXECUTE_READWRITE, &Old);
	CopyMemory(GetProcAddress_OriginalByteCode, GetProcAddress_Original, DetourByteCodeCount);
	VirtualProtect(GetProcAddress_Original, DetourByteCodeCount, Old, &Old);
}

void PatchGetProcAddress()
{
	DWORD Old;
	VirtualProtect(GetProcAddress_Original, DetourByteCodeCount, PAGE_EXECUTE_READWRITE, &Old);
	CopyMemory(GetProcAddress_Original, GetProcAddress_HookedByteCode, DetourByteCodeCount);
	VirtualProtect(GetProcAddress_Original, DetourByteCodeCount, Old, &Old);
}

void UnpatchGetProcAddress()
{
	DWORD Old;
	VirtualProtect(GetProcAddress_Original, DetourByteCodeCount, PAGE_EXECUTE_READWRITE, &Old);
	CopyMemory(GetProcAddress_Original, GetProcAddress_OriginalByteCode, DetourByteCodeCount);
	VirtualProtect(GetProcAddress_Original, DetourByteCodeCount, Old, &Old);
}

FARPROC WINAPI GetProcAddressHook(_In_ HMODULE hModule, _In_ LPCSTR lpProcName) {
	if (strcmp("D3D12SDKVersion", lpProcName) == 0) {
		return (FARPROC)&D3D12SDKVersion;
	}
	else if (strcmp("D3D12SDKPath", lpProcName) == 0) {
		return (FARPROC)&D3D12SDKPath;
	}
	UnpatchGetProcAddress();
	FARPROC result = GetProcAddress_Original(hModule, lpProcName);
	PatchGetProcAddress();
	return result;
}

void CaluclateDetourJmpDst64()
{
	*reinterpret_cast<UINT64*>(GetProcAddress_HookedByteCode + Detour_Address64_ByteCodeIndex) = reinterpret_cast<UINT64>(GetProcAddressHook);
}

#pragma warning (push)
#pragma warning (disable : 6387)
void LoadOriginalDll()
{
	if (D3D12_DLL == NULL)
	{
		if (KERNEL_BASE_DLL == NULL) {
			KERNEL_BASE_DLL = LoadLibraryW(L"KernelBase.dll");
		}
		if (GetProcAddress_Original == NULL) {
			GetProcAddress_Original = (PFN_GET_PROC_ADDRESS)GetProcAddress(KERNEL_BASE_DLL, "GetProcAddress");
		}

		CaluclateDetourJmpDst64();

		BackupGetProcAddress();
		PatchGetProcAddress();
		D3D12_DLL = LoadLibraryW(L"C:\\Windows\\System32\\D3D12.dll");
		UnpatchGetProcAddress();
	}
}
#pragma warning (pop)

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		LoadOriginalDll();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
