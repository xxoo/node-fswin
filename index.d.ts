export declare module DirWatcher {
    export type Callback = (event: Event, message: Message | string) => void;

    export interface Options {
        WATCH_SUB_DIRECTORIES?: boolean;
        CHANGE_FILE_SIZE?: boolean;
        CHANGE_LAST_WRITE?: boolean;
        CHANGE_LAST_ACCESS?: boolean;
        CHANGE_CREATION?: boolean;
        CHANGE_ATTRIBUTES?: boolean;
        CHANGE_SECURITY?: boolean;
    }

    export interface Message {
        OLD_NAME?: string;
        NEW_NAME?: string;
    }

    export type Event =
        | 'STARTED'
        | 'MOVED'
        | 'ADDED'
        | 'REMOVED'
        | 'MODIFIED'
        | 'RENAMED'
        | 'ENDED'
        | 'ERROR';
}

export enum EjectDriveMethod {
    DEVICE_IO_CONTROL = 0, // works with optical drive
    CM_REQUEST_DEVICE_EJECT_W = 1, // works with USB flash drive
    HOT_PLUG_EJECT_DEVICE = 2, // works with most media types but requires user interaction and HotPlug.dll. Only exists in system32 folder which means it is not possible to use this method when you execute a 32bit node process in a 64bit Windows.
}

export declare module Find {
    export interface File {
        LONG_NAME: string;
        SHORT_NAME: string;
        CREATION_TIME: Date;
        LAST_ACCESS_TIME: Date;
        LAST_WRITE_TIME: Date;
        SIZE: number;
        RAW_ATTRIBUTES: number;
        IS_ARCHIVED: boolean;
        IS_COMPRESSED: boolean;
        IS_DEVICE: boolean;
        IS_DIRECTORY: boolean;
        IS_ENCRYPTED: boolean;
        IS_HIDDEN: boolean;
        IS_NOT_CONTENT_INDEXED: boolean;
        IS_OFFLINE: boolean;
        IS_READ_ONLY: boolean;
        IS_SPARSE_FILE: boolean;
        IS_SYSTEM: boolean;
        IS_TEMPORARY: boolean;
        IS_INTEGRITY_STREAM: boolean;
        IS_NO_SCRUB_DATA: boolean;
        IS_RECALL_ON_DATA_ACCESS: boolean;
        IS_RECALL_ON_OPEN: boolean;
        IS_VIRTUAL: boolean;
        IS_EA: boolean;
        IS_PINNED: boolean;
        IS_UNPINNED: boolean;
        REPARSE_POINT_TAG: ReparsePointTagType;
    }

    export type ReparsePointTagType =
        | 'MOUNT_POINT'
        | 'HSM'
        | 'HSM2'
        | 'SIS'
        | 'WIM'
        | 'CSV'
        | 'DFS'
        | 'SYMLINK'
        | 'DFSR'
        | 'DEDUP'
        | 'NFS'
        | 'FILE_PLACEHOLDER'
        | 'WOF'
        | 'WCI'
        | 'WCI_1'
        | 'GLOBAL_REPARSE'
        | 'CLOUD'
        | 'CLOUD_1'
        | 'CLOUD_2'
        | 'CLOUD_3'
        | 'CLOUD_4'
        | 'CLOUD_5'
        | 'CLOUD_6'
        | 'CLOUD_7'
        | 'CLOUD_8'
        | 'CLOUD_9'
        | 'CLOUD_A'
        | 'CLOUD_B'
        | 'CLOUD_C'
        | 'CLOUD_D'
        | 'CLOUD_E'
        | 'CLOUD_F'
        | 'CLOUD_MASK'
        | 'APPEXECLINK'
        | 'PROJFS'
        | 'STORAGE_SYNC'
        | 'WCI_TOMBSTONE'
        | 'UNHANDLED'
        | 'ONEDRIVE'
        | 'PROJFS_TOMBSTONE'
        | 'AF_UNIX'
        | 'WCI_LINK'
        | 'WCI_LINK_1'
        | 'DATALESS_CIM'
        | ''
        | number;

