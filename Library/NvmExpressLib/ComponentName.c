/** @file
  NvmExpressDxe driver is used to manage non-volatile memory subsystem which follows
  NVM Express specification.

  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
/**
 * Modified for RefindPlus
 * Copyright (c) 2021 Dayo Akanji (sf.net/u/dakanji/profile)
 *
 * Modifications distributed under the preceding terms.
**/

#include "NvmExpress.h"
#include "../../include/refit_call_wrapper.h"

// EFI Component Name Protocol
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME_PROTOCOL gNvmExpressComponentName = {
    NvmExpressComponentNameGetDriverName,
    NvmExpressComponentNameGetControllerName,
    "eng"
};

// EFI Component Name 2 Protocol
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME2_PROTOCOL gNvmExpressComponentName2 = {
    (EFI_COMPONENT_NAME2_GET_DRIVER_NAME) NvmExpressComponentNameGetDriverName,
    (EFI_COMPONENT_NAME2_GET_CONTROLLER_NAME) NvmExpressComponentNameGetControllerName,
    "en"
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE mNvmExpressDriverNameTable[] = {
    { "eng;en", L"NVM Express Driver" },
    { NULL, NULL }
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE mNvmExpressControllerNameTable[] = {
    { "eng;en", L"NVM Express Controller" },
    { NULL, NULL }
};

/**
  Retrieves a Unicode string that is the user readable name of the driver.

  This function retrieves the user readable name of a driver in the form of a
  Unicode string. If the driver specified by This has a user readable name in
  the language specified by Language, then a pointer to the driver name is
  returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
  by This does not support the language specified by Language,
  then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
                                array indicating the language. This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified
                                in RFC 4646 or ISO 639-2 language code format.

  @param  DriverName[out]       A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                driver specified by This in the language
                                specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by
                                This and the language specified by Language was
                                returned in DriverName.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER DriverName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS EFIAPI NvmExpressComponentNameGetDriverName (
    IN  EFI_COMPONENT_NAME_PROTOCOL       *This,
    IN  CHAR8                             *Language,
    OUT CHAR16                            **DriverName
) {
    return LookupUnicodeString2 (
        Language,
        This->SupportedLanguages,
        mNvmExpressDriverNameTable,
        DriverName,
        (BOOLEAN)(This == &gNvmExpressComponentName)
    );
}

/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by a driver.

  This function retrieves the user readable name of the controller specified by
  ControllerHandle and ChildHandle in the form of a Unicode string. If the
  driver specified by This has a user readable name in the language specified by
  Language, then a pointer to the controller name is returned in ControllerName,
  and EFI_SUCCESS is returned.  If the driver specified by This is not currently
  managing the controller specified by ControllerHandle and ChildHandle,
  then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
  support the language specified by Language, then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  ControllerHandle[in]  The handle of a controller that the driver
                                specified by This is managing.  This handle
                                specifies the controller whose name is to be
                                returned.

  @param  ChildHandle[in]       The handle of the child controller to retrieve
                                the name of.  This is an optional parameter that
                                may be NULL.  It will be NULL for device
                                drivers.  It will also be NULL for a bus drivers
                                that wish to retrieve the name of the bus
                                controller.  It will not be NULL for a bus
                                driver that wishes to retrieve the name of a
                                child controller.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
                                array indicating the language.  This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified in
                                RFC 4646 or ISO 639-2 language code format.

  @param  ControllerName[out]   A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                controller specified by ControllerHandle and
                                ChildHandle in the language specified by
                                Language from the point of view of the driver
                                specified by This.

  @retval EFI_SUCCESS           The Unicode string for the user readable name in
                                the language specified by Language for the
                                driver specified by This was returned in
                                DriverName.

  @retval EFI_INVALID_PARAMETER ControllerHandle is NULL.

  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid
                                EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER ControllerName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This is not currently
                                managing the controller specified by
                                ControllerHandle and ChildHandle.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS EFIAPI NvmExpressComponentNameGetControllerName (
    IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
    IN  EFI_HANDLE                                       ControllerHandle,
    IN  EFI_HANDLE                                       ChildHandle        OPTIONAL,
    IN  CHAR8                                           *Language,
    OUT CHAR16                                          **ControllerName
) {
    EFI_STATUS                           Status;
    EFI_BLOCK_IO_PROTOCOL               *BlockIo;
    NVME_DEVICE_PRIVATE_DATA            *Device;
    EFI_UNICODE_STRING_TABLE            *ControllerNameTable;

    // Make sure this driver is currently managing ControllHandle
    Status = EfiTestManagedDevice (
        ControllerHandle,
        gNvmExpressDriverBinding.DriverBindingHandle,
        &gEfiPciIoProtocolGuid
    );
    if (EFI_ERROR (Status)) {
        return Status;
    }

    ControllerNameTable = mNvmExpressControllerNameTable;
    if (ChildHandle != NULL) {
        Status = EfiTestChildHandle (
            ControllerHandle,
            ChildHandle,
            &gEfiNvmExpressPassThruProtocolGuid
        );
        if (EFI_ERROR (Status)) {
            return Status;
        }

        // Get the child context
        Status = REFIT_CALL_6_WRAPPER(
            gBS->OpenProtocol, ChildHandle,
            &gEfiBlockIoProtocolGuid, (VOID **) &BlockIo,
            gNvmExpressDriverBinding.DriverBindingHandle, ChildHandle, EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );
        if (EFI_ERROR (Status)) {
            return EFI_UNSUPPORTED;
        }

        Device = NVME_DEVICE_PRIVATE_DATA_FROM_BLOCK_IO (BlockIo);
        ControllerNameTable = Device->ControllerNameTable;
    }

    return LookupUnicodeString2 (
        Language,
        This->SupportedLanguages,
        ControllerNameTable,
        ControllerName,
        (BOOLEAN)(This == &gNvmExpressComponentName)
    );

}
