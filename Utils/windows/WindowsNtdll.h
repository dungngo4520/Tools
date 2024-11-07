#pragma once
#define UMDF_USING_NTSTATUS
#include <Windows.h>
#include <winternl.h>

namespace utils {
    namespace windows {
        namespace ntdll {
            enum SYSTEM_INFORMATION_CLASS {
                SystemBasicInformation = 0x00,
                SystemProcessorInformation = 0x01,
                SystemPerformanceInformation = 0x02,
                SystemTimeOfDayInformation = 0x03,
                SystemPathInformation = 0x04,
                SystemProcessInformation = 0x05,
                SystemCallCountInformation = 0x06,
                SystemDeviceInformation = 0x07,
                SystemProcessorPerformanceInformation = 0x08,
                SystemFlagsInformation = 0x09,
                SystemCallTimeInformation = 0x0A,
                SystemModuleInformation = 0x0B,
                SystemLocksInformation = 0x0C,
                SystemStackTraceInformation = 0x0D,
                SystemPagedPoolInformation = 0x0E,
                SystemNonPagedPoolInformation = 0x0F,
                SystemHandleInformation = 0x10,
                SystemObjectInformation = 0x11,
                SystemPageFileInformation = 0x12,
                SystemVdmInstemulInformation = 0x13,
                SystemFileCacheInformation = 0x15,
                SystemPoolTagInformation = 0x16,
                SystemInterruptInformation = 0x17,
                SystemDpcBehaviorInformation = 0x18,
                SystemFullMemoryInformation = 0x19,
                SystemTimeAdjustmentInformation = 0x1C,
                SystemSummaryMemoryInformation = 0x1D,
                SystemNextEventIdInformation = 0x1E,
                SystemEventIdsInformation = 0x1F,
                SystemCrashDumpInformation = 0x20,
                SystemExceptionInformation = 0x21,
                SystemCrashDumpStateInformation = 0x22,
                SystemKernelDebuggerInformation = 0x23,
                SystemContextSwitchInformation = 0x24,
                SystemRegistryQuotaInformation = 0x25,
                SystemPlugPlayBusInformation = 0x28,
                SystemDockInformation = 0x29,
                SystemProcessorIdleInformation = 0x2A,
                SystemLegacyDriverInformation = 0x2B,
                SystemCurrentTimeZoneInformation = 0x2C,
                SystemLookasideInformation = 0x2D,
                SystemRangeStartInformation = 0x32,
                SystemVerifierInformation = 0x33,
                SystemSessionProcessInformation = 0x35,
                SystemPrefetcherInformation = 0x38,
                SystemExtendedProcessInformation = 0x39,
                SystemRecommendedSharedDataAlignment = 0x3A,
                SystemComPlusPackage = 0x3B,
                SystemNumaAvailableMemory = 0x3C,
                SystemProcessorPowerInformation = 0x3D,
                SystemEmulationBasicInformation = 0x3E,
                SystemEmulationProcessorInformation = 0x3F,
                SystemExtendedHandleInformation = 0x40,
                SystemLostDelayedWriteInformation = 0x41,
                SystemSessionPoolTagInformation = 0x43,
                SystemSessionMappedViewInformation = 0x44,
                SystemHotpatchInformation = 0x45,
                SystemWatchdogTimerInformation = 0x48,
                SystemLogicalProcessorInformation = 0x49,
                SystemFirmwareTableInformation = 0x4C,
                SystemModuleInformationEx = 0x4D,
                SystemSuperfetchInformation = 0x4F,
                SystemMemoryListInformation = 0x50,
                SystemFileCacheInformationEx = 0x51,
                SystemProcessorIdleCycleTimeInformation = 0x53,
                SystemVerifierCancellationInformation = 0x54,
                SystemRefTraceInformation = 0x56,
                SystemSpecialPoolInformation = 0x57,
                SystemProcessIdInformation = 0x58,
                SystemBootEnvironmentInformation = 0x5A,
                SystemHypervisorInformation = 0x5B,
                SystemVerifierInformationEx = 0x5C,
                SystemCoverageInformation = 0x5F,
                SystemPrefetchPatchInformation = 0x60,
                SystemSystemPartitionInformation = 0x62,
                SystemSystemDiskInformation = 0x63,
                SystemProcessorPerformanceDistribution = 0x64,
                SystemNumaProximityNodeInformation = 0x65,
                SystemDynamicTimeZoneInformation = 0x66,
                SystemCodeIntegrityInformation = 0x67,
                SystemProcessorBrandString = 0x69,
                SystemVirtualAddressInformation = 0x6A,
                SystemProcessorCycleTimeInformation = 0x6C,
                SystemStoreInformation = 0x6D,
                SystemVhdBootInformation = 0x70,
                SystemCpuQuotaInformation = 0x71,
                SystemNativeBasicInformation = 0x72,
                SystemErrorPortTimeouts = 0x73,
                SystemLowPriorityIoInformation = 0x74,
                SystemBootEntropyInformation = 0x75,
                SystemVerifierCountersInformation = 0x76,
                SystemPagedPoolInformationEx = 0x77,
                SystemSystemPtesInformationEx = 0x78,
                SystemAcpiAuditInformation = 0x7A,
                SystemBasicPerformanceInformation = 0x7B,
                SystemQueryPerformanceCounterInformation = 0x7C,
                SystemSessionBigPoolInformation = 0x7D,
                SystemBootGraphicsInformation = 0x7E,
                SystemBadPageInformation = 0x80,
                SystemPlatformBinaryInformation = 0x85,
                SystemPolicyInformation = 0x86,
                SystemHypervisorProcessorCountInformation = 0x87,
                SystemDeviceDataInformation = 0x88,
                SystemDeviceDataEnumerationInformation = 0x89,
                SystemMemoryTopologyInformation = 0x8A,
                SystemMemoryChannelInformation = 0x8B,
                SystemBootLogoInformation = 0x8C,
                SystemProcessorPerformanceInformationEx = 0x8D,
                SystemSecureBootPolicyInformation = 0x8F,
                SystemPageFileInformationEx = 0x90,
                SystemSecureBootInformation = 0x91,
                SystemPortableWorkspaceEfiLauncherInformation = 0x93,
                SystemFullProcessInformation = 0x94,
                SystemKernelDebuggerInformationEx = 0x95,
                SystemBootMetadataInformation = 0x96,
                SystemSoftRebootInformation = 0x97,
                SystemOfflineDumpConfigInformation = 0x99,
                SystemProcessorFeaturesInformation = 0x9A,
                SystemEdidInformation = 0x9C,
                SystemManufacturingInformation = 0x9D,
                SystemEnergyEstimationConfigInformation = 0x9E,
                SystemHypervisorDetailInformation = 0x9F,
                SystemProcessorCycleStatsInformation = 0xA0,
                SystemTrustedPlatformModuleInformation = 0xA2,
                SystemKernelDebuggerFlags = 0xA3,
                SystemCodeIntegrityPolicyInformation = 0xA4,
                SystemIsolatedUserModeInformation = 0xA5,
                SystemHardwareSecurityTestInterfaceResultsInformation = 0xA6,
                SystemSingleModuleInformation = 0xA7,
                SystemDmaProtectionInformation = 0xA9,
                SystemSecureBootPolicyFullInformation = 0xAB,
                SystemCodeIntegrityPolicyFullInformation = 0xAC,
                SystemAffinitizedInterruptProcessorInformation = 0xAD,
                SystemRootSiloInformation = 0xAE,
                SystemCpuSetInformation = 0xAF,
                SystemSecureKernelProfileInformation = 0xB2,
                SystemCodeIntegrityPlatformManifestInformation = 0xB3,
                SystemInterruptSteeringInformation = 0xB4,
                SystemSupportedProcessorArchitectures = 0xB5,
                SystemMemoryUsageInformation = 0xB6,
                SystemCodeIntegrityCertificateInformation = 0xB7,
                SystemPhysicalMemoryInformation = 0xB8,
                SystemControlFlowTransition = 0xB9,
                SystemKernelDebuggingAllowed = 0xBA,
                SystemActivityModerationUserSettings = 0xBC,
                SystemCodeIntegrityPoliciesFullInformation = 0xBD,
                SystemCodeIntegrityUnlockInformation = 0xBE,
                SystemFlushInformation = 0xC0,
                SystemProcessorIdleMaskInformation = 0xC1,
                SystemWriteConstraintInformation = 0xC3,
                SystemKernelVaShadowInformation = 0xC4,
                SystemHypervisorSharedPageInformation = 0xC5,
                SystemFirmwareBootPerformanceInformation = 0xC6,
                SystemCodeIntegrityVerificationInformation = 0xC7,
                SystemFirmwarePartitionInformation = 0xC8,
                SystemSpeculationControlInformation = 0xC9,
                SystemDmaGuardPolicyInformation = 0xCA,
            };