    export type Event = 'FOUND' | 'SUCCEEDED'

    export type SyncCallback = (file: File) => void
    export type AsyncProgressiveCallback = (event: Event, message: File) => void
    export type AsyncBasicCallback = (files: File[]) => void

    export type ProgressiveModeEnabled = true
}

export interface Attributes {
    CREATION_TIME: Date;
    LAST_ACCESS_TIME: Date;
    LAST_WRITE_TIME: Date;
    SIZE: number;
    LINK_COUNT: number;
    RAW_ATTRIBUTES: number;
    IS_ARCHIVED: boolean;
    IS_COMPRESSED: boolean;
    IS_DEVICE: boolean;
    IS_DIRECTORY: boolean;
    IS_ENCRYPTED: boolean;
    IS_HIDDEN: boolean;
    IS_NOT_CONTENT_INDEXED: boolean;
    IS_OFFLINE: boolean;
    IS_READ_ONLY: boolean;
    IS_SPARSE_FILE: boolean;
    IS_SYSTEM: boolean;
    IS_TEMPORARY: boolean;
    IS_INTEGRITY_STREAM: boolean;
    IS_NO_SCRUB_DATA: boolean;
    IS_RECALL_ON_DATA_ACCESS: boolean;
    IS_RECALL_ON_OPEN: boolean;
    IS_VIRTUAL: boolean;
    IS_EA: boolean;
    IS_PINNED: boolean;
    IS_UNPINNED: boolean;
    IS_REPARSE_POINT: boolean;
}

export interface SetAttributes {
    IS_ARCHIVED?: boolean;
    IS_HIDDEN?: boolean;
    IS_NOT_CONTENT_INDEXED?: boolean;
    IS_OFFLINE?: boolean;
    IS_READ_ONLY?: boolean;
    IS_SYSTEM?: boolean;
    IS_TEMPORARY?: boolean;
    IS_UNPINNED?: boolean;
    IS_PINNED?: boolean;
}

export interface DeviceCapabilities {
    LOCK_SUPPORTED: boolean;
    EJECT_SUPPORTED: boolean;
    REMOVABLE: boolean;
    DOCK_DEVICE: boolean;
    UNIQUE_ID: boolean;
    SILENT_INSTALL: boolean;
    RAW_DEVICE_OK: boolean;
    SURPRISE_REMOVAL_OK: boolean;
    HARDWARE_DISABLED: boolean;
    NON_DYNAMIC: boolean;
}

export interface DriveDevice {
    deviceNumber: number;
    devicePath: string;
    deviceId: string;
    parentDeviceId: string;
}

export type DriveType =
    | 'NO_ROOT_DIR'
    | 'REMOVABLE'
    | 'FIXED'
    | 'REMOTE'
    | 'CDROM'
    | 'RAMDISK'
    | 'UNKNOWN';
export interface LogicalDriveList {
    [driveLetter: string]: DriveType;
}

export declare module StorageProperties {
    export interface Properties {
        deviceProperty?: boolean;
        adapterProperty?: boolean;
        deviceWriteCacheProperty?: boolean;
        accessAlignmentProperty?: boolean;
        deviceSeekPenalty?: boolean;
        deviceTrim?: boolean;
        deviceLBProvisioningProperty?: boolean;
        devicePowerProperty?: boolean;
        deviceCopyOffloadProperty?: boolean;
        deviceMediumProductType?: boolean;
        adapterRpmbProperty?: boolean;
        deviceIoCapabilityProperty?: boolean;
        adapterTemperatureProperty?: boolean;
        deviceTemperatureProperty?: boolean;
        adapterSerialNumber?: boolean
    }

    export type BusType =
        | 'SCSI'
        | 'ATAPI'
        | 'ATA'
        | '1394'
        | 'SSA'
        | 'Fibre'
        | 'USB'
        | 'RAID'
        | 'iSCSI'
        | 'SAS'
        | 'SATA'
        | 'SD'
        | 'MMC'
        | 'Virtual'
        | 'FileBackedVirtual'
        | 'Spaces'
        | 'NVMe'
        | 'SCM'
        | 'UFS'
        | number;

