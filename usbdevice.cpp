#include "usbdevice.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <functional>
#include <utility>
#include <algorithm>
#include <cstring>
#include <cctype>

namespace mofousb {

USBDevice::USBDevice(struct libusb_device_descriptor const& devDesc,
                     struct libusb_config_descriptor const& cfgDesc,
                     libusb_device_handle *handle)
{
  desc = devDesc;
  uint8_t szBuf[256] = { '\0' };
  auto n = libusb_get_string_descriptor_ascii(handle,
                                              desc.iManufacturer,
                                              szBuf,
                                              sizeof(szBuf));
  if (n < 1)
    strManufacturer.clear();
  else
    strManufacturer.assign((const char *)szBuf, n);

  memset(szBuf, 0, sizeof(szBuf));
  n = libusb_get_string_descriptor_ascii(handle,
                                         desc.iProduct,
                                         szBuf,
                                         sizeof(szBuf));
  if (n < 1)
    strProduct.clear();
  else
    strProduct.assign((const char *)szBuf, n);

  memset(szBuf, 0, sizeof(szBuf));
  n = libusb_get_string_descriptor_ascii(handle,
                                         desc.iSerialNumber,
                                         szBuf,
                                         sizeof(szBuf));
  if (n < 1)
    strSerialNum.clear();
  else
    strSerialNum.assign((const char *)szBuf, n);

  for (uint8_t i = 0; i < cfgDesc.bNumInterfaces; ++i) {
    libusb_interface const& iface = cfgDesc.interface[i];
    for (int j = 0; j < iface.num_altsetting; ++j) {
      libusb_interface_descriptor const& iface_desc = iface.altsetting[j];
      for (uint8_t k = 0; k < iface_desc.bNumEndpoints; ++k) {
        libusb_endpoint_descriptor const& ep_desc = iface_desc.endpoint[k];
        if (ep_desc.bEndpointAddress & LIBUSB_ENDPOINT_IN) {
          //m_EndPointIn = ep_desc.bEndpointAddress;
        }
        else {
          //m_EndPointOut = ep_desc.bEndpointAddress;
        }
      }
    }
  }
}

static void trim(std::string& str)
{
  if (str.empty())
    return;
  std::string delims(" \f\n\r\v"); // don't trim leading tabs!
  auto pos = str.find_first_not_of(delims);
  if (pos == str.npos) {
    str.clear();
    return;
  }
  if (pos != 0)
    str.erase(0, pos);
  delims += '\t'; // do trim tabs off the end
  pos = str.find_last_not_of(delims);
  if (pos != (str.length() - 1))
    str.erase((pos + 1), (str.length() - pos + 1));
}

struct device {
  device() { }
  device(device const& d) : ID(d.ID), name(d.name) { }
  uint16_t ID;
  std::string name;
};

struct vendor : device {
  vendor() : device() { }
  vendor(device const& d) : device(d) { }
  std::vector<device> devices;
};

auto IDMatches = [](device a, device b) { return a.ID == b.ID; };

static std::vector<vendor> usbIDVendors;

bool ParseDeviceFromLine(std::string const& line, device& dev)
{
  std::istringstream ss(line);
  std::string ID;
  if ((ss >> ID) && (ss >> dev.name)) {
    if ((ID.length() == 4) &&
        std::all_of(ID.begin(),
                    ID.end(),
                    [](unsigned char ch){ return std::isxdigit(ch); })) {
      errno = 0;
      dev.ID = strtoul(ID.c_str(), nullptr, 16);
      if (errno == 0) {
        std::string rem;
        if (getline(ss, rem))
          dev.name += rem;
        return true;
      }
    }
  }
  return false;
}

static void GetPrettyNames()
{
/*  
# Syntax:
# vendor  vendor_name
#	device  device_name				<-- single tab
#		interface  interface_name		<-- two tabs

0001  Fry's Electronics
	7778  Counterfeit flash drive [Kingston]
0002  Ingram
	0002  passport00
	7007  HPRT XT300
*/
  std::ifstream fin("/usr/share/misc/usb.ids");
  if (!fin)
    fin.open("/var/lib/usbutils/usb.ids");
  if (fin) {
    std::string line;
    while (getline(fin, line)) {
      trim(line);
      if (line.empty() || (line[0] == '#'))
        continue;
      device dev;
      if (!ParseDeviceFromLine(line, dev))
        continue;
      if (line[0] != '\t') {
        // new vendor
        vendor ven(dev);
        usbIDVendors.push_back(ven);
      }
      else {
        if (line.rfind("\t\t", 0) == 0) {
          // interface (which we don't care about)
        }
        else if (line.rfind("\t", 0) == 0) {
          // device
          usbIDVendors.back().devices.push_back(dev);
        }
      }
    }
  }

  /*
  for (auto v : usbIDVendors) {
    std::cout << std::hex << std::setfill('0') << std::setw(4) << v.ID << "  "
              << v.name << std::endl;
    for (auto d : v.devices) {
      std::cout << "  " << std::hex << std::setfill('0') << std::setw(4) << d.ID << "  "
                << d.name << std::endl;
    }
  }
  */
}

USBDeviceList GetUSBDevices()
{
  GetPrettyNames();
  
  USBDeviceList devs;
  libusb_context *ptr = nullptr;
  auto ret = libusb_init(&ptr);
  if (ret)
    return devs;
  std::unique_ptr<libusb_context, std::function<void(libusb_context *)> >
    ctx(ptr, [](libusb_context *p) { libusb_exit(p); });

  libusb_device **lptr = nullptr;
  auto n = libusb_get_device_list(ctx.get(), &lptr);
  if (n < 1)
    return devs;
  std::unique_ptr<libusb_device *, std::function<void(libusb_device **)> >
    list(lptr, [](libusb_device **p) { libusb_free_device_list(p, 1); });

  for (int i = 0; i < n; ++i) {
    if (list.get()[i]) {
      libusb_device_handle *hptr = nullptr;
      ret = libusb_open(list.get()[i], &hptr);
      if (ret)
        continue;
      std::unique_ptr<libusb_device_handle, std::function<void(libusb_device_handle *)> >
        handle(hptr, [](libusb_device_handle *p) { libusb_close(p); });

      struct libusb_device_descriptor desc;
      memset(&desc, 0, sizeof(desc));
      ret = libusb_get_device_descriptor(list.get()[i], &desc);
      if (ret)
        continue;

      struct libusb_config_descriptor *cptr = nullptr;
      ret = libusb_get_config_descriptor_by_value(list.get()[i], 1, &cptr);
      if (ret)
        continue;
      std::unique_ptr<libusb_config_descriptor, std::function<void(libusb_config_descriptor *)> >
        cfg(cptr, [](libusb_config_descriptor *p) { libusb_free_config_descriptor(p); });

      USBDevice dev(desc, (*cfg.get()), handle.get());
      for (auto v : usbIDVendors) {
        if (dev.VID() == v.ID) {
          dev.setVendor(v.name);
          for (auto j : v.devices)
            if (dev.PID() == j.ID)
              dev.setName(j.name);
        }
      }
      devs.push_back(dev);
    }
  }

  return devs;
}

} // namespace mofousb