            enum OBJECT_INFORMATION_CLASS {
                ObjectBasicInformation = 0,
                ObjectNameInformation = 1,
                ObjectTypeInformation = 2,
                ObjectTypesInformation = 3,
                ObjectHandleFlagInformation = 4,
                ObjectSessionInformation = 5,
                ObjectSessionObjectInformation = 6,
                MaxObjectInfoClass = 7,
            };

            enum POOL_TYPE {
                NonPagedPool,
                PagedPool,
                NonPagedPoolMustSucceed,
                DontUseThisType,
                NonPagedPoolCacheAligned,
                PagedPoolCacheAligned,
                NonPagedPoolCacheAlignedMustS
            };

            typedef struct _UNICODE_STRING {
                USHORT Length;
                USHORT MaximumLength;
                PWSTR Buffer;
            } UNICODE_STRING, *PUNICODE_STRING;

            typedef struct _SYSTEM_HANDLE {
                ULONG ProcessId;
                BYTE ObjectTypeNumber;
                BYTE Flags;
                USHORT Handle;
                PVOID Object;
                ACCESS_MASK GrantedAccess;
            } SYSTEM_HANDLE, *PSYSTEM_HANDLE;

            typedef struct _SYSTEM_HANDLE_INFORMATION {
                ULONG HandleCount;
                SYSTEM_HANDLE Handles[1];
            } SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