    export type SrbType =
        | 'SCSIRequestBlock'
        | 'StorageRequestBlock'
        | number;

    export type WriteCacheType =
        | 'unknown'
        | 'none'
        | 'writeBack'
        | 'writeThrough'
        | number;

    export type WriteCacheEnabledType =
        | 'unknown'
        | 'disabled'
        | 'enabled'
        | number;

    export type WriteCacheChangeableType =
        | 'unknown'
        | 'notChangeable'
        | 'changeable'
        | number;

    export type WriteCacheWriteThroughSupportedType =
        | 'unknown'
        | 'notSupported'
        | 'supported'
        | number;

    export interface DeviceProperty {
        deviceType: number;
        deviceTypeModifier: number;
        busType: BusType;
        commandQueueing: boolean;
        removableMedia: boolean;
        vendorId: string;
        productId: string;
        productRevision: string;
        serialNumber: string;
    }

    export interface AdapterProperty {
        maximumTransferLength: number;
        maximumPhysicalPages: number;
        alignmentMask: number;
        adapterUsesPio: boolean;
        adapterScansDown: boolean;
        commandQueueing: boolean;
        acceleratedTransfer: boolean;
        busMajorVersion: number;
        busMinorVersion: number;
        busType: BusType;
        srbType: SrbType;
        addressType: string;
    }

    export interface DeviceWriteCacheProperty {
        type: WriteCacheType;
        isEnabled: WriteCacheEnabledType;
        isChangeable: WriteCacheChangeableType;
        isWriteThroughSupported: WriteCacheWriteThroughSupportedType;
        flushCacheSupported: boolean;
        userDefinedPowerProtection: boolean;
        NVCacheEnabled: boolean;
    }

    export interface AccessAlignmentProperty {
        bytesPerCacheLine: number;
        bytesOffsetForCacheAlignment: number;
        bytesPerLogicalSector: number;
        bytesPerPhysicalSector: number;
        bytesOffsetForSectorAlignment: number;
    }

    export interface DeviceLBProvisioningProperty {
        thinProvisioningEnabled: boolean;
        thinProvisioningReadZeros: boolean;
        anchorSupported: boolean;
        unmapGranularityAlignmentValid: boolean;
        getFreeSpaceSupported: boolean;
        mapSupported: boolean;
        optimalUnmapGranularity: bigint;
        unmapGranularityAlignment: bigint;
        maxUnmapLbaCount: number;
        maxUnmapBlockDescriptorCount: number;
    }

    export interface DevicePowerProperty {
        deviceAttentionSupported: boolean;
        asynchronousNotificationSupported: boolean;
        idlePowerManagementEnabled: boolean;
        d3ColdEnabled: boolean;
        d3ColdSupported: boolean;
        noVerifyDuringIdlePower: boolean;
        idleTimeoutInMS: number;
    }

    export interface DeviceCopyOffloadProperty {
        maximumTokenLifetime: number;
        defaultTokenLifetime: number;
        maximumTransferSize: bigint;
        optimalTransferCount: bigint;
        maximumDataDescriptors: number;
        maximumTransferLengthPerDescriptor: number;
        optimalTransferLengthPerDescriptor: number;
        optimalTransferLengthGranularity: number;
    }

    export type DeviceMediumProductType =
        | 'CFast'
        | 'CompactFlash'
        | 'MemoryStick'
        | 'MultiMediaCard'
        | 'SecureDigitalCard'
        | 'QXD'
        | 'UniversalFlashStorage'
        | number;

    export interface AdapterRpmbProperty {
        sizeInBytes: number;
        maxReliableWriteSizeInBytes: number;
        frameFormat: number;
    }

    export interface DeviceIoCapabilityProperty {
        lunMaxIoCount: number;
        adapterMaxIoCount: number;
    }

    export interface TemperatureInfo {
        index: number;
        temperature: number;
        overThreshold: number;
        underThreshold: number;
        overThresholdChangable: boolean;
        underThresholdChangable: boolean;
        eventGenerated: boolean;
    }

