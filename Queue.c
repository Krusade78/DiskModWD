/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include <Ntddstor.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, DiskModWDQueueInitialize)
#endif

NTSTATUS
DiskModWDQueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:


     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG    queueConfig;

    PAGED_CODE();
    
    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
		WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = DiskModWDEvtIoDeviceControl;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
				 WDF_NO_HANDLE
                 );

    return status;
}

VOID
DiskModWDEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{

	if (IoControlCode == IOCTL_STORAGE_QUERY_PROPERTY)
	{
		WDFMEMORY  reqMemory;
		PSTORAGE_PROPERTY_QUERY pspq = NULL;
		NTSTATUS status = WdfRequestRetrieveOutputMemory(Request, &reqMemory);
		if (!NT_SUCCESS(status))
		{
			WdfRequestComplete(Request, status);
			return;
		}
		pspq = (PSTORAGE_PROPERTY_QUERY)WdfMemoryGetBuffer(reqMemory, NULL);
		if (InputBufferLength >= sizeof(STORAGE_PROPERTY_QUERY))
		{
			if ((pspq->PropertyId == StorageDeviceProperty) && (pspq->QueryType == PropertyStandardQuery))
			{
				BOOLEAN ret = FALSE;
				WDF_REQUEST_SEND_OPTIONS options;
				
				WdfRequestFormatRequestUsingCurrentType(Request);
				WDF_REQUEST_SEND_OPTIONS_INIT(&options, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);
				ret = WdfRequestSend(Request, WdfDeviceGetIoTarget(WdfIoQueueGetDevice(Queue)), &options);
				if ((ret == TRUE) && NT_SUCCESS(WdfRequestGetStatus(Request)))
				{
					PSTORAGE_DEVICE_DESCRIPTOR psdd = NULL;
					status = WdfRequestRetrieveOutputMemory(Request, &reqMemory);
					if (!NT_SUCCESS(status))
					{
						WdfRequestComplete(Request, status);
						return;
					}
					psdd = (PSTORAGE_DEVICE_DESCRIPTOR)WdfMemoryGetBuffer(reqMemory, NULL);
					if (OutputBufferLength >= RTL_SIZEOF_THROUGH_FIELD(STORAGE_DEVICE_DESCRIPTOR, RemovableMedia))
					{
						psdd->RemovableMedia = FALSE;
					}
				}
				status = WdfRequestGetStatus(Request);
				WdfRequestComplete(Request, WdfRequestGetStatus(Request));
				return;
			}
		}
	}

	{
		WDF_REQUEST_SEND_OPTIONS options;
		WDF_REQUEST_SEND_OPTIONS_INIT(&options, WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);
		if (!WdfRequestSend(Request, WdfDeviceGetIoTarget(WdfIoQueueGetDevice(Queue)), &options))
		{
			WdfRequestComplete(Request, WdfRequestGetStatus(Request));
		}
	}
}

