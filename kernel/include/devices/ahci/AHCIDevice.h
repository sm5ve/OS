#ifndef AHCIDEVICE
#define AHCIDEVICE

#include <devices/ahci/ahci.h>
#include <paging.h>
#include <ds/smart_pointers.h>
#include <ds/Promise.h>
#include <ds/Queue.h>
#include <ds/Tuple.h>

namespace AHCI{
	struct TransferResponse{
		bool successful;
		void* buffer;

		TransferResponse(bool s, void* b){
			successful = s;
			buffer = b;
		}
		
		TransferResponse(TransferResponse& rhs){
			successful = rhs.successful;
			buffer = rhs.buffer;
		}
		
		TransferResponse& operator=(TransferResponse& rhs){
			successful = rhs.successful;
			buffer = rhs.buffer;
			return *this;
		}
		
		TransferResponse(){}
	};

	struct TransferRequest{
		uint64_t lba;
		void* base;
		void* original_buffer;
		size_t size;
		bool write;
		int assigned_slot;
		PageDirectory& pd;

		TransferRequest() : pd(*MemoryManager::kernel_directory){
			assigned_slot = -1;
		}
		
		TransferRequest(uint64_t l, void* b, size_t s, bool w, PageDirectory& dir) : pd(dir){
			lba = l;
			base = b;
			size = s;
			write = w;
			assigned_slot = -1;
			original_buffer = b;
		}

		TransferRequest(TransferRequest& rhs) : pd(rhs.pd){
			lba = rhs.lba;
			base = rhs.base;
			size = rhs.size;
			write = rhs.write;
			assigned_slot = rhs.assigned_slot;
			original_buffer = rhs.original_buffer;
		}

		TransferRequest& operator=(TransferRequest& rhs){
			lba = rhs.lba;
			base = rhs.base;
			size = rhs.size;
			write = rhs.write;
			assigned_slot = rhs.assigned_slot;
			pd = rhs.pd;
			original_buffer = rhs.original_buffer;
			return *this;
		}
	};
	
	struct WorkRequest{
		TransferRequest req;
		shared_ptr<Promise<TransferResponse, void*>> callback;
		WorkRequest(TransferRequest r, shared_ptr<Promise<TransferResponse, void*>> cb){
			req = r;
			callback = cb;
		}
		
		WorkRequest(){
		
		}

		WorkRequest(WorkRequest& cp){
			req = cp.req;
			callback = cp.callback;
		}

		WorkRequest(WorkRequest&& mv){
			req = mv.req;
			callback = move(mv.callback);
		}

		WorkRequest& operator=(WorkRequest& wrq){
			req = wrq.req;
			callback = wrq.callback;
			return *this;
		}
	};

	class AHCIDevice{
	public:
		virtual void handleInterrupt() = 0;
		virtual bool isDisk() = 0;
	};

	class GenericAHCIDevice : public AHCIDevice{
	public:
		GenericAHCIDevice(HBAPort&);
		void handleInterrupt() override;
		bool isDisk(){
			return false;
		};
	private:
		volatile HBAPort& port;
	};

	class SATA_AHCIDevice : public AHCIDevice{
	public:
		SATA_AHCIDevice(HBAPort&, uint32_t capabilities);
		~SATA_AHCIDevice();
		void handleInterrupt() override;
		bool isDisk(){
			return true;
		};
		//shared_ptr<Promise<size_t>> test();
		void test();
		shared_ptr<Promise<TransferResponse, void*>> queueRequest(TransferRequest);
	private:
		volatile HBAPort& port;
		uint32_t capabilities;
		uint32_t command_slots;
		uint32_t blockSize = 512;  //TODO actually get this information from the hard drive
		volatile FIS* recievedFIS; //must be 256 byte aligned (just allocate new page)
		volatile CMD* commandList; //1k byte aligned (similarly allocate new page)
		volatile CommandTable* commandTables;
		phys_addr port_base;
		Queue<WorkRequest> requests;
		Maybe<WorkRequest>* working;
		uint8_t test_buff[8192];

		void rebase();
		void startCommandEngine();
		void stopCommandEngine();
		uint32_t findCommandSlot();
	
		phys_addr getFISPAddr();
		phys_addr getCommandListPAddr();
		phys_addr getCommandTablePAddr(uint32_t index);
		
		//shared_ptr<Promise<size_t>> workOnRequest(TransferRequest&);
		void updateWorkQueue();	
		void workOnRequest(TransferRequest&);
	};
}

#endif
