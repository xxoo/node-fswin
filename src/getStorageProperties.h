#pragma once
#include "common.h"

class getStorageProperties {
public:
	const struct infor {
		STORAGE_DEVICE_DESCRIPTOR* DP;
		STORAGE_ADAPTER_DESCRIPTOR* AP;
		STORAGE_WRITE_CACHE_PROPERTY* DWCP;
		STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR* AAP;
		DEVICE_SEEK_PENALTY_DESCRIPTOR* DSP;
		DEVICE_TRIM_DESCRIPTOR* DT;
		DEVICE_LB_PROVISIONING_DESCRIPTOR* DLBPP;
		DEVICE_POWER_DESCRIPTOR* DPP;
		DEVICE_COPY_OFFLOAD_DESCRIPTOR* DCOP;
		STORAGE_MEDIUM_PRODUCT_TYPE_DESCRIPTOR* DMPT;
		STORAGE_RPMB_DESCRIPTOR* ARP;
		STORAGE_DEVICE_IO_CAPABILITY_DESCRIPTOR* DICP;
		STORAGE_TEMPERATURE_DATA_DESCRIPTOR* ATP;
		STORAGE_TEMPERATURE_DATA_DESCRIPTOR* DTP;
		STORAGE_ADAPTER_SERIAL_NUMBER* ASN;
	};
	static infor* func(const char* path, bool params[]) {//you need to delete the result yourself if it is not NULL
		infor* result = NULL;
		HANDLE hDrive = CreateFileA(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDrive != INVALID_HANDLE_VALUE) {
			result = new infor({ 0 });
			DWORD bytes = 0;
			STORAGE_PROPERTY_QUERY q;
			q.QueryType = PropertyStandardQuery;
			if (params[0]) {
				q.PropertyId = StorageDeviceProperty;
				STORAGE_DESCRIPTOR_HEADER hd;
				if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), &hd, sizeof(STORAGE_DESCRIPTOR_HEADER), &bytes, NULL)) {
					result->DP = (STORAGE_DEVICE_DESCRIPTOR*)malloc(hd.Size);
					if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DP, hd.Size, &bytes, NULL)) {
						free(result->DP);
						result->DP = NULL;
					}
				}
			}
			if (params[1]) {
				q.PropertyId = StorageAdapterProperty;
				result->AP = new STORAGE_ADAPTER_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->AP, sizeof(STORAGE_ADAPTER_DESCRIPTOR), &bytes, NULL)) {
					delete result->AP;
					result->AP = NULL;
				}
			}
			if (params[2]) {
				q.PropertyId = StorageDeviceWriteCacheProperty;
				result->DWCP = new STORAGE_WRITE_CACHE_PROPERTY;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DWCP, sizeof(STORAGE_WRITE_CACHE_PROPERTY), &bytes, NULL)) {
					delete result->DWCP;
					result->DWCP = NULL;
				}
			}
			if (params[3]) {
				q.PropertyId = StorageAccessAlignmentProperty;
				result->AAP = new STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->AAP, sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR), &bytes, NULL)) {
					delete result->AAP;
					result->AAP = NULL;
				}
			}
			if (params[4]) {
				q.PropertyId = StorageDeviceSeekPenaltyProperty;
				result->DSP = new DEVICE_SEEK_PENALTY_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DSP, sizeof(DEVICE_SEEK_PENALTY_DESCRIPTOR), &bytes, NULL)) {
					delete result->DSP;
					result->DSP = NULL;
				}
			}
			if (params[5]) {
				q.PropertyId = StorageDeviceTrimProperty;
				result->DT = new DEVICE_TRIM_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DT, sizeof(DEVICE_TRIM_DESCRIPTOR), &bytes, NULL)) {
					delete result->DT;
					result->DT = NULL;
				}
			}
			if (params[6]) {
				q.PropertyId = StorageDeviceLBProvisioningProperty;
				result->DLBPP = new DEVICE_LB_PROVISIONING_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DLBPP, sizeof(DEVICE_LB_PROVISIONING_DESCRIPTOR), &bytes, NULL)) {
					delete result->DLBPP;
					result->DLBPP = NULL;
				}
			}
			if (params[7]) {
				q.PropertyId = StorageDevicePowerProperty;
				result->DPP = new DEVICE_POWER_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DPP, sizeof(DEVICE_POWER_DESCRIPTOR), &bytes, NULL)) {
					delete result->DPP;
					result->DPP = NULL;
				}
			}
			if (params[8]) {
				q.PropertyId = StorageDevicePowerProperty;
				result->DCOP = new DEVICE_COPY_OFFLOAD_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DCOP, sizeof(DEVICE_COPY_OFFLOAD_DESCRIPTOR), &bytes, NULL)) {
					delete result->DCOP;
					result->DCOP = NULL;
				}
			}
			if (params[9]) {
				q.PropertyId = StorageDeviceMediumProductType;
				result->DMPT = new STORAGE_MEDIUM_PRODUCT_TYPE_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DMPT, sizeof(STORAGE_MEDIUM_PRODUCT_TYPE_DESCRIPTOR), &bytes, NULL)) {
					delete result->DMPT;
					result->DMPT = NULL;
				}
			}
			if (params[10]) {
				q.PropertyId = StorageAdapterRpmbProperty;
				result->ARP = new STORAGE_RPMB_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->ARP, sizeof(STORAGE_RPMB_DESCRIPTOR), &bytes, NULL)) {
					delete result->ARP;
					result->ARP = NULL;
				}
			}
			if (params[11]) {
				q.PropertyId = StorageDeviceIoCapabilityProperty;
				result->DICP = new STORAGE_DEVICE_IO_CAPABILITY_DESCRIPTOR;
				if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DICP, sizeof(STORAGE_DEVICE_IO_CAPABILITY_DESCRIPTOR), &bytes, NULL)) {
					delete result->DICP;
					result->DICP = NULL;
				}
			}
			if (params[12]) {
				q.PropertyId = StorageAdapterTemperatureProperty;
				STORAGE_DESCRIPTOR_HEADER hd;
				if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), &hd, sizeof(STORAGE_DESCRIPTOR_HEADER), &bytes, NULL)) {
					result->ATP = (STORAGE_TEMPERATURE_DATA_DESCRIPTOR*)malloc(hd.Size);
					if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->ATP, hd.Size, &bytes, NULL)) {
						free(result->ATP);
						result->ATP = NULL;
					}
				}
			}
			if (params[13]) {
				q.PropertyId = StorageDeviceTemperatureProperty;
				STORAGE_DESCRIPTOR_HEADER hd;
				if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), &hd, sizeof(STORAGE_DESCRIPTOR_HEADER), &bytes, NULL)) {
					result->DTP = (STORAGE_TEMPERATURE_DATA_DESCRIPTOR*)malloc(hd.Size);
					if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->DTP, hd.Size, &bytes, NULL)) {
						free(result->DTP);
						result->DTP = NULL;
					}
				}
			}
			if (params[14]) {
				q.PropertyId = StorageAdapterSerialNumberProperty;
				STORAGE_DESCRIPTOR_HEADER hd;
				if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), &hd, sizeof(STORAGE_DESCRIPTOR_HEADER), &bytes, NULL) && hd.Size >= sizeof(STORAGE_ADAPTER_SERIAL_NUMBER)) {
					result->ASN = (STORAGE_ADAPTER_SERIAL_NUMBER*)malloc(hd.Size);
					if (!DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(STORAGE_PROPERTY_QUERY), result->ASN, hd.Size, &bytes, NULL)) {
						free(result->ASN);
						result->ASN = NULL;
					}
				}
			}
			CloseHandle(hDrive);
		}
		return result;
	}
	static napi_value init(napi_env env, bool isSync = false) {
		napi_value f;
		napi_create_function(env, NULL, 0, isSync ? sync : async, NULL, &f);
		return f;
	}
