/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DiskModWDCreateDevice)
#endif


NTSTATUS
DiskModWDCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    WDFDEVICE device;
    NTSTATUS status;

    PAGED_CODE();

	WDF_OBJECT_ATTRIBUTES_INIT(&deviceAttributes);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status))
	{
        //
        // Initialize the I/O Package and any Queues
        //
        status = DiskModWDQueueInitialize(device);
    }

	//{
	//	WDF_OBJECT_ATTRIBUTES attributes;
	//	WDFWAITLOCK lockHandle;
	//	LONGLONG t = -60000000;

	//	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	//	status = WdfWaitLockCreate(
	//		&attributes,
	//		&lockHandle
	//		);
	//	WdfWaitLockAcquire(lockHandle, NULL);
	//	WdfWaitLockAcquire(lockHandle, &t);
	//	WdfWaitLockRelease(lockHandle);
	//}

    return status;
}


