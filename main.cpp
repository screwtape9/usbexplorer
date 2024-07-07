#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <gtkmm/application.h>
#include "win.h"
#include "usbdevice.h"

int main(int argc, char *argv[])
{
  int ret = 0;

  if (argc == 1) {
    auto app = Gtk::Application::create("com.virtualmofo.usbexplorer");
    MainWindow win;
    ret = app->run(win);
  }
  else {
    auto devs = mofousb::GetUSBDevices();
    for (auto const& dev : devs) {
      std::string strVendor(dev.getVendor());
      if (strVendor.empty())
        strVendor = dev.getManufacturer();
      std::string strName(dev.getName());
      if (strName.empty())
        strName = dev.getProduct();
      std::cout << std::hex << std::setfill('0') << std::setw(4) << dev.VID() << ':'
                << std::hex << std::setfill('0') << std::setw(4) << dev.PID()
                << "  " << strVendor << ' ' << strName << std::endl;
    }
  }

  // valgrind closers
  close(2);
  close(1);
  close(0);
  return ret;
}