            typedef struct _OBJECT_TYPE_INFORMATION {
                UNICODE_STRING Name;
                ULONG TotalNumberOfObjects;
                ULONG TotalNumberOfHandles;
                ULONG TotalPagedPoolUsage;
                ULONG TotalNonPagedPoolUsage;
                ULONG TotalNamePoolUsage;
                ULONG TotalHandleTableUsage;
                ULONG HighWaterNumberOfObjects;
                ULONG HighWaterNumberOfHandles;
                ULONG HighWaterPagedPoolUsage;
                ULONG HighWaterNonPagedPoolUsage;
                ULONG HighWaterNamePoolUsage;
                ULONG HighWaterHandleTableUsage;
                ULONG InvalidAttributes;
                GENERIC_MAPPING GenericMapping;
                ULONG ValidAccess;
                BOOLEAN SecurityRequired;
                BOOLEAN MaintainHandleCount;
                USHORT MaintainTypeList;
                POOL_TYPE PoolType;
                ULONG PagedPoolUsage;
                ULONG NonPagedPoolUsage;
            } OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

            typedef struct _SYSTEM_PROCESS_INFO {
                ULONG NextEntryOffset;
                ULONG NumberOfThreads;
                LARGE_INTEGER Reserved[3];
                LARGE_INTEGER CreateTime;
                LARGE_INTEGER UserTime;
                LARGE_INTEGER KernelTime;
                UNICODE_STRING ImageName;
                ULONG BasePriority;
                HANDLE ProcessId;
                HANDLE InheritedFromProcessId;
            } SYSTEM_PROCESS_INFO, *PSYSTEM_PROCESS_INFO;


            NTSTATUS Load();

            NTSTATUS QuerySystemInformation(
                SYSTEM_INFORMATION_CLASS SystemInformationClass,
                PVOID SystemInformation,
                ULONG SystemInformationLength,
                PULONG ReturnLength
            );

            NTSTATUS DuplicateObject(
                HANDLE SourceProcessHandle,
                HANDLE SourceHandle,
                HANDLE TargetProcessHandle,
                PHANDLE TargetHandle,
                ACCESS_MASK DesiredAccess,
                ULONG Attributes,
                ULONG Options
            );

            NTSTATUS QueryObject(
                HANDLE Handle,
                OBJECT_INFORMATION_CLASS ObjectInformationClass,
                PVOID ObjectInformation,
                ULONG ObjectInformationLength,
                PULONG ReturnLength
            );
        };  // namespace ntdll
    };  // namespace windows
};  // namespace utils
