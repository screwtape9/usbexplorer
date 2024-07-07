#ifndef __MOFO_USB_DEVICE_H_
#define __MOFO_USB_DEVICE_H_

#include <string>
#include <vector>
#include <libusb.h>

namespace mofousb {

class USBDevice
{
  public:
    USBDevice(USBDevice const& dev) { (*this) = dev; };
    USBDevice(struct libusb_device_descriptor const& devDesc,
              struct libusb_config_descriptor const& cfgDesc,
              libusb_device_handle *handle);
    ~USBDevice() { }
    USBDevice& operator=(USBDevice const& dev)
    {
      desc            = dev.desc;
      strManufacturer = dev.strManufacturer;
      strProduct      = dev.strProduct;
      strSerialNum    = dev.strSerialNum;
      strVendor       = dev.strVendor;
      strName         = dev.strName;
      return (*this);
    }
    uint16_t VID() const { return desc.idVendor;  };
    uint16_t PID() const { return desc.idProduct; };
    std::string getManufacturer() const { return strManufacturer; }
    std::string getProduct() const      { return strProduct;      }
    std::string getSerialNum() const    { return strSerialNum;    }
    std::string getVendor() const       { return strVendor;       }
    std::string getName() const         { return strName;         }
    void setVendor(std::string const& name) { strVendor = name; }
    void setName(std::string const& name)   { strName   = name; }
  private:
    USBDevice() { }
    struct libusb_device_descriptor desc;
    std::string strManufacturer;
    std::string strProduct;
    std::string strSerialNum;
    std::string strVendor;
    std::string strName;
};

typedef std::vector<USBDevice> USBDeviceList;

USBDeviceList GetUSBDevices();

} // namespace mofousb

#endif // __MOFO_USB_DEVICE_H_