    export interface DeviceTemperatureProperty {
        criticalTemperature: number;
        warningTemperature: number;
        temperatureInfo: TemperatureInfo[];
    }

    export interface RootObject {
        deviceProperty: DeviceProperty;
        adapterProperty: AdapterProperty;
        deviceWriteCacheProperty: DeviceWriteCacheProperty;
        accessAlignmentProperty: AccessAlignmentProperty;
        deviceSeekPenalty: boolean;
        deviceTrim: boolean;
        deviceLBProvisioningProperty: DeviceLBProvisioningProperty;
        devicePowerProperty: DevicePowerProperty;
        deviceCopyOffloadProperty: DeviceCopyOffloadProperty;
        deviceMediumProductType: DeviceMediumProductType;
        adapterRpmbProperty: AdapterRpmbProperty;
        deviceIoCapabilityProperty: DeviceIoCapabilityProperty;
        deviceTemperatureProperty: DeviceTemperatureProperty;
    }

}

export interface VolumeInformation {
    LABEL: string;
    FILESYSTEM: string;
    SERIALNUMBER: number;
    RAW_FLAGS: number;
    CASE_SENSITIVE_SEARCH: boolean;
    CASE_PRESERVED_NAMES: boolean;
    UNICODE_ON_DISK: boolean;
    PERSISTENT_ACLS: boolean;
    FILE_COMPRESSION: boolean;
    VOLUME_QUOTAS: boolean;
    RETURNS_CLEANUP_RESULT_INFO: boolean;
    VOLUME_IS_COMPRESSED: boolean;
    NAMED_STREAMS: boolean;
    READ_ONLY_VOLUME: boolean;
    SEQUENTIAL_WRITE_ONCE: boolean;
    DAX_VOLUME: boolean;
    SUPPORTS_SPARSE_FILES: boolean;
    SUPPORTS_REPARSE_POINTS: boolean;
    SUPPORTS_REMOTE_STORAGE: boolean;
    SUPPORTS_POSIX_UNLINK_RENAME: boolean;
    SUPPORTS_BYPASS_IO: boolean;
    SUPPORTS_OBJECT_IDS: boolean;
    SUPPORTS_ENCRYPTION: boolean;
    SUPPORTS_TRANSACTIONS: boolean;
    SUPPORTS_HARD_LINKS: boolean;
    SUPPORTS_EXTENDED_ATTRIBUTES: boolean;
    SUPPORTS_OPEN_BY_FILE_ID: boolean;
    SUPPORTS_USN_JOURNAL: boolean;
    SUPPORTS_INTEGRITY_STREAMS: boolean;
    SUPPORTS_BLOCK_REFCOUNTING: boolean;
    SUPPORTS_SPARSE_VDL: boolean;
    SUPPORTS_GHOSTING: boolean;
}

export interface VolumeSpace {
    FREE: number;
    TOTAL: number;
}

export interface SplitPath {
    PARENT: string;
    NAME: string;
}

// https://github.com/xxoo/node-fswin/wiki/convertPath-and-convertPathSync
export function convertPath(pathToConvert: string, callback: (result?: string) => void, isLong?: boolean): boolean;
export function convertPathSync(pathToConvert: string, isLong?: boolean): string | undefined;

// https://github.com/xxoo/node-fswin/wiki/dirWatcher
export class dirWatcher {
    constructor(dirToWatch: string, callback: DirWatcher.Callback, options?: DirWatcher.Options);
    close(): boolean;
}

// https://github.com/xxoo/node-fswin/wiki/ejectDrive-and-ejectDriveSync
export function ejectDrive(letter: string, method: EjectDriveMethod, callback: (succeeded: boolean) => void): boolean;
export function ejectDriveSync(letter: string, method: EjectDriveMethod): boolean

