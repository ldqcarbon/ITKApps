// generated by Fast Light User Interface Designer (fluid) version 1.0011

#ifndef liSpineInterventionConfigurationGUI_h
#define liSpineInterventionConfigurationGUI_h
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <liSpineInterventionConfiguration.h>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Button.H>

class SpineInterventionConfigurationGUI {
  li::SpineInterventionConfiguration::Pointer m_Configuration;
public:
  SpineInterventionConfigurationGUI();
  Fl_Window *controlWindow;
  static Fl_Menu_Item menu_[];
private:
  inline void cb_Load_i(Fl_Menu_*, void*);
  static void cb_Load(Fl_Menu_*, void*);
  inline void cb_Load1_i(Fl_Menu_*, void*);
  static void cb_Load1(Fl_Menu_*, void*);
  inline void cb_Load2_i(Fl_Menu_*, void*);
  static void cb_Load2(Fl_Menu_*, void*);
  inline void cb_Load3_i(Fl_Menu_*, void*);
  static void cb_Load3(Fl_Menu_*, void*);
  inline void cb_Save_i(Fl_Menu_*, void*);
  static void cb_Save(Fl_Menu_*, void*);
  inline void cb_Quit_i(Fl_Menu_*, void*);
  static void cb_Quit(Fl_Menu_*, void*);
  inline void cb_Room_i(Fl_Menu_*, void*);
  static void cb_Room(Fl_Menu_*, void*);
  inline void cb_Fluoroscopy_i(Fl_Menu_*, void*);
  static void cb_Fluoroscopy(Fl_Menu_*, void*);
  inline void cb_Volume_i(Fl_Menu_*, void*);
  static void cb_Volume(Fl_Menu_*, void*);
  inline void cb_Table_i(Fl_Menu_*, void*);
  static void cb_Table(Fl_Menu_*, void*);
  inline void cb_Fluoroscopy1_i(Fl_Menu_*, void*);
  static void cb_Fluoroscopy1(Fl_Menu_*, void*);
  inline void cb_Spine_i(Fl_Menu_*, void*);
  static void cb_Spine(Fl_Menu_*, void*);
  inline void cb_Vessels_i(Fl_Menu_*, void*);
  static void cb_Vessels(Fl_Menu_*, void*);
  inline void cb_Slicer_i(Fl_Menu_*, void*);
  static void cb_Slicer(Fl_Menu_*, void*);
public:
  virtual ~SpineInterventionConfigurationGUI();
  void Show(void);
  void Hide(void);
  void SetConfiguration( li::SpineInterventionConfiguration * configuration );
};
#endif