private:
	static inline const char* names[] = { "deviceProperty", "adapterProperty", "deviceWriteCacheProperty", "accessAlignmentProperty", "deviceSeekPenalty", "deviceTrim", "deviceLBProvisioningProperty", "devicePowerProperty", "deviceCopyOffloadProperty", "deviceMediumProductType", "adapterRpmbProperty", "deviceIoCapabilityProperty", "adapterTemperatureProperty", "deviceTemperatureProperty", "adapterSerialNumber" };
	const struct cbdata {
		napi_async_work work;
		napi_ref self;
		napi_ref cb;
		char* path;
		bool params[15];
		infor* result;
	};

	static napi_value getBusType(napi_env env, BYTE type) {
		napi_value result;
		const char* bustype;
		if (type == BusTypeScsi) {
			bustype = "SCSI";
		} else if (type == BusTypeAtapi) {
			bustype = "ATAPI";
		} else if (type == BusTypeAta) {
			bustype = "ATA";
		} else if (type == BusType1394) {
			bustype = "1394";
		} else if (type == BusTypeSsa) {
			bustype = "SSA";
		} else if (type == BusTypeFibre) {
			bustype = "Fibre";
		} else if (type == BusTypeUsb) {
			bustype = "USB";
		} else if (type == BusTypeRAID) {
			bustype = "RAID";
		} else if (type == BusTypeiScsi) {
			bustype = "iSCSI";
		} else if (type == BusTypeSas) {
			bustype = "SAS";
		} else if (type == BusTypeSata) {
			bustype = "SATA";
		} else if (type == BusTypeSd) {
			bustype = "SD";
		} else if (type == BusTypeMmc) {
			bustype = "MMC";
		} else if (type == BusTypeVirtual) {
			bustype = "Virtual";
		} else if (type == BusTypeFileBackedVirtual) {
			bustype = "FileBackedVirtual";
		} else if (type == BusTypeSpaces) {
			bustype = "Spaces";
		} else if (type == BusTypeNvme) {
			bustype = "NVMe";
		} else if (type == BusTypeSCM) {
			bustype = "SCM";
		} else if (type == BusTypeUfs) {
			bustype = "UFS";
		} else {
			bustype = NULL;
		}
		if (bustype) {
			napi_create_string_latin1(env, bustype, NAPI_AUTO_LENGTH, &result);
		} else {
			napi_create_uint32(env, type, &result);
		}
		return result;
	}
	static napi_value convert(napi_env env, infor* data) {
		napi_value o, tmp, result;
		napi_create_object(env, &result);
		if (data->DP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[0], o);
			napi_create_uint32(env, data->DP->DeviceType, &tmp);
			napi_set_named_property(env, o, "deviceType", tmp);
			napi_create_uint32(env, data->DP->DeviceTypeModifier, &tmp);
			napi_set_named_property(env, o, "deviceTypeModifier", tmp);
			tmp = getBusType(env, data->DP->BusType);
			napi_set_named_property(env, o, "busType", tmp);
			napi_get_boolean(env, data->DP->CommandQueueing, &tmp);
			napi_set_named_property(env, o, "commandQueueing", tmp);
			napi_get_boolean(env, data->DP->RemovableMedia, &tmp);
			napi_set_named_property(env, o, "removableMedia", tmp);
			if (data->DP->VendorIdOffset) {
				char* txt = (char*)((ULONG_PTR)data->DP + data->DP->VendorIdOffset);
				napi_create_string_latin1(env, txt, NAPI_AUTO_LENGTH, &tmp);
				napi_set_named_property(env, o, "vendorId", tmp);
			}
			if (data->DP->ProductIdOffset) {
				char* txt = (char*)((ULONG_PTR)data->DP + data->DP->ProductIdOffset);
				napi_create_string_latin1(env, txt, NAPI_AUTO_LENGTH, &tmp);
				napi_set_named_property(env, o, "productId", tmp);
			}
			if (data->DP->ProductRevisionOffset) {
				char* txt = (char*)((ULONG_PTR)data->DP + data->DP->ProductRevisionOffset);
				napi_create_string_latin1(env, txt, NAPI_AUTO_LENGTH, &tmp);
				napi_set_named_property(env, o, "productRevision", tmp);
			}
			if (data->DP->SerialNumberOffset) {
				char* txt = (char*)((ULONG_PTR)data->DP + data->DP->SerialNumberOffset);
				napi_create_string_latin1(env, txt, NAPI_AUTO_LENGTH, &tmp);
				napi_set_named_property(env, o, "serialNumber", tmp);
			}
			free(data->DP);
		}

		if (data->AP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[1], o);
			napi_create_uint32(env, data->AP->MaximumTransferLength, &tmp);
			napi_set_named_property(env, o, "maximumTransferLength", tmp);
			napi_create_uint32(env, data->AP->MaximumPhysicalPages, &tmp);
			napi_set_named_property(env, o, "maximumPhysicalPages", tmp);
			napi_create_uint32(env, data->AP->AlignmentMask, &tmp);
			napi_set_named_property(env, o, "alignmentMask", tmp);
			napi_get_boolean(env, data->AP->AdapterUsesPio, &tmp);
			napi_set_named_property(env, o, "adapterUsesPio", tmp);
			napi_get_boolean(env, data->AP->AdapterScansDown, &tmp);
			napi_set_named_property(env, o, "adapterScansDown", tmp);
			napi_get_boolean(env, data->AP->CommandQueueing, &tmp);
			napi_set_named_property(env, o, "commandQueueing", tmp);
			napi_get_boolean(env, data->AP->AcceleratedTransfer, &tmp);
			napi_set_named_property(env, o, "acceleratedTransfer", tmp);
			napi_create_uint32(env, data->AP->BusMajorVersion, &tmp);
			napi_set_named_property(env, o, "busMajorVersion", tmp);
			napi_create_uint32(env, data->AP->BusMinorVersion, &tmp);
			napi_set_named_property(env, o, "busMinorVersion", tmp);
			tmp = getBusType(env, data->AP->BusType);
			napi_set_named_property(env, o, "busType", tmp);
			const char* SrbType;
			if (data->AP->SrbType == SRB_TYPE_SCSI_REQUEST_BLOCK) {
				SrbType = "SCSIRequestBlock";
			} else if (data->AP->SrbType == SRB_TYPE_STORAGE_REQUEST_BLOCK) {
				SrbType = "StorageRequestBlock";
			} else {
				SrbType = NULL;
			}
			if (SrbType) {
				napi_create_string_latin1(env, SrbType, NAPI_AUTO_LENGTH, &tmp);
			} else {
				napi_create_uint32(env, data->AP->SrbType, &tmp);
			}
			napi_set_named_property(env, o, "srbType", tmp);
			const char* AddressType;
			if (data->AP->AddressType == STORAGE_ADDRESS_TYPE_BTL8) {
				AddressType = "BTL8";
			} else {
				AddressType = NULL;
			}
			if (AddressType) {
				napi_create_string_latin1(env, AddressType, NAPI_AUTO_LENGTH, &tmp);
			} else {
				napi_create_uint32(env, data->AP->AddressType, &tmp);
			}
			napi_set_named_property(env, o, "addressType", tmp);
			delete data->AP;
		}

		if (data->DWCP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[2], o);
			const char* WriteCacheType;
			if (data->DWCP->WriteCacheType == WriteCacheTypeUnknown) {
				WriteCacheType = "unknown";
			} else if (data->DWCP->WriteCacheType == WriteCacheTypeNone) {
				WriteCacheType = "none";
			} else if (data->DWCP->WriteCacheType == WriteCacheTypeWriteBack) {
				WriteCacheType = "writeBack";
			} else if (data->DWCP->WriteCacheType == WriteCacheTypeWriteThrough) {
				WriteCacheType = "writeThrough";
			} else {
				WriteCacheType = NULL;
			}
			if (WriteCacheType) {
				napi_create_string_latin1(env, WriteCacheType, NAPI_AUTO_LENGTH, &tmp);
			} else {
				napi_create_uint32(env, data->DWCP->WriteCacheType, &tmp);
			}
			napi_set_named_property(env, o, "type", tmp);
			const char* status;
			if (data->DWCP->WriteCacheEnabled == WriteCacheEnableUnknown) {
				status = "unknown";
			} else if (data->DWCP->WriteCacheEnabled == WriteCacheDisabled) {
				status = "disabled";
			} else if (data->DWCP->WriteCacheEnabled == WriteCacheEnabled) {
				status = "enabled";
			} else {
				status = NULL;
			}
			if (status) {
				napi_create_string_latin1(env, status, NAPI_AUTO_LENGTH, &tmp);
			} else {
				napi_create_uint32(env, data->DWCP->WriteCacheEnabled, &tmp);
			}
			napi_set_named_property(env, o, "isEnabled", tmp);
			const char* isChangeable;
			if (data->DWCP->WriteCacheChangeable == WriteCacheChangeUnknown) {
				isChangeable = "unknown";
			} else if (data->DWCP->WriteCacheChangeable == WriteCacheNotChangeable) {
				isChangeable = "notChangeable";
			} else if (data->DWCP->WriteCacheChangeable == WriteCacheChangeable) {
				isChangeable = "changeable";
			} else {
				isChangeable = NULL;
			}
			if (isChangeable) {
				napi_create_string_latin1(env, isChangeable, NAPI_AUTO_LENGTH, &tmp);
			} else {
				napi_create_uint32(env, data->DWCP->WriteCacheEnabled, &tmp);
			}
			napi_set_named_property(env, o, "isChangeable", tmp);
			const char* isWriteThroughSupported;
			if (data->DWCP->WriteThroughSupported == WriteThroughUnknown) {
				isWriteThroughSupported = "unknown";
			} else if (data->DWCP->WriteThroughSupported == WriteThroughNotSupported) {
				isWriteThroughSupported = "notSupported";
			} else if (data->DWCP->WriteThroughSupported == WriteThroughSupported) {
				isWriteThroughSupported = "supported";
			} else {
				isWriteThroughSupported = NULL;
			}
			if (isWriteThroughSupported) {
				napi_create_string_latin1(env, isWriteThroughSupported, NAPI_AUTO_LENGTH, &tmp);
			} else {
				napi_create_uint32(env, data->DWCP->WriteThroughSupported, &tmp);
			}
			napi_set_named_property(env, o, "isWriteThroughSupported", tmp);
			napi_get_boolean(env, data->DWCP->FlushCacheSupported, &tmp);
			napi_set_named_property(env, o, "flushCacheSupported", tmp);
			napi_get_boolean(env, data->DWCP->UserDefinedPowerProtection, &tmp);
			napi_set_named_property(env, o, "userDefinedPowerProtection", tmp);
			napi_get_boolean(env, data->DWCP->NVCacheEnabled, &tmp);
			napi_set_named_property(env, o, "NVCacheEnabled", tmp);
			delete data->DWCP;
		}

		if (data->AAP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[3], o);
			napi_create_uint32(env, data->AAP->BytesPerCacheLine, &tmp);
			napi_set_named_property(env, o, "bytesPerCacheLine", tmp);
			napi_create_uint32(env, data->AAP->BytesOffsetForCacheAlignment, &tmp);
			napi_set_named_property(env, o, "bytesOffsetForCacheAlignment", tmp);
			napi_create_uint32(env, data->AAP->BytesPerLogicalSector, &tmp);
			napi_set_named_property(env, o, "bytesPerLogicalSector", tmp);
			napi_create_uint32(env, data->AAP->BytesPerPhysicalSector, &tmp);
			napi_set_named_property(env, o, "bytesPerPhysicalSector", tmp);
			napi_create_uint32(env, data->AAP->BytesOffsetForSectorAlignment, &tmp);
			napi_set_named_property(env, o, "bytesOffsetForSectorAlignment", tmp);
			delete data->AAP;
		}
		if (data->DSP) {
			napi_get_boolean(env, data->DSP->IncursSeekPenalty, &tmp);
			napi_set_named_property(env, result, names[4], tmp);
			delete data->DSP;
		}
		if (data->DT) {
			napi_get_boolean(env, data->DT->TrimEnabled, &tmp);
			napi_set_named_property(env, result, names[5], tmp);
			delete data->DT;
		}
		if (data->DLBPP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[6], o);
			napi_get_boolean(env, data->DLBPP->ThinProvisioningEnabled, &tmp);
			napi_set_named_property(env, o, "thinProvisioningEnabled", tmp);
			napi_get_boolean(env, data->DLBPP->ThinProvisioningReadZeros, &tmp);
			napi_set_named_property(env, o, "thinProvisioningReadZeros", tmp);
			napi_get_boolean(env, data->DLBPP->AnchorSupported, &tmp);
			napi_set_named_property(env, o, "anchorSupported", tmp);
			napi_get_boolean(env, data->DLBPP->UnmapGranularityAlignmentValid, &tmp);
			napi_set_named_property(env, o, "unmapGranularityAlignmentValid", tmp);
			napi_get_boolean(env, data->DLBPP->GetFreeSpaceSupported, &tmp);
			napi_set_named_property(env, o, "getFreeSpaceSupported", tmp);
			napi_get_boolean(env, data->DLBPP->MapSupported, &tmp);
			napi_set_named_property(env, o, "mapSupported", tmp);
			napi_create_bigint_uint64(env, data->DLBPP->OptimalUnmapGranularity, &tmp);
			napi_set_named_property(env, o, "optimalUnmapGranularity", tmp);
			napi_create_bigint_uint64(env, data->DLBPP->UnmapGranularityAlignment, &tmp);
			napi_set_named_property(env, o, "unmapGranularityAlignment", tmp);
			napi_create_uint32(env, data->DLBPP->MaxUnmapLbaCount, &tmp);
			napi_set_named_property(env, o, "maxUnmapLbaCount", tmp);
			napi_create_uint32(env, data->DLBPP->MaxUnmapBlockDescriptorCount, &tmp);
			napi_set_named_property(env, o, "maxUnmapBlockDescriptorCount", tmp);
			delete data->DLBPP;
		}
		if (data->DPP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[7], o);
			napi_get_boolean(env, data->DPP->DeviceAttentionSupported, &tmp);
			napi_set_named_property(env, o, "deviceAttentionSupported", tmp);
			napi_get_boolean(env, data->DPP->AsynchronousNotificationSupported, &tmp);
			napi_set_named_property(env, o, "asynchronousNotificationSupported", tmp);
			napi_get_boolean(env, data->DPP->IdlePowerManagementEnabled, &tmp);
			napi_set_named_property(env, o, "idlePowerManagementEnabled", tmp);
			napi_get_boolean(env, data->DPP->D3ColdEnabled, &tmp);
			napi_set_named_property(env, o, "d3ColdEnabled", tmp);
			napi_get_boolean(env, data->DPP->D3ColdSupported, &tmp);
			napi_set_named_property(env, o, "d3ColdSupported", tmp);
			napi_get_boolean(env, data->DPP->NoVerifyDuringIdlePower, &tmp);
			napi_set_named_property(env, o, "noVerifyDuringIdlePower", tmp);
			napi_create_uint32(env, data->DPP->IdleTimeoutInMS, &tmp);
			napi_set_named_property(env, o, "idleTimeoutInMS", tmp);
			delete data->DPP;
		}
		if (data->DCOP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[8], o);
			napi_create_uint32(env, data->DCOP->MaximumTokenLifetime, &tmp);
			napi_set_named_property(env, o, "maximumTokenLifetime", tmp);
			napi_create_uint32(env, data->DCOP->DefaultTokenLifetime, &tmp);
			napi_set_named_property(env, o, "defaultTokenLifetime", tmp);
			napi_create_bigint_uint64(env, data->DCOP->MaximumTransferSize, &tmp);
			napi_set_named_property(env, o, "maximumTransferSize", tmp);
			napi_create_bigint_uint64(env, data->DCOP->OptimalTransferCount, &tmp);
			napi_set_named_property(env, o, "optimalTransferCount", tmp);
			napi_create_uint32(env, data->DCOP->MaximumDataDescriptors, &tmp);
			napi_set_named_property(env, o, "maximumDataDescriptors", tmp);
			napi_create_uint32(env, data->DCOP->MaximumTransferLengthPerDescriptor, &tmp);
			napi_set_named_property(env, o, "maximumTransferLengthPerDescriptor", tmp);
			napi_create_uint32(env, data->DCOP->OptimalTransferLengthPerDescriptor, &tmp);
			napi_set_named_property(env, o, "optimalTransferLengthPerDescriptor", tmp);
			napi_create_uint32(env, data->DCOP->OptimalTransferLengthGranularity, &tmp);
			napi_set_named_property(env, o, "optimalTransferLengthGranularity", tmp);
			delete data->DCOP;
		}
		if (data->DMPT) {
			const char* MediumProductType;
			if (data->DMPT->MediumProductType == 1) {
				MediumProductType = "CFast";
			} else if (data->DMPT->MediumProductType == 2) {
				MediumProductType = "CompactFlash";
			} else if (data->DMPT->MediumProductType == 3) {
				MediumProductType = "MemoryStick";
			} else if (data->DMPT->MediumProductType == 4) {
				MediumProductType = "MultiMediaCard";
			} else if (data->DMPT->MediumProductType == 5) {
				MediumProductType = "SecureDigitalCard";
			} else if (data->DMPT->MediumProductType == 6) {
				MediumProductType = "QXD";
			} else if (data->DMPT->MediumProductType == 7) {
				MediumProductType = "UniversalFlashStorage";
			} else {
				MediumProductType = NULL;
			}
			if (MediumProductType) {
				napi_create_string_latin1(env, MediumProductType, NAPI_AUTO_LENGTH, &tmp);
			} else {
				napi_create_uint32(env, data->DMPT->MediumProductType, &tmp);
			}
			napi_set_named_property(env, result, names[9], tmp);
			delete data->DMPT;
		}
		if (data->ARP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[10], o);
			napi_create_uint32(env, data->ARP->SizeInBytes, &tmp);
			napi_set_named_property(env, o, "sizeInBytes", tmp);
			napi_create_uint32(env, data->ARP->MaxReliableWriteSizeInBytes, &tmp);
			napi_set_named_property(env, o, "maxReliableWriteSizeInBytes", tmp);
			const char* FrameFormat;
			if (data->ARP->FrameFormat == StorageRpmbFrameTypeStandard) {
				FrameFormat = "standard";
			} else {
				FrameFormat = NULL;
			}
			if (FrameFormat) {
				napi_create_string_latin1(env, FrameFormat, NAPI_AUTO_LENGTH, &tmp);
			} else {
				napi_create_uint32(env, data->ARP->FrameFormat, &tmp);
			}
			napi_set_named_property(env, o, "frameFormat", tmp);
			delete data->ARP;
		}
		if (data->DICP) {
			napi_create_object(env, &o);
			napi_set_named_property(env, result, names[11], o);
			napi_create_uint32(env, data->DICP->LunMaxIoCount, &tmp);
			napi_set_named_property(env, o, "lunMaxIoCount", tmp);
			napi_create_uint32(env, data->DICP->AdapterMaxIoCount, &tmp);
			napi_set_named_property(env, o, "adapterMaxIoCount", tmp);
			delete data->DICP;
		}
		if (data->ATP) {
			napi_set_named_property(env, result, names[12], convertTemperature(env, data->ATP));
			free(data->ATP);
		}
		if (data->DTP) {
			napi_set_named_property(env, result, names[13], convertTemperature(env, data->DTP));
			free(data->DTP);
		}
		if (data->ASN) {
			napi_create_string_utf16(env, (char16_t*)data->ASN->SerialNumber, NAPI_AUTO_LENGTH, &tmp);
			napi_set_named_property(env, result, names[14], tmp);
			free(data->ASN);
		}
		delete data;
		return result;
	}
	static napi_value convertTemperature(napi_env env, STORAGE_TEMPERATURE_DATA_DESCRIPTOR* data) {
		napi_value o, tmp;
		napi_create_object(env, &o);
		napi_create_int32(env, data->CriticalTemperature, &tmp);
		napi_set_named_property(env, o, "criticalTemperature", tmp);
		napi_create_int32(env, data->WarningTemperature, &tmp);
		napi_set_named_property(env, o, "warningTemperature", tmp);
		napi_create_array(env, &tmp);
		napi_set_named_property(env, o, "temperatureInfo", tmp);
		napi_value t, p, push;
		napi_get_named_property(env, tmp, "push", &push);
		for (WORD i = 0; i < data->InfoCount; ++i) {
			napi_create_object(env, &t);
			napi_call_function(env, tmp, push, 1, &t, NULL);
			napi_create_uint32(env, data->TemperatureInfo[i].Index, &p);
			napi_set_named_property(env, t, "index", p);
			napi_create_int32(env, data->TemperatureInfo[i].Temperature, &p);
			napi_set_named_property(env, t, "temperature", p);
			napi_create_int32(env, data->TemperatureInfo[i].OverThreshold, &p);
			napi_set_named_property(env, t, "overThreshold", p);
			napi_create_int32(env, data->TemperatureInfo[i].UnderThreshold, &p);
			napi_set_named_property(env, t, "underThreshold", p);
			napi_get_boolean(env, data->TemperatureInfo[i].OverThresholdChangable, &p);
			napi_set_named_property(env, t, "overThresholdChangable", p);
			napi_get_boolean(env, data->TemperatureInfo[i].UnderThresholdChangable, &p);
			napi_set_named_property(env, t, "underThresholdChangable", p);
			napi_get_boolean(env, data->TemperatureInfo[i].EventGenerated, &p);
			napi_set_named_property(env, t, "eventGenerated", p);
		}
		return o;
	}
	static void getParams(napi_env env, napi_value argv, bool params[]) {
		napi_value tmp;
		for (BYTE i = 0; i < _countof(names); ++i) {
			napi_get_named_property(env, argv, names[i], &tmp);
			napi_coerce_to_bool(env, tmp, &tmp);
			napi_get_value_bool(env, tmp, &params[i]);
		}
	}
	static napi_value sync(napi_env env, napi_callback_info info) {
		napi_value result;
		napi_get_new_target(env, info, &result);
		if (result) {
			result = NULL;
			napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			napi_value argv[2];
			size_t argc = 2;
			napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
			if (argc >= 2) {
				napi_valuetype t;
				napi_typeof(env, argv[1], &t);
				if (t == napi_object) {
					size_t str_len;
					napi_value tmp;
					napi_coerce_to_string(env, argv[0], &tmp);
					napi_get_value_string_latin1(env, tmp, NULL, 0, &str_len);
					str_len += 1;
					char* str = new char[str_len];
					napi_get_value_string_latin1(env, tmp, str, str_len, NULL);
					bool params[15];
					getParams(env, argv[1], params);
					infor* r = func(str, params);
					delete[]str;
					if (r) {
						result = convert(env, r);
					} else {
						napi_get_null(env, &result);
					}
				}
			}
			if (!result) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		return result;
	}
	static napi_value async(napi_env env, napi_callback_info info) {
		napi_value result;
		napi_get_new_target(env, info, &result);
		if (result) {
			result = NULL;
			napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			napi_value argv[3], self;
			size_t argc = 3;
			napi_get_cb_info(env, info, &argc, argv, &self, NULL);
			if (argc >= 3) {
				napi_valuetype t;
				napi_typeof(env, argv[2], &t);
				if (t == napi_function) {
					napi_typeof(env, argv[1], &t);
					if (t == napi_object) {
						cbdata* data = new cbdata;
						size_t str_len;
						napi_value tmp;
						napi_create_reference(env, argv[2], 1, &data->cb);
						napi_create_reference(env, self, 1, &data->self);
						napi_coerce_to_string(env, argv[0], &tmp);
						napi_get_value_string_latin1(env, tmp, NULL, 0, &str_len);
						str_len += 1;
						data->path = new char[str_len];
						napi_get_value_string_latin1(env, tmp, data->path, str_len, NULL);
						getParams(env, argv[1], data->params);
						napi_create_string_latin1(env, "fswin.getStroageProperties", NAPI_AUTO_LENGTH, &tmp);
						napi_create_async_work(env, NULL, tmp, execute, complete, data, &data->work);
						if (napi_queue_async_work(env, data->work) == napi_ok) {
							napi_get_boolean(env, true, &result);
						} else {
							napi_get_boolean(env, false, &result);
							napi_delete_reference(env, data->cb);
							napi_delete_reference(env, data->self);
							napi_delete_async_work(env, data->work);
							delete[]data->path;
							delete data;
						}
					}
				}
			}
			if (!result) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		return result;
	}
	static void execute(napi_env env, void* data) {
		cbdata* d = (cbdata*)data;
		d->result = func(d->path, d->params);
	}
	static void complete(napi_env env, napi_status status, void* data) {
		cbdata* d = (cbdata*)data;
		delete[]d->path;
		napi_value cb, self, argv;
		napi_get_reference_value(env, d->cb, &cb);
		napi_get_reference_value(env, d->self, &self);
		if (status == napi_ok && d->result) {
			argv = convert(env, d->result);
		} else {
			napi_get_null(env, &argv);
		}
		napi_call_function(env, self, cb, 1, &argv, NULL);
		napi_delete_reference(env, d->cb);
		napi_delete_reference(env, d->self);
		napi_delete_async_work(env, d->work);
		delete d;
	}
};