// https://github.com/xxoo/node-fswin/wiki/find-and-findSync
export function find(pathToFind: string, callback: Find.AsyncProgressiveCallback, isProgressiveMode: Find.ProgressiveModeEnabled): boolean;
export function find(pathToFind: string, callback: Find.AsyncBasicCallback): boolean;
export function findSync(pathToFind: string, callback: Find.SyncCallback): number;
export function findSync(pathToFind: string): Find.File[];

// https://github.com/xxoo/node-fswin/wiki/getAttributes-and-getAttributesSync
export function getAttributes(path: string, callback: (result?: Attributes) => void): boolean;
export function getAttributesSync(path: string): Attributes;

// https://github.com/xxoo/node-fswin/wiki/getDeviceCapabilities-and-getDeviceCapabilitiesSync
export function getDeviceCapabilities(id: string, callback: (result: DeviceCapabilities) => void): boolean;
export function getDeviceCapabilitiesSync(id: string): DeviceCapabilities;

// https://github.com/xxoo/node-fswin/wiki/getDriveDevice-and-getDriveDeviceSync
export function getDriveDevice(letter: string, callback: (result: DriveDevice) => void): boolean;
export function getDriveDeviceSync(letter: string): DriveDevice;

// https://github.com/xxoo/node-fswin/wiki/getLogicalDriveList-and-getLogicalDriveListSync
export function getLogicalDriveList(callback: (result: LogicalDriveList) => void): boolean;
export function getLogicalDriveListSync(): LogicalDriveList;

// https://github.com/xxoo/node-fswin/wiki/getStorageProperties-and-getStoragePropertiesSync
export function getStorageProperties(path: string, properties: StorageProperties.Properties, callback: (result: StorageProperties.RootObject) => void): boolean;
export function getStoragePropertiesSync(path: string, properties: StorageProperties.Properties): StorageProperties.RootObject;

// https://github.com/xxoo/node-fswin/wiki/splitPath
export function splitPath(fullPath: string): SplitPath;

// https://github.com/xxoo/node-fswin/wiki/getVolumeInformation-and-getVolumeInformationSync
export function getVolumeInformation(path: string, callback: (result: VolumeInformation) => void): boolean;
export function getVolumeInformationSync(path: string): VolumeInformation;

// https://github.com/xxoo/node-fswin/wiki/getVolumeSpace-and-getVolumeSpaceSync
export function getVolumeSpace(path: string, callback: (result: VolumeSpace) => void): boolean;
export function getVolumeSpaceSync(path: string): VolumeSpace;

// https://github.com/xxoo/node-fswin/wiki/setAttributes-and-setAttributesSync
export function setAttributes(path: string, attributes: SetAttributes, callback: (succeeded?: boolean) => void): boolean;
export function setAttributesSync(path: string, attributes: SetAttributes): boolean;

// https://github.com/xxoo/node-fswin/wiki/setVolumeLabel-and-setVolumeLabelSync
export function setVolumeLabel(path: string, label: string, callback: (succeeded: boolean) => void): boolean;
export function setVolumeLabelSync(volume: string, label: string): boolean;

export namespace ntfs {
    // https://github.com/xxoo/node-fswin/wiki/getCompressedSize-and-getCompressedSizeSync
    export function getCompressedSize(file: string, callback: (result: number) => void): boolean;
    export function getCompressedSizeSync(file: string): number;

    // https://github.com/xxoo/node-fswin/wiki/setCompression-and-setCompressionSync
    export function setCompression(fileOrDir: string, callback: (succeeded: boolean) => void, compress?: boolean, create?: boolean): boolean;
    export function setCompressionSync(fileOrDir: string, compress?: boolean, create?: boolean): number;

    // https://github.com/xxoo/node-fswin/wiki/setShortName-and-setShortNameSync
    export function setShortName(pathToSet: string, newShortName?: string, callback?: (succeeded: boolean) => void): boolean;
    export function setShortNameSync(pathToSet: string, newShortName?: string): boolean;

    // https://github.com/xxoo/node-fswin/wiki/setSparse-and-setSparseSync
    export function setSparse(file: string, callback: (succeeded: boolean) => void, create?: boolean): boolean;
    export function setSparseSync(file: string, create?: boolean): boolean;
}
