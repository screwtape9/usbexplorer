#include <iomanip>
#include <sstream>
#include "win.h"

static const char *szTitle = "USB Device Explorer";

MainWindow::MainWindow()
  : m_VBox(Gtk::ORIENTATION_VERTICAL, 8)
  , m_Label(szTitle)
  , m_QuitBtn("Quit")
{
  set_title(szTitle);
  set_border_width(8);
  set_default_size(280, 250);

  add(m_VBox);
  m_VBox.pack_start(m_Label, Gtk::PACK_SHRINK);

  m_Scroller.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  m_Scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  m_VBox.pack_start(m_Scroller);

  m_VBox.pack_start(m_BtnBox, Gtk::PACK_SHRINK);
  m_BtnBox.pack_start(m_QuitBtn, Gtk::PACK_SHRINK);
  m_BtnBox.set_border_width(5);
  m_BtnBox.set_layout(Gtk::BUTTONBOX_END);
  m_QuitBtn.signal_clicked().connect(sigc::mem_fun(*this,
                                                   &MainWindow::onQuitBtnClicked));

  createModel();

  m_TreeView.set_model(m_refListStore);
  m_TreeView.set_search_column(m_columns.vidPid.index());

  addColumns();
  m_Scroller.add(m_TreeView);

  show_all();
}

void MainWindow::onQuitBtnClicked()
{
  hide();
}

void MainWindow::createModel()
{
  m_refListStore = Gtk::ListStore::create(m_columns);
  auto devs = mofousb::GetUSBDevices();
  std::for_each(devs.begin(),
                devs.end(),
                sigc::mem_fun(*this, &MainWindow::addItemToList));
}

void MainWindow::addItemToList(mofousb::USBDevice const& dev)
{
  std::stringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(4) << dev.VID() << ':'
     << std::hex << std::setfill('0') << std::setw(4) << dev.PID();
  std::string strVendor(dev.getVendor());
  if (strVendor.empty())
    strVendor = dev.getManufacturer();
  std::string strName(dev.getName());
  if (strName.empty())
    strName = dev.getProduct();

  Gtk::TreeRow row = *(m_refListStore->append());
  row[m_columns.vidPid] = ss.str();;
  row[m_columns.vendor] = strVendor;
  row[m_columns.device] = strName;
}

void MainWindow::addColumns()
{
  m_TreeView.append_column("ID", m_columns.vidPid);
  m_TreeView.append_column("Vendor", m_columns.vendor);
  m_TreeView.append_column("Device/Product", m_columns.device);
}
