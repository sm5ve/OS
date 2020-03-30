#include <devices/PCIDevice.h>

PCIDevice::PCIDevice(void* b, bool isPCIe) : pcie(isPCIe), base(b){

}
