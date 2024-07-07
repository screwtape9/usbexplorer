#ifndef __MOFO_USB_DEVS_WIN_H_
#define __MOFO_USB_DEVS_WIN_H_

#include <gtkmm.h>
#include "usbdevice.h"

class MainWindow : public Gtk::Window
{
public:
  MainWindow();
  virtual ~MainWindow() { }

protected:
  void onQuitBtnClicked();
  void createModel();
  void addColumns();
  void addItemToList(mofousb::USBDevice const& dev);

  //Member widgets:
  Gtk::Box m_VBox;
  Gtk::ScrolledWindow m_Scroller;
  Gtk::Label m_Label;
  Gtk::TreeView m_TreeView;
  Glib::RefPtr<Gtk::ListStore> m_refListStore;
  Gtk::ButtonBox m_BtnBox;
  Gtk::Button m_QuitBtn;

  struct ModelColumns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> vidPid;
    Gtk::TreeModelColumn<Glib::ustring> vendor;
    Gtk::TreeModelColumn<Glib::ustring> device;

    ModelColumns() { add(vidPid); add(vendor); add(device); }
  };

  const ModelColumns m_columns;
};

#endif // __MOFO_USB_DEVS_WIN_H_
